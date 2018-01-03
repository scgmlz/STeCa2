// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      core/data/angle_map.h
//! @brief     Defines classes Angles, AngleMap
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef ANGLE_MAP_H
#define ANGLE_MAP_H

#include "typ/angles.h"
#include "data/geometry.h"

class AnglePair {
public:
    deg tth;
    deg gma;

    AnglePair();
    AnglePair(deg, deg);
};

class AngleMap {
public:
    AngleMap() = delete;
    AngleMap(ImageKey const&);

    AnglePair const& at(uint i) const { return arrAngles_.at(i); }
    AnglePair const& at(uint i, uint j) const { return arrAngles_.at(i, j); }

    Range rgeTth() const { return rgeTth_; }
    Range rgeGma() const { return rgeGma_; }
    Range rgeGmaFull() const { return rgeGmaFull_; }

    void getGmaIndexes(Range const&, uint_vec const*&, uint&, uint&) const;

private:
    void calculate();

    ImageKey key_;

    Array2D<AnglePair> arrAngles_;

    Range rgeTth_;
    Range rgeGma_, rgeGmaFull_;

    vec<deg> gmas; //!< sorted gamma values
    uint_vec gmaIndexes;
};

typedef QSharedPointer<AngleMap> shp_AngleMap;

#endif // ANGLE_MAP_H
