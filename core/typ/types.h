// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      core/typ/types.h
//! @brief     Defines types inten_t, fwhm_t, inten_vec, eNorm, TO_(INT|DOUBLE)
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef TYPES_H
#define TYPES_H

#include "core/typ/vec.h"
#include "core/typ/exception.h"

typedef float inten_t;
typedef float fwhm_t;

typedef vec<inten_t> inten_vec;

enum class eNorm {
    NONE,
    MONITOR,
    DELTA_MONITOR,
    DELTA_TIME,
    BACKGROUND,
};

int TO_INT(const QString&) THROWS;
double TO_DOUBLE(const QString&) THROWS;

#endif // TYPES_H
