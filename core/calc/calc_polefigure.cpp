// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      core/calc/calc_polefigure.cpp
//! @brief     Implements function interpolateInfos
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "core/calc/calc_polefigure.h"
#include "core/def/idiomatic_for.h"
#include <qmath.h>

namespace {

struct itf_t {
    itf_t() : itf_t(inten_t(NAN), deg(NAN), fwhm_t(NAN)) {}
    itf_t(inten_t _inten, deg _tth, fwhm_t _fwhm) : inten(_inten), tth(_tth), fwhm(_fwhm) {}

    void operator+=(itf_t const&); // used once to compute average

    inten_t inten;
    deg tth;
    fwhm_t fwhm;
};

void itf_t::operator+=(itf_t const& that) {
    inten += that.inten;
    tth += that.tth;
    fwhm += that.fwhm;
}

typedef vec<itf_t> itfs_t;
typedef vec<ReflectionInfo const*> info_vec;

// Calculates the difference of two angles. Parameters should be in [0, 360].
deg calculateDeltaBeta(deg beta1, deg beta2) {
    // Due to cyclicity of angles (360 is equivalent to 0), some magic is needed.
    qreal deltaBeta = beta1 - beta2;
    qreal tempDelta = deltaBeta - 360;
    if (qAbs(tempDelta) < qAbs(deltaBeta))
        deltaBeta = tempDelta;
    tempDelta = deltaBeta + 360;
    if (qAbs(tempDelta) < qAbs(deltaBeta))
        deltaBeta = tempDelta;
    debug::ensure(-180 <= deltaBeta && deltaBeta <= 180);
    return deg(deltaBeta);
}

// Calculates the angle between two points on a unit sphere.
deg angle(deg alpha1, deg alpha2, deg deltaBeta) {
    // Absolute value of deltaBeta is not needed because cos is an even function.
    deg ret = rad(acos( cos(alpha1.toRad()) * cos(alpha2.toRad())
                       + sin(alpha1.toRad()) * sin(alpha2.toRad()) * cos(deltaBeta.toRad())))
                 .toDeg();
    debug::ensure(0 <= ret && ret <= 180);
    return ret;
}

enum class eQuadrant {
    NORTHEAST,
    SOUTHEAST,
    SOUTHWEST,
    NORTHWEST,
};

static int NUM_QUADRANTS = 4;

typedef vec<eQuadrant> Quadrants;

Quadrants allQuadrants() {
    return { eQuadrant::NORTHEAST, eQuadrant::SOUTHEAST, eQuadrant::SOUTHWEST,
             eQuadrant::NORTHWEST };
}

bool inQuadrant(eQuadrant quadrant, deg deltaAlpha, deg deltaBeta) {
    switch (quadrant) {
    case eQuadrant::NORTHEAST: return deltaAlpha >= 0 && deltaBeta >= 0;
    case eQuadrant::SOUTHEAST: return deltaAlpha >= 0 && deltaBeta < 0;
    case eQuadrant::SOUTHWEST: return deltaAlpha < 0 && deltaBeta < 0;
    case eQuadrant::NORTHWEST: return deltaAlpha < 0 && deltaBeta >= 0;
    }
    NEVER return false;
}

// Search quadrant remapping in case no point was found.
eQuadrant remapQuadrant(eQuadrant q) {
    switch (q) {
    case eQuadrant::NORTHEAST: return eQuadrant::NORTHWEST;
    case eQuadrant::SOUTHEAST: return eQuadrant::SOUTHWEST;
    case eQuadrant::SOUTHWEST: return eQuadrant::NORTHEAST;
    case eQuadrant::NORTHWEST: return eQuadrant::SOUTHEAST;
    }
    NEVER return eQuadrant::NORTHEAST;
}

// Checks if (alpha,beta) is inside radius from (centerAlpha,centerBeta).
bool inRadius(deg alpha, deg beta, deg centerAlpha, deg centerBeta, deg radius) {
    qreal a = angle(alpha, centerAlpha, calculateDeltaBeta(beta, centerBeta));
    return qAbs(a) < radius;
}

// Adds data from reflection infos within radius from alpha and beta
// to the peak parameter lists.
void searchPoints(deg alpha, deg beta, deg radius, ReflectionInfos const& infos, itfs_t& itfs) {
    // REVIEW Use value trees to improve performance.
    for (const ReflectionInfo& info : infos) {
        if (inRadius(info.alpha(), info.beta(), alpha, beta, radius))
            itfs.append(itf_t(info.inten(), info.tth(), info.fwhm()));
    }
}

// Searches closest ReflectionInfos to given alpha and beta in quadrants.
void searchInQuadrants(
    Quadrants const& quadrants, deg alpha, deg beta, deg searchRadius, ReflectionInfos const& infos,
    info_vec& foundInfos, vec<qreal>& distances) {
    debug::ensure(quadrants.count() <= NUM_QUADRANTS);
    // Take only reflection infos with beta within +/- BETA_LIMIT degrees into
    // account. Original STeCa used something like +/- 1.5*36 degrees.
    qreal const BETA_LIMIT = 30;
    distances.fill(std::numeric_limits<qreal>::max(), quadrants.count());
    foundInfos.fill(nullptr, quadrants.count());
    // Find infos closest to given alpha and beta in each quadrant.
    for (const ReflectionInfo& info : infos) {
        // REVIEW We could do better with value trees than looping over all infos.
        deg deltaBeta = calculateDeltaBeta(info.beta(), beta);
        if (fabs(deltaBeta) > BETA_LIMIT)
            continue;
        deg deltaAlpha = info.alpha() - alpha;
        // "Distance" between grid point and current info.
        deg d = angle(alpha, info.alpha(), deltaBeta);
        for_i (quadrants.count()) {
            if (inQuadrant(quadrants.at(i), deltaAlpha, deltaBeta)) {
                if (d >= distances.at(i))
                    continue;
                distances[i] = d;
                if (qIsNaN(searchRadius) || d < searchRadius) {
                    foundInfos[i] = &info;
                }
            }
        }
    }
}

itf_t inverseDistanceWeighing(vec<qreal> const& distances, info_vec const& infos) {
    int N = NUM_QUADRANTS;
    // Generally, only distances.count() == values.count() > 0 is needed for this
    // algorithm. However, in this context we expect exactly the following:
    RUNTIME_CHECK(distances.count() == N, "distances size should be 4");
    RUNTIME_CHECK(infos.count() == N, "infos size should be 4");
    vec<qreal> inverseDistances(N);
    qreal inverseDistanceSum = 0;
    for_i (NUM_QUADRANTS) {
        if (distances.at(i) == .0) {
            // Points coincide; no need to interpolate.
            const ReflectionInfo* in = infos.at(i);
            return { in->inten(), in->tth(), in->fwhm() };
        }
        inverseDistances[i] = 1 / distances.at(i);
        inverseDistanceSum += inverseDistances.at(i);
    }
    // REVIEW The RAW peak may need different handling.
    qreal offset = 0;
    qreal height = 0;
    qreal fwhm = 0;
    for_i (N) {
        const ReflectionInfo* in = infos.at(i);
        qreal d = inverseDistances.at(i);
        offset += in->tth() * d;
        height += in->inten() * d;
        fwhm += in->fwhm() * d;
    }

    return { inten_t(height/inverseDistanceSum),
            deg(offset/inverseDistanceSum),
            fwhm_t(fwhm/inverseDistanceSum) };
}

// Interpolates reflection infos to a single point using idw.
itf_t interpolateValues(deg searchRadius, ReflectionInfos const& infos, deg alpha, deg beta) {
    info_vec interpolationInfos;
    vec<qreal> distances;
    searchInQuadrants(
        allQuadrants(), alpha, beta, searchRadius, infos, interpolationInfos, distances);
    // Check that infos were found in all quadrants.
    int numQuadrantsOk = 0;
    for_i (NUM_QUADRANTS) {
        if (interpolationInfos.at(i)) {
            ++numQuadrantsOk;
            continue;
        }
        // No info found in quadrant? Try another quadrant. See
        // [J.Appl.Cryst.(2011),44,641] for the angle mapping.
        eQuadrant newQ = remapQuadrant(eQuadrant(i));
        qreal const newAlpha = i == int(eQuadrant::NORTHEAST) || i == int(eQuadrant::SOUTHEAST)
            ? 180 - alpha
            : -alpha;
        qreal newBeta = beta < 180 ? beta + 180 : beta - 180;
        info_vec renewedSearch;
        vec<qreal> newDistance;
        searchInQuadrants(
            { newQ }, newAlpha, newBeta, searchRadius, infos, renewedSearch, newDistance);
        debug::ensure(renewedSearch.count() == 1);
        debug::ensure(newDistance.count() == 1);
        if (renewedSearch.first()) {
            interpolationInfos[i] = renewedSearch.first();
            distances[i] = newDistance.first();
            ++numQuadrantsOk;
        }
    }
    // Use inverse distance weighing if everything is alright.
    if (numQuadrantsOk == NUM_QUADRANTS)
        return inverseDistanceWeighing(distances, interpolationInfos);
    else
        return {};
}

} // local methods

