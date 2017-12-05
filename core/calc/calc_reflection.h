// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      core/calc/calc_reflection.h
//! @brief     Defines ...
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef CALC_REFLECTION_H
#define CALC_REFLECTION_H

#include "fit/fit_fun.h"
#include "typ/typ_strlst.h"
#include "typ/typ_types.h"
#include <QSharedPointer>

namespace calc {

class Reflection final {
    CLASS(Reflection)
public:
    static str_lst::rc typeStrLst();
    static rcstr typeTag(fit::ePeakType);

    Reflection(fit::ePeakType = fit::ePeakType::RAW);

    fit::ePeakType type() const;
    void setType(fit::ePeakType);

    fit::PeakFunction::rc peakFunction() const; // REMOVE

    typ::Range::rc range() const;
    void setRange(typ::Range::rc);

    void invalidateGuesses();

    void setGuessPeak(peak_t::rc peak) { peakFunction_->setGuessedPeak(peak); }
    void setGuessFWHM(fwhm_t fwhm) { peakFunction_->setGuessedFWHM(fwhm); }

    void fit(typ::Curve::rc);

private:
    void setPeakFunction(fit::ePeakType);
    void setPeakFunction(fit::PeakFunction*);

    scoped<fit::PeakFunction*> peakFunction_;

public:
    typ::JsonObj saveJson() const;
    void loadJson(typ::JsonObj::rc) THROWS;
};

typedef QSharedPointer<Reflection> shp_Reflection;
typedef typ::vec<shp_Reflection> Reflections;
}

Q_DECLARE_METATYPE(calc::shp_Reflection)

#endif // CORE_REFLECTION_H
