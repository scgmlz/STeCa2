// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      core/data/measurement.h
//! @brief     Defines class Measurement
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include "typ/angles.h"
#include "data/image.h"

class Metadata;

//! A Measurement consts of an Image with associated Metadata

class Measurement final {

public:
    Measurement(Metadata const&, size2d const&, inten_vec const&);
    Measurement(Measurement const&) = delete;

    QSharedPointer<Metadata const> metadata() const;

    deg midTth() const;

    qreal monitorCount() const;
    qreal deltaMonitorCount() const;
    qreal deltaTime() const;

    deg omg() const;
    deg phi() const;
    deg chi() const;

    Range rgeGma() const;
    Range rgeGmaFull() const;
    Range rgeTth() const;

    Range rgeInten() const;

    QSharedPointer<Image> image() const { return image_; }
    size2d imageSize() const;

    void collectIntens(Image const* intensCorr, inten_vec&, uint_vec&, Range const&,
                       deg minTth, deg deltaTth) const;

private:
    QSharedPointer<Metadata const> md_;
    QSharedPointer<Image> image_;
};

#endif // MEASUREMENT_H
