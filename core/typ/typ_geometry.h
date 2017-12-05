// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      core/typ/typ_geometry.h
//! @brief     Defines ...
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef TYP_GEOMETRY_H
#define TYP_GEOMETRY_H

#include "def/def_cmp.h"
#include "def/defs.h"
#include "typ/typ_array2d.h"
#include "typ/typ_ij.h"
#include "typ/typ_types.h"
#include <QSharedPointer>

namespace typ {

// detector geometry

struct Geometry {
    CLASS(Geometry)

    static preal const MIN_DETECTOR_DISTANCE;
    static preal const MIN_DETECTOR_PIXEL_SIZE;

    static preal const DEF_DETECTOR_DISTANCE;
    static preal const DEF_DETECTOR_PIXEL_SIZE;

    Geometry();

    COMPARABLE

    preal detectorDistance; // the distance from the sample to the detector
    preal pixSize; // size of the detector pixel
    IJ midPixOffset;
};

// image cut (margins)

struct ImageCut {
    CLASS(ImageCut)

    uint left, top, right, bottom;

    ImageCut();
    ImageCut(uint left, uint top, uint right, uint bottom);

    COMPARABLE

    size2d marginSize() const;
};

struct Angles {
    CLASS(Angles)

    tth_t tth;
    gma_t gma;

    Angles();
    Angles(tth_t, gma_t);
};

class AngleMap {
    CLASS(AngleMap)
public:
    struct Key {
        CLASS(Key)

        Key(Geometry::rc, size2d::rc, ImageCut::rc, IJ::rc midPix, tth_t midTth);

        COMPARABLE

        bool operator<(rc that) const { return compare(that) < 0; }

        Geometry geometry;
        size2d size;
        ImageCut cut;
        IJ midPix;
        tth_t midTth;
    };

    AngleMap(Key::rc);

    Angles::rc at(uint i) const { return arrAngles_.at(i); }

    Angles::rc at(uint i, uint j) const { return arrAngles_.at(i, j); }

    tth_rge rgeTth() const { return rgeTth_; }
    gma_rge rgeGma() const { return rgeGma_; }
    gma_rge rgeGmaFull() const { return rgeGmaFull_; }

    // TODO remove  IJ gmaPixel(gma_t);

    void getGmaIndexes(gma_rge::rc, uint_vec const*&, uint&, uint&) const;

private:
    void calculate();

    Key key_;

    Array2D<Angles> arrAngles_;

    tth_rge rgeTth_;
    gma_rge rgeGma_, rgeGmaFull_;

    // sorted
    vec<gma_t> gmas;
    uint_vec gmaIndexes;
};

typedef QSharedPointer<AngleMap> shp_AngleMap;
}
#endif // TYP_GEOMETRY_H
