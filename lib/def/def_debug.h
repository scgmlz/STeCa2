// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      lib/def/def_debug.h
//! @brief     Defines ...
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef DEF_DEBUG_H
#define DEF_DEBUG_H

#include <QtGlobal>

// debugging helpers

#ifndef QT_NO_DEBUG

#include <QDebug>

/* TRace: trace prints; takes several things separated by << e.g.
 * TR(var)
 * TR("name" << var1 << 2+3)
 * Note: there must be an available QDebug& operator<<
 */
#define TR(what)                                                                                   \
    { qDebug() << what; }

// asserts redefined, to include (or not) the ';'

#define EXPECT(cond) EXPECT2(cond, "")
#define ENSURE(cond) ENSURE2(cond, "")

// with a message

#define EXPECT2(cond, text) Q_ASSERT_X(cond, "precondition", text);
#define ENSURE2(cond, text) Q_ASSERT_X(cond, "postcondition", text);

// with a debug message

#define EXPECT_WT(cond, what)                                                                      \
    {                                                                                              \
        if (!(cond))                                                                               \
            WT(what);                                                                              \
        EXPECT(cond)                                                                               \
    }
#define ENSURE_WT(cond, what)                                                                      \
    {                                                                                              \
        if (!(cond))                                                                               \
            WT(what);                                                                              \
        ENSURE(cond)                                                                               \
    }

// marks code that should not be reached

#define NEVER Q_ASSERT_X(false, "Here", "not be!");

#else

#define TR(what)                                                                                   \
    {}

#define EXPECT(cond)
#define ENSURE(cond)

#define EXPECT2(cond, text)
#define ENSURE2(cond, text)

#define EXPECT_TR(cond, what)
#define ENSURE_TR(cond, what)

#define NEVER

#endif

// WaTch: same as TR, also prints stringized version (what is being printed)
#define WT(what) TR(#what ":" << what)

#endif // DEF_DEBUG_H
