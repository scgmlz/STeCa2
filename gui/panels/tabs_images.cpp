// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      gui/panels/tabs_images.cpp
//! @brief     Implements class TabsImages
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "tabs_images.h"
#include "cfg/colors.h"
#include "data/suite.h"
#include "data/measurement.h"
#include "cfg/gui_cfg.h"
#include "thehub.h"
#include "session.h"
#include "widgets/widget_makers.h"
#include <qmath.h>
#include <QAction>
#include <QPainter>

namespace gui {
namespace panel {

// ************************************************************************** //
//  file-scoped class ImageWidget
// ************************************************************************** //

class ImageWidget : public QWidget {
public:
    ImageWidget();

    void setPixmap(QPixmap const&);
    void setScale();

protected:
    void resizeEvent(QResizeEvent*);

private:
    qreal scale_;
    QPixmap original_, scaled_;

    void paintEvent(QPaintEvent*);
};

ImageWidget::ImageWidget() : scale_(0) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(gHub->toggle_showOverlay, &QAction::toggled, [this]() { update(); });

    connect(gHub->toggle_stepScale, &QAction::toggled, [this]() { setScale(); });
}

void ImageWidget::setPixmap(QPixmap const& pixmap) {
    original_ = pixmap;
    setScale();
}

void ImageWidget::setScale() {
    if (original_.isNull()) {
        scale_ = 0;
    } else {
        auto sz = size();
        auto os = original_.size();
        scale_ = qMin(qreal(sz.width() - 2) / os.width(), qreal(sz.height() - 2) / os.height());
    }

    if (gHub->toggle_stepScale->isChecked() && scale_ > 0)
        scale_ = (scale_ >= 1) ? qFloor(scale_) : 1.0 / qCeil(1.0 / scale_);

    if (original_.isNull() || !(scale_ > 0))
        scaled_ = QPixmap();
    else
        scaled_ = original_.scaled(original_.size() * scale_);

    update();
}

void ImageWidget::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    setScale();
}

void ImageWidget::paintEvent(QPaintEvent*) {
    // paint centered
    auto margin = (size() - scaled_.size()) / 2;
    QRect rect(QPoint(margin.width(), margin.height()), scaled_.size());

    QPainter p(this);

    // image
    p.drawPixmap(rect.left(), rect.top(), scaled_);

    // overlay
    if (gHub->toggle_showOverlay->isChecked()) {
        p.setPen(Qt::lightGray);

        // cut
        auto cut = gSession->imageCut();
        QRect r = rect.adjusted(-1, -1, 0, 0)
                      .adjusted(
                          qRound(scale_ * cut.left), qRound(scale_ * cut.top),
                          -qRound(scale_ * cut.right), -qRound(scale_ * cut.bottom));
        p.drawRect(r);

        QPoint rc;
        rc = r.center();
        int rcx = rc.x(), rcy = rc.y();

        int rl, rt, rr, rb;
        r.getCoords(&rl, &rt, &rr, &rb);
        int rw = rr - rl;

        // cross
        auto off = gSession->geometry().midPixOffset;
        auto x = qRound(rcx + scale_ * off.i);
        auto y = qRound(rcy + scale_ * off.j);
        p.drawLine(x, rt, x, rb);
        p.drawLine(rl, y, rr, y);

        // text annotations
        auto paintText = [this, &p](QPoint pos, rcstr s, bool alignLeft) {
            auto fm = fontMetrics();
            if (alignLeft)
                pos.rx() -= fm.width(s);
            p.drawText(pos, s);
        };

        p.setPen(Qt::cyan);
        paintText(QPoint(rr - rw / 5, rcy), "γ=0", false);
    }

    // frame
    p.setPen(Qt::black);
    p.drawRect(rect.adjusted(-1, -1, 0, 0));
}


// ************************************************************************** //
//  class TabsImages
// ************************************************************************** //

