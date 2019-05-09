//  ***********************************************************************************************
//
//  Steca: stress and texture calculator
//
//! @file      core/data/dataset.cpp
//! @brief     Implements class Dataset
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
//  ***********************************************************************************************

#include "core/data/cluster.h"
#include "core/loaders/loaders.h"
#include "core/session.h"
#include "qcr/engine/mixin.h" // remakeAll
#include "qcr/base/debug.h" // ASSERT
#include <algorithm>

//  ***********************************************************************************************
//! @class Datafile

void Datafile::setFileActivation(bool on)
{
    activated_ = on;
    gSession->activeClusters.invalidate();
}

bool Datafile::allClustersSelected() const
{
    for (const Cluster* cluster : clusters_)
        if (!cluster->isSelected())
            return false;
    return true;
}

Qt::CheckState Datafile::clusterState() const
{
    if (activated_)
        return allClustersSelected() ? Qt::Checked : Qt::PartiallyChecked;
    return Qt::Unchecked;
}

//  ***********************************************************************************************
//! @class HighlightedData

void HighlightedData::clear()
{
    setCluster(-1);
}

void HighlightedData::setFile(int i)
{
    if (i<0)
        return clear();
    ASSERT(i<gSession->dataset.countFiles());
    setCluster(gSession->dataset.fileAt(i).clusters_[0]->index());
    ASSERT(i==cluster()->file().index_);
}

void HighlightedData::setCluster(int i)
{
    ASSERT(i < (int)gSession->dataset.allClusters.size());
    currentClusterIndex_ = i;
}

void HighlightedData::reset()
{
    if (!gSession->dataset.allClusters.size())
        return clear();
    setCluster(0);
}

const Cluster *HighlightedData::cluster() const
{
    if (currentClusterIndex_ >= (int)gSession->dataset.allClusters.size())
        return nullptr;
    if (currentClusterIndex_ < 0)
        return nullptr;
    return gSession->dataset.allClusters.at(currentClusterIndex_).get();
}

Cluster *HighlightedData::cluster()
{
    if (currentClusterIndex_ >= (int)gSession->dataset.allClusters.size())
        return nullptr;
    if (currentClusterIndex_ < 0)
        return nullptr;
    return gSession->dataset.allClusters.at(currentClusterIndex_).get();
}


//  ***********************************************************************************************
//! @class Dataset

Dataset::Dataset() {
    binning.       setHook( [this](int)  { onClusteringChanged(); } );
    dropIncomplete.setHook( [this](bool) { onClusteringChanged(); } );
}

void Dataset::clear()
{
    //qDebug() << "Dataset::clear";
    highlight_.clear();
    files_.clear();
    onFileChanged();
    gSession->updateImageSize();
    gSession->params.imageCut.clear();
    gRoot->remakeAll();
    //qDebug() << "Dataset::clear/";
}

void Dataset::removeFile()
{
    const Cluster* cluster = highlight_.cluster();
    if (!cluster)
        return;
    int i = cluster->file().index();
    files_.erase(files_.begin()+i);
    if (files_.empty())
        return clear();
    onFileChanged();
    // reset highlight, which was temporarily unset at the beginning of this function
    if (i<countFiles())
        highlight_.setFile(i);
    else if (i>0)
        highlight_.setFile(i-1);
    else
        qFatal("impossible case in Dataset::removeFile");
    gRoot->remakeAll();
}

void Dataset::addGivenFiles(const QStringList& filePaths)
{
    int i = 0;
    if (const Cluster* cluster = highlight_.cluster())
        i = cluster->file().index();
    highlight_.clear();
    for (const QString& path: filePaths) {
        if (path.isEmpty() || hasFile(path))
            continue;
        Rawfile rawfile { load::loadRawfile(path) };
        gSession->setImageSize(rawfile.imageSize());
        files_.push_back(Datafile {std::move(rawfile)});
        onFileChanged();
    }
    if (countFiles())
        highlight_.setFile( i<0 ? 0 : i );
    onFileChanged();
    gRoot->remakeAll();
}

