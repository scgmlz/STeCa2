// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      core/calc/gamma_selection.cpp
//! @brief     Implements GammaSelection
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "gamma_selection.h"
#include "core/session.h"

GammaSelection::GammaSelection()
{
}

//! Resets fullRange_ according to loaded data.
void GammaSelection::onData()
{
    const Cluster* cluster = gSession->dataset().highlight().cluster();
    if (!cluster) {
        fullRange_.invalidate();
        return;
    }
    fullRange_ = cluster->rgeGma();
     recomputeCache();
}

//! Recomputes range_ and iSlice_.
void GammaSelection::recomputeCache()
{
    if (!fullRange_.isValid()) {
        range_.invalidate();
        numSlices_ = 0;
        return;
    }
    if        (mode_ == Mode::all) {
        range_ = fullRange_;
    } else if (mode_ == Mode::slicing) {
        iSlice_ = qMin(qMax(iSlice_, 0), numSlices_-1);
        if (numSlices_)
            range_ = fullRange_.slice(iSlice_, numSlices_);
    } else if (mode_ == Mode::minmax) {
        range_ = range_.intersect(fullRange_);
    }
    emit gSession->sigGamma();
}

void GammaSelection::setModeTakeAll()
{
    mode_ = Mode::all;
    iSlice_ = 0;
    recomputeCache();
}

void GammaSelection::setModeSlicing()
{
    mode_ = Mode::slicing;
    recomputeCache();
}

void GammaSelection::setModeMinMax()
{
    mode_ = Mode::all;
    iSlice_ = 0;
    recomputeCache();
}

void GammaSelection::setNumSlices(int n)
{
    if (n<=0) {
        setModeTakeAll();
    } else {
        setModeSlicing();
        numSlices_ = qMax(n, 1);
        selectSlice(iSlice_);
    }
}

void GammaSelection::selectSlice(int i)
{
    setModeSlicing();
    iSlice_ = i;
    recomputeCache();
}

void GammaSelection::setRange(const Range& r)
{
    setModeMinMax();
    range_ = r;
    recomputeCache();
}
