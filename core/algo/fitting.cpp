// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      core/algo/fitting.cpp
//! @brief     Implements function rawFit, rawFits
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "fitting.h"
#include "core/session.h"
#include "core/algo/calc_polefigure.h"
#include "core/algo/coord_trafos.h"
#include "core/typ/async.h"

//! Fits peak to the given gamma sector and constructs a PeakInfo.
PeakInfo algo::rawFit(const Cluster& cluster, const Peak& peak, const Range& gmaSector)
{
    // fit peak, and retrieve peak parameters:
    Curve curve = cluster.toCurve(gmaSector);
    auto& baseline = gSession->baseline();
    const Polynom f = Polynom::fromFit(baseline.polynomDegree(), curve, baseline.ranges());
    curve.subtract([f](qreal x) {return f.y(x);});

    std::unique_ptr<PeakFunction> peakFunction( FunctionRegistry::clone(peak.peakFunction()) );
    peakFunction->fit(curve);
    const Range& rgeTth = peakFunction->range();
    qpair fitresult = peakFunction->fittedPeak();
    fwhm_t fwhm = peakFunction->fittedFWHM();
    qpair peakError = peakFunction->peakError();
    fwhm_t fwhmError = peakFunction->fwhmError();

    // compute alpha, beta:
    deg alpha, beta;
    algo::calculateAlphaBeta(alpha, beta, rgeTth.center(), gmaSector.center(),
                             cluster.chi(), cluster.omg(), cluster.phi());

    shp_Metadata metadata = cluster.avgeMetadata();

    return rgeTth.contains(fitresult.x)
        ? PeakInfo(
              metadata, alpha, beta, gmaSector, inten_t(fitresult.y), inten_t(peakError.y),
              deg(fitresult.x), deg(peakError.x), fwhm_t(fwhm), fwhm_t(fwhmError))
        : PeakInfo(metadata, alpha, beta, gmaSector);
}

//! Gathers PeakInfos from Datasets.

//! Either uses the whole gamma range of the cluster (if gammaSector is invalid),
//!  or user limits the range.
//! Even though the betaStep of the equidistant polefigure grid is needed here,
//!  the returned infos won't be on the grid.
//! TODO? gammaStep separately?

PeakInfos algo::rawFits(const ActiveClusters& seq, const Peak& peak, Progress* progress)
{
    PeakInfos ret;
    bool interpol = gSession->interpol().enabled();
    if (progress)
        progress->setTotal((interpol ? 2 : 1)*seq.size());
    int nGamma = qMax(1, gSession->gammaSelection().numSlices());
    for (const Cluster* cluster : seq.clusters()) {
        if (progress)
            progress->step();
        for_i (nGamma) {
            const PeakInfo refInfo = rawFit(
                *cluster, peak, gSession->gammaSelection().slice2range(i));
            if (!qIsNaN(refInfo.inten()))
                ret.append(refInfo);
        }
    }

    if (interpol)
        ret = interpolateInfos(ret, progress);

    return ret;
}