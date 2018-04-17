//  ***********************************************************************************************
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
//  ***********************************************************************************************

#include "fitting.h"
#include "core/session.h"
#include "core/typ/async.h"
#include "core/data/cluster.h"
#include "core/algo/coord_trafos.h"
#include "core/def/idiomatic_for.h"

namespace {

//! Fits peak to the given gamma sector and constructs a PeakInfo.
PeakInfo rawFit(const Cluster& cluster, int iGamma, const Peak& peak)
{
    std::unique_ptr<PeakFunction> peakFunction( FunctionRegistry::clone(peak.peakFunction()) );
    const Range& fitrange = peakFunction->range();
    const Metadata* metadata = cluster.avgeMetadata();
    const Range gammaSector = gSession->gammaSelection().slice2range(iGamma);
    deg alpha, beta;
    // TODO (MATH) use fitted tth center, not center of given fit range
    algo::calculateAlphaBeta(alpha, beta, fitrange.center(), gammaSector.center(),
                             cluster.chi(), cluster.omg(), cluster.phi());
    if (fitrange.isEmpty())
        return {metadata, alpha, beta, gammaSector};

    auto& baseline = gSession->baseline();

    // Diffractogram minus fitted background:
    Curve curve = cluster.curve(iGamma);
    const Polynom f = Polynom::fromFit(baseline.polynomDegree(), curve, baseline.ranges());
    curve.subtract([f](double x) {return f.y(x);});

    // Fit the peak:
    peakFunction->fit(curve);
    qpair fitresult = peakFunction->fittedPeak();
    if (!fitrange.contains(fitresult.x))
        return {metadata, alpha, beta, gammaSector};

    float fwhm = peakFunction->fittedFWHM();
    qpair peakError = peakFunction->peakError();
    float fwhmError = peakFunction->fwhmError();
    return {metadata, alpha, beta, gammaSector, float(fitresult.y), float(peakError.y),
            deg(fitresult.x), deg(peakError.x), float(fwhm), float(fwhmError)};
}

} // namespace


void algo::projectIntensities(class QProgressBar* progressBar)
{
    const ActiveClusters& seq = gSession->activeClusters();
    Progress progress(progressBar, "project intensities", seq.size());
    int nGamma = qMax(1, gSession->gammaSelection().numSlices());
    for (const Cluster* cluster : seq.clusters()) {
        progress.step();
        for_i (nGamma) {
            const Range gammaSector = gSession->gammaSelection().slice2range(i);
            cluster->setCurve(i, cluster->toCurve(gammaSector));
        }
    }
}

//! Gathers PeakInfos from Datasets.

//! Either uses the whole gamma range of the cluster (if gammaSector is invalid),
//!  or user limits the range.
//! Even though the betaStep of the equidistant polefigure grid is needed here,
//!  the returned infos won't be on the grid.
//! TODO? gammaStep separately?

void algo::rawFits(class QProgressBar* progressBar)
{
    if (!gSession->peaks().count())
        THROW("BUG: rawFits must not be called unless peak is defined");
    Peak* peak = gSession->peaks().selectedPeak();
    if (!peak)
        qFatal("BUG: no peak selected");

    PeakInfos tmp;
    const ActiveClusters& seq = gSession->activeClusters();
    Progress progress(progressBar, "peak fitting", seq.size());
    int nGamma = qMax(1, gSession->gammaSelection().numSlices());
    for (const Cluster* cluster : seq.clusters()) {
        progress.step();
        for_i (nGamma) {
            const PeakInfo refInfo = rawFit(*cluster, i, *peak);
            if (!qIsNaN(refInfo.inten()))
                tmp.append(refInfo);
        }
    }
    gSession->setDirectPeakInfos(std::move(tmp));
}
