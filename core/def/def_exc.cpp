// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      core/def/def_exc.cpp
//! @brief     Implements class Exception
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "def_exc.h"

#ifdef QT_NO_EXCEPTIONS
#error needs exception handling
#endif

Exception::Exception(rcstr msg, bool silent) noexcept : silent_(silent) {
    setMsg(msg);
}

Exception::Exception() noexcept : Cls(EMPTY_STR, true) {}

Exception::Exception(rcstr msg) noexcept : Cls(msg, false) {}

Exception::Exception(rc that) noexcept : Cls(that.msg_) {}

pcstr Exception::what() const noexcept {
    return msg8bit_.constData();
}

void Exception::setMsg(rcstr s) {
    msg_ = s;
    msg8bit_ = msg_.toLocal8Bit();
}

Exception* Exception::clone() const {
    return new Exception(*this);
}

void Exception::raise() const {
    throw * this;
}
