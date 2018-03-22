// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      core/typ/range.h
//! @brief     Defines classes Range and Ranges
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef RANGE_H
#define RANGE_H

#include "core/def/comparable.h"
#include "core/typ/exception.h"
#include "core/def/numbers.h"
#include <QJsonArray>

class JsonObj;

//! a range of values - a closed interval
class Range {
public:
    Range(); //!< invalid (NaN)
    Range(qreal min, qreal max); //!< normal

    static Range infinite(); //!< factory: -inf .. +inf

    COMPARABLE(const Range&)

    void invalidate(); //!< make invalid
    bool isValid() const; //!< is not NaN
    bool isEmpty() const; //!< is invalid or empty

    qreal width() const;
    qreal center() const;
    Range slice(int i, int n) const;

    qreal min, max; // this is the range

    void set(qreal min, qreal max); //!< must be: min <= max
    void safeSet(qreal, qreal); //!< will be set in the right order min/max

    static Range safeFrom(qreal, qreal); //!< safe factory

    void extendBy(qreal); //!< extend to include the number
    void extendBy(const Range&); //!< extend to include the range

    bool contains(qreal) const;
    bool contains(const Range&) const;
    bool intersects(const Range&) const;
    Range intersect(const Range&) const;

    qreal bound(qreal) const; //!< limit the number to the interval, as qBound would

    QJsonObject toJson() const;
    void fromJson(const JsonObj&) THROWS;

    QString to_s(int precision=5, int digitsAfter=2) const;
};

//! A set of *sorted* *non-overlapping* ranges
class Ranges {
public:
    Ranges() {}

    void clear() { ranges_.clear(); }
    bool isEmpty() const { return ranges_.isEmpty(); }
    int count() const { return ranges_.count(); }

    const Range& at(int i) const { return ranges_.at(i); }

    bool add(const Range&); //!< collapses overlapping ranges; returns true if *this changed
    bool remove(const Range&); //!< removes (cuts out) a range; returns whether there was a change

    QJsonArray toJson() const;
    void fromJson(const QJsonArray&) THROWS;

private:
    void sort();
    QVector<Range> ranges_;
};

#endif // RANGE_H
