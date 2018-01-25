// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      core/data/corrset.cpp
//! @brief     Implements class Corrset
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "core/data/dataset.h"
#include "core/calc/reflection.h"
#include "core/calc/reflection_info.h"
#include "core/data/angle_map.h"
#include "core/data/rawfile.h"
#include "core/data/experiment.h"
#include "core/data/image.h"
#include "core/session.h"
#include "core/typ/async.h"
#include "core/typ/cache.h"
#include "core/typ/singleton.h"
#include "core/loaders/loaders.h"
#include <QDir>
#include <QSharedPointer>

void Corrset::clear() {
    enabled_ = true;
    hasNANs_ = false;
}

void Corrset::removeFile() {
    raw_.clear();
    corrImage_.clear();
    intensCorr_.clear();
    gSession->updateImageSize();
    emit gSession->sigCorr();
}

void Corrset::loadFile(rcstr filePath) THROWS {
    if (filePath.isEmpty())
        throw("invalid call of Corrset::loadFile with empty filePath argument");
    QSharedPointer<Rawfile> rawfile = load::loadRawfile(filePath);
    if (rawfile.isNull())
        return;
    gSession->setImageSize(rawfile->imageSize());
    corrImage_ = rawfile->foldedImage();
    intensCorr_.clear(); // will be calculated lazily
    // all ok
    raw_ = rawfile;
    enabled_ = true;
    emit gSession->sigCorr();
}

void Corrset::calcIntensCorr() const {
    hasNANs_ = false;

    debug::ensure(corrImage_);
    size2d size = corrImage_->size() - gSession->imageCut_.marginSize();
    debug::ensure(!size.isEmpty());

    int w = size.w, h = size.h, di = gSession->imageCut_.left, dj = gSession->imageCut_.top;

    qreal sum = 0;
    for_ij (w, h)
        sum += corrImage_->inten(i + di, j + dj);
    qreal avg = sum / (w * h);

    intensCorr_.fill(1, corrImage_->size());

    for_ij (w, h) {
        const inten_t inten = corrImage_->inten(i + di, j + dj);
        qreal fact;
        if (inten > 0) {
            fact = avg / inten;
        } else {
            fact = NAN;
            hasNANs_ = true;
        }
        intensCorr_.setInten(i + di, j + dj, inten_t(fact));
    }
}

const Image* Corrset::intensCorr() const {
    if (!enabled_)
        return nullptr;
    if (intensCorr_.isEmpty())
        calcIntensCorr();
    return &intensCorr_;
}

void Corrset::tryEnable(bool on) {
    if ((on && !hasFile()) || on==enabled_)
        return;
    enabled_ = on;
    emit gSession->sigCorr();
}