//! Interpolates infos to equidistant grid in alpha and beta.
ReflectionInfos interpolateInfos(
    ReflectionInfos const& infos, deg alphaStep, deg betaStep, deg idwRadius, deg averagingAlphaMax,
    deg averagingRadius, qreal inclusionTreshold, Progress* progress) {
    // Two interpolation methods are used here:
    // If grid point alpha <= averagingAlphaMax, points within averagingRadius
    // will be averaged.
    // If averaging fails, or alpha > averagingAlphaMax, inverse distance weighing
    // will be used.

    debug::ensure(0 < alphaStep && alphaStep <= 90);
    debug::ensure(0 < betaStep && betaStep <= 360);
    debug::ensure(0 <= averagingAlphaMax && averagingAlphaMax <= 90);
    debug::ensure(0 <= averagingRadius);
    // Setting idwRadius = NaN disables idw radius checks and falling back to
    // idw when averaging fails.
    debug::ensure(qIsNaN(idwRadius) || 0 <= idwRadius);
    debug::ensure(0 <= inclusionTreshold && inclusionTreshold <= 1);

    // NOTE We expect all infos to have the same gamma range.

    // REVIEW qRound oder qCeil?
    int numAlphas = qRound(90. / alphaStep);
    int numBetas = qRound(360. / betaStep);

    ReflectionInfos interpolatedInfos; // Output data.

    interpolatedInfos.reserve(numAlphas * numBetas);

    if (progress)
        progress->setTotal(numAlphas * numBetas); // REVIEW + 1?

    for_int (i, numAlphas + 1) { // REVIEW why + 1
        deg const alpha = i * alphaStep;
        for_int (j, numBetas) {
            deg const beta = j * betaStep;

            if (progress)
                progress->step();

            if (infos.isEmpty()) {
                interpolatedInfos.append(ReflectionInfo(alpha, beta));
                continue;
            }

            if (alpha <= averagingAlphaMax) {
                // Use averaging.

                itfs_t itfs;
                searchPoints(alpha, beta, averagingRadius, infos, itfs);

                if (!itfs.isEmpty()) {
                    // If inclusionTreshold < 1, we'll only use a fraction of largest
                    // reflection parameter values.
                    std::sort(itfs.begin(), itfs.end(), [](itf_t const& i1, itf_t const& i2) {
                        return i1.inten < i2.inten;
                    });

                    itf_t avg(0, 0, 0);

                    int iEnd = itfs.count();
                    int iBegin = qMin(qRound(itfs.count() * (1. - inclusionTreshold)), iEnd - 1);
                    debug::ensure(iBegin < iEnd);
                    int n = iEnd - iBegin;

                    for (int i = iBegin; i < iEnd; ++i)
                        avg += itfs.at(i);

                    interpolatedInfos.append(ReflectionInfo(
                        alpha, beta, infos.first().rgeGma(), avg.inten / n, inten_t(NAN),
                        avg.tth / n, deg(NAN), avg.fwhm / n, fwhm_t(NAN)));
                    continue;
                }

                if (qIsNaN(idwRadius)) {
                    // Don't fall back to idw, just add an unmeasured info.
                    interpolatedInfos.append(ReflectionInfo(alpha, beta));
                    continue;
                }
            }

            // Use idw, if alpha > averagingAlphaMax OR averaging failed (too small
            // averagingRadius?).
            itf_t itf = interpolateValues(idwRadius, infos, alpha, beta);
            interpolatedInfos.append(ReflectionInfo(
                alpha, beta, infos.first().rgeGma(), itf.inten, inten_t(NAN), itf.tth, deg(NAN),
                itf.fwhm, fwhm_t(NAN)));
        }
    }

    return interpolatedInfos;
}
