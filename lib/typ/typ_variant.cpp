// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      lib/typ/typ_variant.cpp
//! @brief     Implements ...
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "typ_variant.h"

#include "def/def_cmp_impl.h"
#include <QDate>
#include <QMetaType>

#include "test/tests.h"

namespace typ {

bool isNumeric(QVariant const& v) {
    auto type = QMetaType::Type(v.type());

    switch (type) {
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
    case QMetaType::Double:
    case QMetaType::Long:
    case QMetaType::Short:
    case QMetaType::ULong:
    case QMetaType::UShort:
    case QMetaType::Float: return true;
    default: return false;
    }
}

#define IMPL_CMP(name, toType)                                                                     \
    int name(QVariant const& v1, QVariant const& v2) {                                             \
        auto val1 = v1.toType(), val2 = v2.toType();                                               \
        RET_COMPARE_VALUE2(val1, val2)                                                             \
        return 0;                                                                                  \
    }

IMPL_CMP(cmp_int, toInt)
IMPL_CMP(cmp_str, toString)
IMPL_CMP(cmp_date, toDate)

int cmp_real(QVariant const& v1, QVariant const& v2) {
    auto val1 = v1.toDouble(), val2 = v2.toDouble();
    if (qIsNaN(val1)) {
        return qIsNaN(val2) ? 0 : +1;
    }
    if (qIsNaN(val2)) {
        return -1;
    }
    RET_COMPARE_VALUE2(val1, val2)
    return 0;
}

TEST("cmp_int(QVariants)", ({ // not exhaustive, just due diligence
         QVariant v1(1), v2(2);
         CHECK_EQ(0, cmp_int(v1, v1));
         CHECK_EQ(-1, cmp_int(v1, v2));
         CHECK_EQ(+1, cmp_int(v2, v1));
     });)

#undef IMPL_CMP
}
