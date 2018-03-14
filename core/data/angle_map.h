// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      core/data/angle_map.h
//! @brief     Defines classes ScatterDirection, AngleMap
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef ANGLE_MAP_H
#define ANGLE_MAP_H

#include "core/data/geometry.h"
#include <QSharedPointer> // no auto rm

//! Holds (gamma, 2theta) for all pixels in a detector image, and caches sorted gamma values.

class AngleMap {
public:
    AngleMap() = delete;
    AngleMap(const ImageKey&);

    const ScatterDirection& at(int i) const { return arrAngles_.at(i); }
    const ScatterDirection& at(int i, int j) const { return arrAngles_.at(i, j); }

    Range rgeTth() const { return rgeTth_; }
    Range rgeGma() const { return rgeGma_; }
    Range rgeGmaFull() const { return rgeGmaFull_; }

    void getGmaIndexes(const Range&, vec<int> const*&, int&, int&) const;

private:
    ImageKey key_;
    Array2D<ScatterDirection> arrAngles_;
    Range rgeTth_;
    Range rgeGma_, rgeGmaFull_;
    vec<deg> gmas_; //!< sorted gamma values
    vec<int> gmaIndexes_;
};

typedef QSharedPointer<AngleMap> shp_AngleMap;

#endif // ANGLE_MAP_H
