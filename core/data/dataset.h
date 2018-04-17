//  ***********************************************************************************************
//
//  Steca: stress and texture calculator
//
//! @file      core/data/dataset.h
//! @brief     Defines class Dataset
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
//  ***********************************************************************************************

#ifndef DATASET_H
#define DATASET_H

#include "core/data/active_clusters.h"
#include "core/data/cluster.h"
#include <memory>

//! A Rawfile and associated information.

class Datafile {
public:
    Datafile() = delete;
    Datafile(const Datafile&) = delete;
    Datafile(Datafile&&) = default;
    Datafile& operator=(Datafile&&) = default;
    Datafile(Rawfile&& raw) : raw_(std::move(raw)) {}

    int numMeasurements() const { return raw_.numMeasurements(); }
    QString name() const { return raw_.fileName(); }
    Qt::CheckState activated() const;

    // TODO privatize
    Rawfile raw_; //!< owned by this
    int index_; //!< index in files_
    int offset_;  //!< first index in total list of Measurement|s
    std::vector<Cluster*> clusters_; //!< back links to Cluster|s made from this
};


//! Pointer to highlighted data.

class HighlightedData {
public:
    void clear();
    void unset();
    void setFile(int);
    void setCluster(int);
    void reset();
    void setMeasurement(int val);

    const Cluster* cluster() const { return current_; }
    int clusterIndex() const;
    const Datafile* file() const;
    int fileIndex() const;
    const Measurement* measurement() const;
    int measurementIndex() const { return measurement_; }

private:
    const Cluster* current_ { nullptr };
    int measurement_ {0}; //!< selected for image display (index in highlighted cluster)
};


//! Loaded Datafile|s. Does not include the correction file.

class Dataset {
public:
    // Accessor methods
    HighlightedData& highlight() { return highlight_; }
    const HighlightedData& highlight() const { return highlight_; }

    // Modifying methods:
    void clear();
    void fromJson(const JsonObj& obj);
    void addGivenFiles(const QStringList& filePaths);
    void removeFile();
    void setBinning(int by);
    void setDropIncomplete(bool on);
    void activateCluster(int index, bool on);
    void setFileActivation(int index, bool on);

    // Const methods:
    int countFiles() const { return files_.size(); }
    int countClusters() const { return allClusters_.size(); }
    const Datafile& fileAt(int i) const;
    const Cluster& clusterAt(int i) const;
    int offset(const Datafile& file) const { return file.offset_; }

    int binning() const { return binning_; }
    bool dropIncomplete() const { return dropIncomplete_; }
    bool hasIncomplete() const { return hasIncomplete_; }

    const ActiveClusters& activeClusters() const { return activeClusters_; }

    QJsonObject toJson() const;

private:
    std::vector<Datafile> files_; //!< loaded Datafile|s only live here
    std::vector<std::unique_ptr<Cluster>> allClusters_; //!< all Cluster|s are owned by this
    // leave this a unique_ptr because other vectors backlink through Cluster* pointers

    int binning_ {1}; //!< bin so many Measurement|s into one cluster
    bool dropIncomplete_ {false}; //!< drop Cluster|s that have less than binning_ members.
    bool hasIncomplete_; //!< current binning does result in at least one incomplete cluster

    HighlightedData highlight_; //!< wraps pointer to highlighted Datafile and Cluster

    ActiveClusters activeClusters_; //!< active clusters

    void onFileChanged();
    void onClusteringChanged();
    void updateClusters();
    void updateActiveClusters();

    bool hasFile(const QString& fileName) const;
};

#endif // DATASET_H
