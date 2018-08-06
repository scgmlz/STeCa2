//  ***********************************************************************************************
//
//  Steca: stress and texture calculator
//
//! @file      core/calc/peak_info.h
//! @brief     Defines classes PeakInfo, PeakInfos
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
//  ***********************************************************************************************

#ifndef PEAK_INFO_H
#define PEAK_INFO_H

#include "core/meta/metadata.h"
#include "core/typ/range.h"

//! Metadata, peak fit results, and pole figure angles.

class PeakInfo final {
public:
    PeakInfo();
    PeakInfo(const Metadata*, deg alpha, deg beta, Range, float, float /*error*/,
             deg, deg /*error*/, float, float /*error*/);
    PeakInfo(const Metadata*, deg alpha, deg beta, Range);
    PeakInfo(deg alpha, deg beta, Range, float, float /*error*/, deg, deg /*error*/,
             float, float /*error*/);
    PeakInfo(deg alpha, deg beta);
//    PeakInfo(const PeakInfo&) = default;
//    PeakInfo(PeakInfo&&) = default;

    enum class eReflAttr {
        ALPHA,
        BETA,
        GAMMA1,
        GAMMA2,
        INTEN,
        SIGMA_INTEN,
        TTH,
        SIGMA_TTH,
        FWHM,
        SIGMA_FWHM,
        NUM_REFL_ATTR,
    };

    static QStringList dataTags(bool out);
    static std::vector<VariantComparator*> dataCmps();

    deg alpha() const { return alpha_; }
    deg beta() const { return beta_; }
    Range rgeGma() const { return rgeGma_; }
    double inten() const { return inten_; }
    double intenError() const { return intenError_; }
    deg tth() const { return tth_; }
    deg tthError() const { return tthError_; }
    double fwhm() const { return fwhm_; }
    double fwhmError() const { return fwhmError_; }
    std::vector<QVariant> data() const;

private:
    const Metadata* md_;
    deg alpha_, beta_;
    Range rgeGma_;
    float inten_, intenError_;
    deg tth_, tthError_;
    float fwhm_, fwhmError_;

    static QString const reflStringTag(int attr, bool out);
};


//! A list of PeakInfo's, associated with _one_ Bragg peak and different orientations alpha,beta.

class PeakInfos {
public:
    PeakInfos() {}
    PeakInfos(const PeakInfos&) = delete;

    void appendPeak(PeakInfo&&);

    const std::vector<PeakInfo>& peaks() const { return peaks_; }
    void get4(const int idxX, const int idxY,
              std::vector<double>& xs, std::vector<double>& ys,
              std::vector<double>& ysLow, std::vector<double>& ysHig) const;

private:
    std::vector<PeakInfo> peaks_;
};


class AllPeaks {
public:
    // accessor methods:
    const PeakInfos& directPeakInfos() const { return directPeakInfos_; }
    const PeakInfos& interpolatedPeakInfos() const { return interpolatedPeakInfos_; }
    const PeakInfos& peakInfos() const;

    void setDirectPeakInfos(PeakInfos&& val) {
        directPeakInfos_ = std::move(val); }
    void setInterpolatedPeakInfos(PeakInfos&& val) {
        interpolatedPeakInfos_ = std::move(val); }

private:
    PeakInfos directPeakInfos_;
    PeakInfos interpolatedPeakInfos_;
};

#endif // PEAK_INFO_H