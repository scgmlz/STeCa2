// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      core/typ/vec.h
//! @brief     Defines class vec
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef VEC_H
#define VEC_H

#include "def/numbers.h"

//! limited QVector, only needed methods reexported

template <typename T> class vec : protected QVector<T> {
private:
    using super = QVector<T>;
public:
    vec() : super() {}
    vec(std::initializer_list<T> l) : super(l) {}

    explicit vec(uint count) : super(to_i(count)) {}
    explicit vec(uint count, T const& init) : super(to_i(count), init) {}

    uint count() const { return to_u(super::count()); }
    void reserve(uint n) { super::reserve(to_i(n)); }

    super const& sup() const { return *this; }
    using super::clear;
    using super::isEmpty;
    using super::begin;
    using super::end;
    using super::cbegin;
    using super::cend;
    using super::data;
    using super::first;

    vec& fill(T const& init) { return static_cast<vec&>(super::fill(init)); }

    vec& fill(T const& init, uint count) {
        return static_cast<vec&>(super::fill(init, to_i(count)));
    }

    void resize(uint count) { super::resize(to_i(count)); }
    void append(T const& that) { *this += that; }
    void append(vec const& that) { *this += that; }
    void remove(uint i) { super::remove(to_i(i)); }

    T const& at(uint i) const { return super::at(to_i(i)); }
    T& operator[](uint i) { return super::operator[](to_i(i)); }
};

// most useful vectors (that's why they are in the global namespace)
typedef vec<qreal> qreal_vec;
typedef vec<uint> uint_vec;

#endif // VEC_H
