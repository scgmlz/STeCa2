// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      core/data/suite.h
//! @brief     Defines class Suite
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef SUITE_H
#define SUITE_H

#include "typ/angles.h"
#include "data/image.h"

class Metadata;
class Measurement;
class Experiment;

//! One or more Measurement's

class Suite final : public vec<QSharedPointer<Measurement const>> {
private:
    friend class Experiment;

public:
    Suite();

    QSharedPointer<Metadata const> metadata() const;
    Experiment const& experiment() const;

    deg omg() const;
    deg phi() const;
    deg chi() const;

    Range rgeGma() const;
    Range rgeGmaFull() const;
    Range rgeTth() const;

    Range rgeInten() const;

    qreal avgMonitorCount() const;
    qreal avgDeltaMonitorCount() const;
    qreal avgDeltaTime() const;

    inten_vec collectIntens(Image const* intensCorr, Range const&) const;
    void calculateAlphaBeta(deg tth, deg gma, deg& alpha, deg& beta) const;

    size2d imageSize() const;

private:
    Experiment* experiment_; // here it belongs (or can be nullptr)
    QSharedPointer<Metadata const> md_; // on demand, compute once
};

Q_DECLARE_METATYPE(QSharedPointer<Suite>)

#endif // SUITE_H
