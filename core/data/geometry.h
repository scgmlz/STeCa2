// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      core/data/geometry.h
//! @brief     Defines classes Geometry, ImageCut, ImageKey
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "core/typ/angles.h"
#include "core/typ/array2d.h"
#include "core/typ/ij.h"
#include "core/typ/range.h"

//! Detector geometry.
class Geometry {
public:
    static qreal const MIN_DETECTOR_DISTANCE;
    static qreal const MIN_DETECTOR_PIXEL_SIZE;

    static qreal const DEF_DETECTOR_DISTANCE;
    static qreal const DEF_DETECTOR_PIXEL_SIZE;

    Geometry();

    COMPARABLE(Geometry const&);

    qreal detectorDistance; // the distance from the sample to the detector
    qreal pixSize; // size of the detector pixel
    IJ midPixOffset;
};

//! Image cut (margins)
class ImageCut {
public:
    int left, top, right, bottom;

    ImageCut();
    ImageCut(int left, int top, int right, int bottom);
    COMPARABLE(ImageCut const&);
    void update(bool topLeftFirst, bool linked, ImageCut const& cut, size2d size);

    size2d marginSize() const;
};

//! Holds geometry parameters that define a mapping of image coordinates onto (gamma,2theta).

//! Needed for caching such coordinate maps.
class ImageKey {
public:
    ImageKey(Geometry const&, size2d const&, ImageCut const&, IJ const& midPix, deg midTth);

    COMPARABLE(ImageKey const&);
    bool operator<(ImageKey const& that) const { return compare(that) < 0; }

    Geometry geometry;
    size2d size;
    ImageCut cut;
    IJ midPix;
    deg midTth;
};

#endif // GEOMETRY_H