TabsImages::TabsImages() : TabsPanel() {
    {
        auto& box = addTab("Image", Qt::Vertical).box();

        auto hb = hbox();
        box.addLayout(hb);
        box.setAlignment(hb, Qt::AlignTop);

        hb->addWidget(iconButton(gHub->toggle_fixedIntenImage));
        hb->addWidget(iconButton(gHub->toggle_stepScale));
        hb->addWidget(iconButton(gHub->toggle_showOverlay));
        hb->addWidget((spinN_ = spinCell(gui_cfg::em4, 1)));

        hb->addStretch(1);

        hb->addWidget(iconButton(gHub->toggle_showBins));
        hb->addWidget(label("γ count"));
        hb->addWidget((numSlices_ = spinCell(gui_cfg::em4, 0)));
        hb->addWidget(label("#"));
        hb->addWidget((numSlice_ = spinCell(gui_cfg::em4, 1)));

        hb->addWidget(label("min"));
        hb->addWidget((minGamma_ = spinDoubleCell(gui_cfg::em4_2)));
        hb->addWidget(label("max"));
        hb->addWidget((maxGamma_ = spinDoubleCell(gui_cfg::em4_2)));

        minGamma_->setReadOnly(true);
        maxGamma_->setReadOnly(true);

        hb->addWidget(label("bin#"));
        hb->addWidget((numBin_ = spinCell(gui_cfg::em4, 1)));

        box.addWidget((dataImageWidget_ = new ImageWidget()));

        connect(spinN_, slot(QSpinBox, valueChanged, int), [this]() { render(); });

        connect(numSlices_, slot(QSpinBox, valueChanged, int), [this]() { render(); });

        connect(numSlice_, slot(QSpinBox, valueChanged, int), [this]() { render(); });

        connect(numBin_, slot(QSpinBox, valueChanged, int), [this]() { render(); });
    }

    {
        auto& tab = addTab("Correction", Qt::Vertical);

        connect(gHub, &TheHubSignallingBase::sigCorrFile,
                [&tab](QSharedPointer<Datafile const> file) { tab.setEnabled(!file.isNull()); });

        auto& box = tab.box();

        auto hb = hbox();
        box.addLayout(hb);
        box.setAlignment(hb, Qt::AlignTop);

        hb->addWidget(iconButton(gHub->toggle_fixedIntenImage));
        hb->addWidget(iconButton(gHub->toggle_stepScale));
        hb->addWidget(iconButton(gHub->toggle_showOverlay));
        hb->addStretch(1);

        box.addWidget((corrImageWidget_ = new ImageWidget()));
    }

    connect(gHub->toggle_enableCorr, &QAction::toggled, [this](bool) { render(); });

    connect(gHub->toggle_showBins, &QAction::toggled, [this]() { render(); });

    connect(gHub, &TheHubSignallingBase::sigDisplayChanged, [this](){ render(); });
    connect(gHub, &TheHubSignallingBase::sigGeometryChanged, [this](){ render(); });
    connect(gHub, &TheHubSignallingBase::sigNormChanged, [this](){ render(); });
    connect(gHub, &TheHubSignallingBase::sigSuiteSelected,
            [this](QSharedPointer<Suite> dataseq){ setSuite(dataseq); });

    render();
}

QPixmap TabsImages::makeBlankPixmap() {
    auto size = gSession->imageSize();

    QPixmap pixmap(to_i(size.w), to_i(size.h));
    pixmap.fill(QColor(0, 0, 0, 0));

    return pixmap;
}

QImage TabsImages::makeImage(QSharedPointer<Image> image, bool curvedScale) {
    QImage im;
    if (!image)
        return im;

    auto imageLens = gSession->imageLens(*image, gSession->experiment(), true, false);
    auto size = imageLens->size();
    if (size.isEmpty())
        return im;

    im = QImage(QSize(to_i(size.w), to_i(size.h)), QImage::Format_RGB32);

    auto rgeInten = imageLens->rgeInten(gHub->isFixedIntenImageScale());
    inten_t maxInten = inten_t(rgeInten.max);

    for_ij (size.w, size.h)
        im.setPixel(
            to_i(i), to_i(j), intenImage(imageLens->imageInten(i, j), maxInten, curvedScale));
    return im;
}

