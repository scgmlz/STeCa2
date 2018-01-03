// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      core/typ/async.h
//! @brief     Defines classes TakesLongTime and Progress
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef ASYNC_H
#define ASYNC_H

#include "def/numbers.h"

//! As long as an instance of this class exists, we see the 'waiting' cursor.

class TakesLongTime final {
public:
    TakesLongTime();
    ~TakesLongTime();
};

//! Manages the progress bar.

class Progress final {
public:
    Progress(uint mulTotal, class QProgressBar*);
    ~Progress();

    void setTotal(uint);
    void setProgress(uint);
    void step();

private:
    uint total_, mulTotal_, i_;
    QProgressBar* bar_;
};

#endif // ASYNC_H
