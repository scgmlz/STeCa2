// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      core/data/image.cpp
//! @brief     Implements class Image
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "data/image.h"

Image::Image(size2d const& size) {
    fill(0, size);
}

Image::Image(inten_arr const& that) : Image(that.size()) {
    addIntens(that);
}

void Image::addIntens(Image const& that) THROWS {
    RUNTIME_CHECK(size() == that.size(), "inconsistent image size");
    auto w = size().w, h = size().h;
    for (uint i = 0; i < w; ++i)
        for (uint j = 0; j < h; ++j) {
            inten_t inten = that.inten(i, j);
            rgeInten_.extendBy(inten);
            addInten(i, j, inten);
        }
}