void Dataset::setClusterSelection(int index, bool on)
{
    allClusters.at(index)->setSelected(on);
    gSession->activeClusters.invalidate();
}

void Dataset::onFileChanged()
{
    int idx = 0;
    int cnt = 0;
    for (Datafile& file: files_) {
        file.index_ = idx++;
        file.offset_ = cnt;
        cnt += file.numMeasurements();
    }
    updateClusters();
    updateMetaModes();
}

void Dataset::onClusteringChanged()
{
    updateClusters();
    highlight_.reset();
}

void Dataset::updateClusters()
{
    allClusters.clear();
    hasIncomplete_ = false;
    int measureNum = 1;
    double measureTime =0;
    for (Datafile& file : files_) {
        file.clusters_.clear();
        for (int i=0; i<file.numMeasurements(); i+=binning.val()) {
            if (i+binning.val()>file.numMeasurements()) {
                hasIncomplete_ = true;
                if (dropIncomplete.val())
                    break;
            }
            std::vector<const Measurement*> group;
            for (int ii=i; ii<file.numMeasurements() && ii<i+binning.val(); ii++) {
                file.raw_.setMeasurementNum(ii, measureNum);
                file.raw_.setMeasurementTime(ii, measureTime);
                measureTime += file.raw_.measurements().at(ii)->deltaTime();
                group.push_back(file.raw_.measurements().at(ii));
                measureNum++;
            }
            std::unique_ptr<Cluster> cluster(new Cluster(group, file, allClusters.size(), i));
            file.clusters_.push_back(cluster.get());
            allClusters.push_back(std::move(cluster));
        }
    }
    gSession->activeClusters.invalidate();

    gSession->gammaSelection.onData();
    gSession->thetaSelection.onData();
}

//! Returns list of activated clusters.
std::vector<const Cluster*> Dataset::activeClustersList() const
{
    std::vector<const Cluster*> ret;
    for (const auto& pCluster : allClusters)
        if (pCluster->isActive())
            ret.push_back(pCluster.get());
    return ret;
}

QJsonObject Dataset::toJson() const
{
    QJsonObject ret;
    QJsonArray arr;
    for (const Datafile& file : files_)
        arr.append(file.raw_.fileInfo().absoluteFilePath());
    ret.insert("files", arr);
    ret.insert("binning", binning.val());
    return ret;
}

void Dataset::fromJson(const JsonObj& obj)
{
    const QJsonArray& files = obj.loadArr("files");
    QStringList paths;
    for (const QJsonValue& file : files)
        paths.append(file.toString());
    addGivenFiles(paths);
    binning.setVal(obj.loadPint("binning", 1));
}

bool Dataset::hasFile(const QString& fileName) const
{
    QFileInfo fileInfo(fileName);
    for (const Datafile& file : files_)
        if (fileInfo == file.raw_.fileInfo())
            return true;
    return false;
}

void Dataset::updateMetaModes() const
{
    meta::clearMetaModes();
    int metasize = meta::numAttributes(false);
    for (int f=0; f<files_.size(); f++) {
        const Datafile& file = files_.at(f);
        for (int m=0; m<metasize; m++) {
            if (meta::getMetaMode(m) == metaMode::MEASUREMENT_DEPENDENT)
                continue;
            else {
                std::vector<const Measurement*> measurements = file.raw_.measurements();
                for (int i=0; i<file.raw_.numMeasurements()-1; i++) {
                    if (measurements.at(i)->metadata().attributeValue(m) ==
                            measurements.at(i+1)->metadata().attributeValue(m))
                        continue;
                    else
                        meta::setMetaMode(m, metaMode::MEASUREMENT_DEPENDENT);
                }
                if (meta::getMetaMode(m) == metaMode::FILE_DEPENDENT)
                    continue;
                else {
                    if (f<files_.size()-1 &&
                            files_.at(f).raw_.measurements().at(0)->metadata().attributeValue(m) ==
                            files_.at(f+1).raw_.measurements().at(0)->metadata().attributeValue(m)
                            || f==files_.size()-1)
                        continue;
                    else
                        meta::setMetaMode(m, metaMode::FILE_DEPENDENT);
                }
            }
        }
    }
}
