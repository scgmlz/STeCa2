// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      core/def/debug.h
//! @brief     Defines preprocessor macros and functions for debugging
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef DEBUG_H
#define DEBUG_H

#include <QtGlobal> // protect
#include <QDebug>

// TRace:
#define TR(what) { qDebug() << what; }

// WaTch: same as TR, also prints stringized version (what is being printed)
#define WT(what) TR(#what ":" << what)

namespace debug {
    void ensure(bool cond, const char* text="assertion failed");
}

#define NEVER { qFatal("fall-through bug"); }

#endif // DEBUG_H
