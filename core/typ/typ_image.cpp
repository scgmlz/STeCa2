// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      core/typ/typ_image.cpp
//! @brief     Implements ...
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "typ/typ_image.h"

namespace typ {

Image::Image(size2d::rc size) {
    fill(0, size);
}

Image::Image(inten_arr::rc that) : Image(that.size()) {
    addIntens(that);
}

void Image::addIntens(rc that) THROWS {
    RUNTIME_CHECK(size() == that.size(), "inconsistent image size");
    auto w = size().w, h = size().h;
    for (uint i = 0; i < w; ++i)
        for (uint j = 0; j < h; ++j) {
            inten_t inten = that.inten(i, j);
            rgeInten_.extendBy(inten);
            addInten(i, j, inten);
        }
}
}