QPixmap TabsImages::makePixmap(QSharedPointer<Image> image) {
    return QPixmap::fromImage(makeImage(image, !gHub->isFixedIntenImageScale()));
}

QPixmap TabsImages::makePixmap(
    Measurement const& dataseq, Range const& rgeGma, Range const& rgeTth) {
    auto im = makeImage(dataseq.image(), !gHub->isFixedIntenImageScale());
    auto angleMap = gSession->angleMap(dataseq);

    auto size = im.size();
    for_ij (size.width(), size.height()) {
        AnglePair const& a = angleMap->at(to_u(i), to_u(j));
        auto color = QColor(im.pixel(i, j));
        if (rgeGma.contains(a.gma)) {
            if (rgeTth.contains(a.tth)) {
                color = Qt::yellow;
            } else {
                color.setGreen(qFloor(color.green() * .3 + 255 * .7));
            }
        } else if (rgeTth.contains(a.tth)) {
            color.setGreen(qFloor(color.green() * .3 + 255 * .7));
        }
        im.setPixel(i, j, color.rgb());
    }

    return QPixmap::fromImage(im);
}

void TabsImages::setSuite(QSharedPointer<Suite> dataseq) {
    dataseq_ = dataseq;
    render();
}

void TabsImages::render() {
    {
        QPixmap pixMap;

        uint nSlices = to_u(numSlices_->value());
        numSlice_->setMaximum(qMax(1, to_i(nSlices)));
        numSlice_->setEnabled(nSlices > 0);

        if (dataseq_) {
            // 1 - based
            uint by = qBound(1u, uint(gHub->suiteGroupedBy()), dataseq_->count());
            uint n = qBound(1u, to_u(spinN_->value()), by);

            spinN_->setValue(to_i(n));
            spinN_->setEnabled(by > 1);

            lens_ = gSession->defaultDatasetLens(*dataseq_);

            Range rge;
            if (nSlices > 0) {
                uint nSlice = qMax(1u, to_u(numSlice_->value()));
                uint iSlice = nSlice - 1;

                auto rgeGma = lens_->rgeGma();
                auto min = rgeGma.min;
                auto wn = rgeGma.width() / nSlices;

                rge = Range(min + iSlice * wn, min + (iSlice + 1) * wn);

                minGamma_->setValue(rge.min);
                maxGamma_->setValue(rge.max);
            } else {
                rge = Range::infinite();
                minGamma_->clear();
                maxGamma_->clear();
            }

            gHub->setGammaRange(rge);

            auto measurement = dataseq_->at(n - 1);

            numBin_->setEnabled(true);
            if (gHub->toggle_showBins->isChecked()) {
                Range rgeTth = lens_->rgeTth();
                auto curve = lens_->makeCurve();
                     // had argument averaged=false
                     // TODO factor out lens::binCount()
                int count = to_i(curve.count());
                numBin_->setMaximum(count - 1);
                auto min = rgeTth.min, wdt = rgeTth.width();
                qreal num = qreal(numBin_->value());
                pixMap = makePixmap(
                    *measurement, rge,
                    Range(min + wdt * (num / count), min + wdt * ((num + 1) / count)));
            } else {
                pixMap = makePixmap(measurement->image());
            }
        } else {
            spinN_->setEnabled(false);
            numBin_->setMaximum(0);
            numBin_->setEnabled(false);

            pixMap = makeBlankPixmap();
        }

        dataImageWidget_->setPixmap(pixMap);
    }

    {
        QPixmap pixMap = makePixmap(gSession->corrImage());
        corrImageWidget_->setPixmap(pixMap);
    }
}

} // namespace panel
} // namespace gui
