// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      core/typ/array2d.h
//! @brief     Defines and mostly implements size2d and Array2D
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef ARRAY2D_H
#define ARRAY2D_H

#include "core/def/comparable.h"
#include "core/def/numbers.h"

//! Geometry of a rectangle
struct size2d {
    int w, h;

    size2d() : size2d(0, 0) {}
    size2d(int w_, int h_) : w(w_), h(h_) {}

    bool isEmpty() const { return 0 == w && 0 == h; }

    int count() const { return w * h; }

    COMPARABLE(size2d const&)

    friend size2d operator-(size2d const& s1, size2d const& s2) {
        int w = s1.w - s2.w;
        int h = s1.h - s2.h;
        return size2d(qMax(w, 0), qMax(h, 0));
    }

    size2d transposed() const { return size2d(h, w); }
};

//! 2D (indexed by int i/j) array
template <typename T> class Array2D {
    private:
    size2d size_;

    typedef T* col_t;
    col_t* ts_;

    void alloc(size2d const& size) {
        free();

        if (!(size_ = size).isEmpty()) {
            ts_ = static_cast<col_t*>(::calloc(size_.w, sizeof(col_t*)));
            for (int i = 0; i < size_.w; ++i)
                ts_[i] = static_cast<col_t>(::calloc(size_.h, sizeof(T)));
        }
    }

    void free() {
        if (ts_) {
            for (int i = 0; i < size_.w; ++i)
                ::free(ts_[i]);
            ::free(ts_);
            ts_ = nullptr;
        }
    }

public:
    // empty array
    Array2D() : size_(0, 0), ts_(nullptr) {}

    Array2D(Array2D const&) = delete;
    Array2D& operator=(Array2D const&) = delete;

    virtual ~Array2D() { free(); }

    // is empty?
    bool isEmpty() const { return size_.isEmpty(); }

    // 2D image size
    size2d const& size() const { return size_; }

    // make empty
    void clear() { alloc(size2d(0, 0)); }

    // allocate and fill with a value
    void fill(T const& val, size2d const& size) {
        alloc(size);
        for (int i = 0; i < size_.w; ++i)
            for (int j = 0; j < size_.h; ++j)
                ts_[i][j] = val;
    }

    // allocate and fill with a default value
    void resize(size2d const& size) { fill(T(), size); }

    // access using 1D index
    T const& at(int i) const { return at(i % size_.w, i / size_.h); }

    // access using 2D index
    T const& at(int i, int j) const {
        debug::ensure(i < size_.w && j < size_.h);
        return ts_[i][j];
    }

    // set using 1D index
    void setAt(int i, T const& val) { setAt(i % size_.w, i / size_.h, val); }

    // set using 2D index
    void setAt(int i, int j, T const& val) {
        debug::ensure(i < size_.w && j < size_.h);
        ts_[i][j] = val;
    }

    // mutable access
    T& refAt(int i, int j) const {
        debug::ensure(i < size_.w && j < size_.h);
        return ts_[i][j];
    }
};

#endif // ARRAY2D_H
