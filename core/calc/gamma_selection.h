// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      core/calc/gamma_selection.h
//! @brief     Defines GammaSelection
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef GAMMA_SELECTION_H
#define GAMMA_SELECTION_H

#include "core/typ/range.h"

//! Supports different ways of setting a gamma range.

class GammaSelection : public QObject {
public:
    GammaSelection();

    void onData();

    void setModeTakeAll();
    void setModeSlicing();
    void setModeMinMax();

    void setNumSlices(int);
    void selectSlice(int);
    void setRange(const Range&);

    const Range& range() const { return range_; }
    qreal min() const { return range_.min; }
    qreal max() const { return range_.max; }
    int numSlices() const { return numSlices_; }
    int idxSlice() const { return iSlice_; }
    bool isModeMinMax() const { return mode_==Mode::minmax; }

private:
    void recomputeCache();
    enum class Mode { all, slicing, minmax } mode_ {Mode::slicing};
    Range fullRange_;
    Range range_;
    int numSlices_ {1};
    int iSlice_ {0};
};

#endif // GAMMA_SELECTION_H
