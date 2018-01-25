// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      core/session.h
//! @brief     Defines class Session
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef SESSION_H
#define SESSION_H

#include "core/data/corrset.h"
#include "core/data/dataset.h"

extern class Session* gSession;

//! Companion of MainWin and TheHub, holds data and data-related settings.

//! One instance of this class coexists with the main window. It is accessible from everywhere
//! through the global pointer gSession.

//! The original idea was that core and GUI only communicate via function calls between
//! Session and TheHub. In the big refactoring after v2.0.5, this has been given up.

class Session final : public QObject, public ISingleton<Session> {
    Q_OBJECT
public:
    Session();

    // Accessor methods:
    Dataset& dataset() { return dataset_; }
    const Dataset& dataset() const { return dataset_; }

    Corrset& corrset() { return corrset_; }
    const Corrset& corrset() const { return corrset_; }

    // Modifying methods:
    void clear();

    void setMetaSelection(const vec<bool>&);
    const vec<bool>& getMetaSelection() { return metaSelection_; }

    void setImageTransformMirror(bool);
    void setImageTransformRotate(ImageTransform const&);
    void setImageCut(bool isTopOrLeft, bool linked, ImageCut const&);
    void setGeometry(qreal detectorDistance, qreal pixSize, IJ const& midPixOffset);
    void setGammaRange(const Range&);
    void setBgRanges(const Ranges&);
    bool addBgRange(const Range&);
    bool removeBgRange(const Range&);
    void setBgPolyDegree(int);
    void setIntenScaleAvg(bool, qreal);
    void addReflection(const QString&);
    void addReflection(const QJsonObject& obj);
    void removeReflection(int i) { reflections_.remove(i); }
    void setNorm(eNorm);

    // Const methods: // TODO expand corrset() calls in calling code
    bool hasCorrFile() const { return corrset().hasFile(); }
    const Rawfile& corrFile() const { return corrset().raw(); }
    shp_Image corrImage() const { return corrset().corrImage(); }
    const Image* intensCorr() const { return corrset().intensCorr(); }
    void tryEnableCorr(bool on) { return corrset().tryEnable(on); }
    bool isCorrEnabled() const { return corrset().isEnabled(); }

    const Experiment& experiment() const { return dataset().experiment(); }

    size2d imageSize() const;
    ImageTransform const& imageTransform() const { return imageTransform_; }
    ImageCut const& imageCut() const { return imageCut_; }

    Geometry const& geometry() const { return geometry_; }
    IJ midPix() const;

    const Range& gammaRange() const { return gammaRange_; }

    shp_AngleMap angleMap(Measurement const&) const;
    static shp_AngleMap angleMap(Session const& session, Measurement const& ds) {
        return session.angleMap(ds); }

    shp_ImageLens imageLens(const Image&, bool trans, bool cut) const;
    shp_SequenceLens defaultClusterLens(Sequence const& seq) const;
    shp_SequenceLens highlightsLens() const;

    ReflectionInfos makeReflectionInfos(
        Reflection const&, int gmaSlices, const Range&, Progress*) const;

    const Ranges& bgRanges() const { return bgRanges_; }
    int bgPolyDegree() const { return bgPolyDegree_; }
    bool intenScaledAvg() const { return intenScaledAvg_; }
    qreal intenScale() const { return intenScale_; }
    Reflections const& reflections() const { return reflections_; }

    qreal calcAvgBackground(Sequence const&) const;
    qreal calcAvgBackground() const;

signals:
    void sigFiles();         //!< list of loaded files has changed
    void sigClusters();      //!< list of clusters has changed
    void sigHighlight();     //!< highlighted File or/and Cluster has changed
    void sigMetaSelection(); //!< meta data selected for display have changed
    void sigCorr();          //!< corr file has been loaded or unloaded or enabled or disabled
    void sigActivated();     //!< selection of active clusters has changed
    void sigDetector();      //!< detector geometry has changed
    void sigDiffractogram(); //!< diffractogram must be repainted
    void sigBaseline();      //!< baseline fit has changed
    void sigNorm();          //!< normalization has changed

private:
    friend Dataset; // TODO try to get rid of this
    Dataset dataset_;
    friend Corrset; // TODO try to get rid of this
    Corrset corrset_;

    vec<bool> metaSelection_; //!< true if meta datum is to be displayed
    bool intenScaledAvg_ {true}; // if not, summed
    qreal intenScale_ {1};
    size2d imageSize_; //!< All images must have this same size
    ImageTransform imageTransform_;
    ImageCut imageCut_;
    Geometry geometry_;
    Range gammaRange_;
    int bgPolyDegree_ {0};
    Ranges bgRanges_;
    Reflections reflections_;
    eNorm norm_ {eNorm::NONE};

    mutable cache_lazy<ImageKey, AngleMap> angleMapCache_ {360};

    void updateImageSize(); //!< Clears image size if session has no files
    void setImageSize(size2d const&) THROWS; //!< Ensures same size for all images

    shp_SequenceLens dataseqLens(Sequence const&, eNorm, bool trans, bool cut) const;
    Curve curveMinusBg(SequenceLens const&, const Range&) const;
    ReflectionInfo makeReflectionInfo(SequenceLens const&, Reflection const&, const Range&) const;
};

#endif // SESSION_H
