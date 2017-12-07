// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      core/typ/typ_array2d.cpp
//! @brief     Implements ...
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
//  Unit tests in test004_array2d
//
// ************************************************************************** //

#include "typ_array2d.h"
#include "def/def_cmp_impl.h"

namespace typ {

int size2d::compare(rc that) const {
    RET_COMPARE_VALUE(w)
    RET_COMPARE_VALUE(h)
    return 0;
}

EQ_NE_OPERATOR(size2d)

} // namespace typ
