//  ***********************************************************************************************
//
//  Steca: stress and texture calculator
//
//! @file      core/pars/baseline.cpp
//! @brief     Implements class Baseline
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
//  ***********************************************************************************************

#include "core/session.h"
#include "qcr/base/debug.h"

void Baseline::fromJson(const JsonObj obj)
{
    ranges.fromJson(obj.loadArr("ranges"));
    polynomDegree.setVal(obj.loadUint("polynom degree"));
}

void Baseline::clear()
{
    ranges.clear();
    polynomDegree.setVal(0);
}

QJsonObject Baseline::toJson() const
{
    QJsonObject ret;
    ret.insert("ranges", ranges.toJson());
    ret.insert("polynom degree", polynomDegree.val());
    return ret;
}