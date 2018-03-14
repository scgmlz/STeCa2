// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      core/typ/realpair.cpp
//! @brief     Implements class qpair
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
//  Unit tests in test002_qpair
//
// ************************************************************************** //

#include "core/def/comparators.h"
#include "core/typ/json.h"
#include "core/typ/realpair.h"

int qpair::compare(const qpair& that) const {
    ASSERT(isValid() && that.isValid());
    RET_COMPARE_VALUE(x)
    RET_COMPARE_VALUE(y)
    return 0;
}

VALID_EQ_NE_OPERATOR(qpair)

void qpair::invalidate() {
    x = y = NAN;
}

QJsonObject qpair::to_json() const {
    return { { "x", qreal_to_json(x) }, { "y", qreal_to_json(y) } };
}

void qpair::from_json(const JsonObj& obj) THROWS {
    x = obj.loadQreal("x");
    y = obj.loadQreal("y");
}
