// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      core/fit/fit_fun.cpp
//! @brief     Implements classes Polynom and PeakFunction, and FunctionRegistry
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "def/idiomatic_for.h"
#include "fit_methods.h"
#include "typ/curve.h"

namespace { // file-scope functions

//! Compute a low power with an exponent of type uint
static qreal pow_n(qreal x, uint n) {
    qreal val = 1;
    while (n-- > 0)
        val *= x;
    return val;
}

} // file-scope functions

// ************************************************************************** //
//  class Polynom
// ************************************************************************** //

uint Polynom::degree() const {
    uint parCount = parameterCount();
    debug::ensure(parCount > 0);
    return parCount - 1;
}

void Polynom::setDegree(uint degree) {
    setParameterCount(degree + 1);
}

qreal Polynom::y(qreal x, qreal const* parValues) const {
    qreal val = 0, xPow = 1;
    for_i (parameters_.count()) {
        val += parValue(i, parValues) * xPow;
        xPow *= x;
    }
    return val;
}

qreal Polynom::dy(qreal x, uint i, qreal const*) const {
    return pow_n(x, i);
}

// REVIEW
qreal Polynom::avgY(Range const& rgeX, qreal const* parValues) const {
    debug::ensure(rgeX.isValid());

    qreal w = rgeX.width();
    if (w <= 0)
        return y(rgeX.min, parValues);

    qreal minY = 0, maqpair = 0, minPow = 1, maxPow = 1;
    for_i (parameters_.count()) {
        qreal facY = parValue(i, parValues) / (i + 1);
        minY += facY * (minPow *= rgeX.min);
        maqpair += facY * (maxPow *= rgeX.max);
    }

    return (1 / w) * (maqpair - minY);
}

void Polynom::fit(Curve const& curve, Ranges const& ranges) {
    FitWrapper().fit(*this, curve.intersect(ranges));
}

Polynom Polynom::fromFit(uint degree, Curve const& curve, Ranges const& ranges) {
    Polynom poly(degree);
    poly.fit(curve, ranges);
    return poly;
}

JsonObj Polynom::to_json() const {
    JsonObj ret = Function::to_json();
    ret.insert("type", name());
    return ret;
}

void Polynom::from_json(JsonObj const& obj) THROWS {
    Function::from_json(obj);
}

// ************************************************************************** //
//  class PeakFunction
// ************************************************************************** //

void PeakFunction::setRange(Range const& range) {
    range_ = range;
}

PeakFunction::PeakFunction() : guessedPeak_(), guessedFWHM_(NAN) {}

void PeakFunction::setGuessedPeak(qpair const& peak) {
    guessedPeak_ = peak;
}

void PeakFunction::setGuessedFWHM(fwhm_t fwhm) {
    guessedFWHM_ = fwhm;
}

void PeakFunction::reset() {
    Function::reset();
    setGuessedPeak(guessedPeak_);
    setGuessedFWHM(guessedFWHM_);
}

void PeakFunction::fit(Curve const& curve, Range const& range) {
    Curve c = prepareFit(curve, range);
    if (c.isEmpty())
        return;

    //  if (!guessedPeak().isValid()) {  // calculate guesses // TODO caching
    //  temporarily disabled, until it works correctly
    uint peakIndex = c.maqpairindex();
    auto peakTth = c.x(peakIndex);
    auto peakIntens = c.y(peakIndex);

    // half-maximum indices
    uint hmi1 = peakIndex, hmi2 = peakIndex;

    // left
    for (uint i = peakIndex; i-- > 0;) {
        hmi1 = i;
        if (c.y(i) < peakIntens / 2)
            break;
    }

    // right
    for (uint i = peakIndex, iCnt = c.count(); i < iCnt; ++i) {
        hmi2 = i;
        if (c.y(i) < peakIntens / 2)
            break;
    }

    setGuessedPeak(qpair(peakTth, peakIntens));
    setGuessedFWHM(c.x(hmi2) - c.x(hmi1));

    FitWrapper().fit(*this, c);
}

Curve PeakFunction::prepareFit(Curve const& curve, Range const& range) {
    reset();
    return curve.intersect(range);
}

JsonObj PeakFunction::to_json() const {
    JsonObj ret = Function::to_json();
    ret.insert("range", range_.to_json());
    ret.insert("guessed peak", guessedPeak_.to_json());
    ret.insert("guessed fwhm", qreal_to_json(guessedFWHM_));
    ret.insert("type", name());
    return ret;
}

void PeakFunction::from_json(JsonObj const& obj) THROWS {
    Function::from_json(obj);
    range_ = obj.loadRange("range");
    guessedPeak_.from_json(obj.loadObj("guessed peak"));
    guessedFWHM_ = obj.loadQreal("guessed fwhm");
}


// ************************************************************************** //
//  FunctionRegistry
// ************************************************************************** //

void FunctionRegistry::register_fct(const initializer_type f) {
    PeakFunction* tmp = f();
    register_item(tmp->name(), f);
};

PeakFunction* FunctionRegistry::name2new(QString const& peakFunctionName) {
    initializer_type make_new = instance()->find_or_fail(peakFunctionName);
    return make_new();
}

PeakFunction* FunctionRegistry::clone(PeakFunction const& old) {
    PeakFunction* ret = name2new(old.name());
    *ret = old;
    return ret;
}
