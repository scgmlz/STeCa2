// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      core/def/macros.h
//! @brief     Defines macro THROWS, sets some compiler pragmas
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef MACROS_H
#define MACROS_H

#include <QtGlobal> // to define Q_OS_WIN

#ifdef Q_CC_GNU

#pragma GCC diagnostic ignored "-Wpragmas"

// for clang static analyser
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wswitch-enum"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wpadded"
#pragma GCC diagnostic ignored "-Wcomment"

#endif

// exception specification macro
#ifdef Q_OS_WIN

#define THROWS

#else

#define THROWS noexcept(false)

#endif

#endif // MACROS_H
