// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      core/typ/matrix.h
//! @brief     Defines the structs vec3f, vec3r, mat3r
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef MATRIX_H
#define MATRIX_H

#include "core/def/macros.h"

struct vec3r;

//! 3D vector with base type float.
struct vec3f {
    typedef const vec3f& rc;

    float _0, _1, _2;

    vec3f(float, float, float);
    vec3f(const vec3r&);

    bool operator==(const vec3f&) const;
};

//! 3D vector with base type qreal.
struct vec3r {
    typedef const vec3r& rc;

    qreal _0, _1, _2;

    vec3r(qreal, qreal, qreal);
    vec3r(const vec3f&);

    bool operator==(const vec3r&) const;
};

//! Rotation matrix in 3D, with base type qreal.
struct mat3r {
    typedef const mat3r& rc;

    qreal _00, _01, _02, _10, _11, _12, _20, _21, _22;

    mat3r(qreal, qreal, qreal, qreal, qreal, qreal, qreal, qreal, qreal);

    bool operator==(const mat3r&) const;

    void transpose();
    mat3r transposed() const;

    mat3r operator*(const mat3r&) const;
    vec3r operator*(const vec3r&) const;

    // factories
    static mat3r rotationCWx(qreal angle);
    static mat3r rotationCWz(qreal angle);
    static mat3r rotationCCWz(qreal angle);
};

#endif // MATRIX_H
