// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      core/calc/calc_lens.h
//! @brief     Defines ...
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef CALC_LENS_H
#define CALC_LENS_H

#include "data/data_dataset.h"
#include "def/defs.h"
#include "typ/typ_array2d.h"
#include "typ/typ_curve.h"
#include "typ/typ_geometry.h"
#include "typ/typ_image.h"
#include "typ/typ_image_transform.h"
#include "typ/typ_strlst.h"
#include "typ/typ_types.h"
#include <QSharedPointer>

namespace core {
class Session;
}

namespace calc {

// View the dataset through a lens (thanks, Antti!)

class LensBase {
public:
    LensBase(
        core::Session const&, data::Datasets::rc, bool trans, bool cut, typ::ImageTransform::rc,
        typ::ImageCut::rc);

    virtual typ::size2d size() const = 0;

protected:
    typ::size2d transCutSize(typ::size2d) const;

    void doTrans(uint& i, uint& j) const;
    void doCut(uint& i, uint& j) const;

    core::Session const& session_;
    data::Datasets::rc datasets_;
    bool trans_, cut_;
    typ::ImageTransform imageTransform_;
    typ::ImageCut imageCut_;
    typ::Image const* intensCorr_;
};

class ImageLens final : public LensBase {
    CLASS(ImageLens)
    SUPER(LensBase)
    public
        : ImageLens(core::Session const&, typ::Image::rc, data::Datasets::rc, bool trans, bool cut);

    typ::size2d size() const;

    inten_t imageInten(uint i, uint j) const;

    inten_rge::rc rgeInten(bool fixed) const;

private:
    typ::Image const& image_;

    mutable inten_rge rgeInten_;
};

typedef QSharedPointer<ImageLens> shp_ImageLens;

class DatasetLens final : public LensBase {
    CLASS(DatasetLens)
    SUPER(LensBase)
    public : DatasetLens(
                 core::Session const&, data::Dataset::rc, data::Datasets::rc, eNorm, bool trans,
                 bool cut, typ::ImageTransform::rc, typ::ImageCut::rc);

    typ::size2d size() const;

    gma_rge rgeGma() const;
    gma_rge rgeGmaFull() const;
    tth_rge rgeTth() const;
    inten_rge rgeInten() const;

    typ::Curve makeCurve() const;
    typ::Curve makeCurve(gma_rge::rc) const;

    data::Dataset::rc dataset() const { return dataset_; }

private:
    void setNorm(eNorm);
    inten_t normFactor_;

    data::Dataset::rc dataset_;
};

typedef QSharedPointer<DatasetLens> shp_DatasetLens;
}
#endif // CALC_LENS_H
