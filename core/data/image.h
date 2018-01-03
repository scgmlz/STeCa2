// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      core/data/image.h
//! @brief     Defines class Image
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef IMAGE_H
#define IMAGE_H

#include "typ/array2d.h"
#include "typ/range.h"
#include "typ/types.h"

typedef Array2D<inten_t> inten_arr;

class Image final {
    public:
    // Image as vector of intensities, filled with 0 or given intensities.
    Image(size2d const& = size2d(0, 0));
    Image(inten_arr const&);

    size2d const& size() const { return intens_.size(); }

    void clear() { intens_.clear(); }

    bool isEmpty() const { return intens_.isEmpty(); }

    void fill(inten_t val, size2d const& size) { intens_.fill(val, size); }

    inten_t inten(uint i) const { return intens_.at(i); }

    inten_t inten(uint i, uint j) const { return intens_.at(i, j); }

    void setInten(uint i, inten_t val) { intens_.setAt(i, val); }

    void setInten(uint i, uint j, inten_t val) { intens_.setAt(i, j, val); }

    void addInten(uint i, uint j, inten_t val) { intens_.refAt(i, j) += val; }

    // Sum all intensities with new ones.
    void addIntens(Image const&) THROWS;

    Range const& rgeInten() const { return rgeInten_; }

private:
    inten_arr intens_;
    Range rgeInten_;
};

#endif // IMAGE_H
