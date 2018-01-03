// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      gui/panels/panel_diffractogram.cpp
//! @brief     Implements classes DiffractogramPlot and Diffractogram.
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "panel_diffractogram.h"
#include "actions.h"
#include "data/suite.h"
#include "fit/fit_fun.h"
#include "cfg/gui_cfg.h"
#include "session.h"
#include "thehub.h"
#include "widgets/widget_makers.h"
#include "QCustomPlot/qcustomplot.h"
#include <QAction>

namespace gui {
namespace panel {

// ************************************************************************** //
//  define file-scoped classes
// ************************************************************************** //

class DiffractogramPlotOverlay : public QWidget {
public:
    DiffractogramPlotOverlay(DiffractogramPlot&);

    void setMargins(int left, int right);

private:
    DiffractogramPlot& plot_;

    QColor addColor_, remColor_, color_, bgColor_, reflColor_;
    int marginLeft_, marginRight_;

protected:
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);

    void paintEvent(QPaintEvent*);

    bool hasCursor_, mouseDown_;
    int cursorPos_, mouseDownPos_;

    void updateCursorRegion();
};

class DiffractogramPlot : public QCustomPlot {
public:
    enum class eTool {
        NONE,
        BACKGROUND,
        PEAK_REGION,
    };

    DiffractogramPlot(class Diffractogram&);

public:
    void setTool(eTool);
    eTool getTool() const { return tool_; }

    void plot(Curve const&, Curve const&, Curve const&, curve_vec const&, uint);

    Range fromPixels(int, int);

    void clearBg();
    void addBg(Range const&);
    void remBg(Range const&);
    void setNewReflRange(Range const&);
    void updateBg();

    void clearReflLayer();

    QColor bgRgeColor_, reflRgeColor_;
    eFittingTab selectedFittingTab();

    void enterZoom(bool);

protected:
    void addBgItem(Range const&);
    void resizeEvent(QResizeEvent*);

private:
    Diffractogram& diffractogram_;

    eTool tool_;
    bool showBgFit_;

    QCPGraph *bgGraph_, *dgramGraph_, *dgramBgFittedGraph_, *dgramBgFittedGraph2_, *guesses_,
        *fits_;

    vec<QCPGraph*> reflGraph_;
    DiffractogramPlotOverlay* overlay_;
    void onReflectionData(shp_Reflection reflection);
};

// ************************************************************************** //
//  class DiffractogramPlotOverlay (file scoped)
// ************************************************************************** //

DiffractogramPlotOverlay::DiffractogramPlotOverlay(DiffractogramPlot& plot_)
    : QWidget(&plot_)
    , plot_(plot_)
    , hasCursor_(false)
    , mouseDown_(false)
    , cursorPos_(0)
    , mouseDownPos_(0) {
    setMouseTracking(true);
    setMargins(0, 0);

    remColor_ = QColor(0xf8, 0xf8, 0xff, 0x90);
    bgColor_ = QColor(0x98, 0xfb, 0x98, 0x70);
    reflColor_ = QColor(0x87, 0xce, 0xfa, 0x70);
}

void DiffractogramPlotOverlay::setMargins(int left, int right) {
    marginLeft_ = left;
    marginRight_ = right;
}

void DiffractogramPlotOverlay::enterEvent(QEvent*) {
    hasCursor_ = true;
    updateCursorRegion();
}

void DiffractogramPlotOverlay::leaveEvent(QEvent*) {
    hasCursor_ = false;
    updateCursorRegion();
}

void DiffractogramPlotOverlay::mousePressEvent(QMouseEvent* e) {
    mouseDownPos_ = cursorPos_;
    mouseDown_ = true;
    addColor_ = (eFittingTab::BACKGROUND == plot_.selectedFittingTab()) ? bgColor_ : reflColor_;
    color_ = Qt::LeftButton == e->button() ? addColor_ : remColor_;
    update();
}

void DiffractogramPlotOverlay::mouseReleaseEvent(QMouseEvent* e) {
    mouseDown_ = false;
    update();

    Range range(plot_.fromPixels(mouseDownPos_, cursorPos_));
    switch (plot_.getTool()) {
    case DiffractogramPlot::eTool::BACKGROUND:
        if (Qt::LeftButton == e->button())
            plot_.addBg(range);
        else
            plot_.remBg(range);
        break;

    case DiffractogramPlot::eTool::PEAK_REGION: plot_.setNewReflRange(range); break;

    case DiffractogramPlot::eTool::NONE: break;
    }
}

void DiffractogramPlotOverlay::mouseMoveEvent(QMouseEvent* e) {
    updateCursorRegion();
    cursorPos_ = qBound(marginLeft_, e->x(), width() - marginRight_);
    updateCursorRegion();
    if (mouseDown_)
        update();
}

void DiffractogramPlotOverlay::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    QRect g = geometry();

    if (mouseDown_) {
        g.setLeft(qMin(mouseDownPos_, cursorPos_));
        g.setRight(qMax(mouseDownPos_, cursorPos_));

        painter.fillRect(g, color_);
    }

    if (hasCursor_) {
        QLineF cursor(cursorPos_, g.top(), cursorPos_, g.bottom());

        painter.setPen(Qt::red);
        painter.drawLine(cursor);
    }
}

void DiffractogramPlotOverlay::updateCursorRegion() {
    auto g = geometry();
    // updating 2 pixels seems to work both on Linux & Mac
    update(cursorPos_ - 1, g.top(), 2, g.height());
}

// ************************************************************************** //
//  class DiffractogramPlot (file scope)
// ************************************************************************** //

DiffractogramPlot::DiffractogramPlot(Diffractogram& diffractogram)
    : diffractogram_(diffractogram), showBgFit_(false) {
    overlay_ = new DiffractogramPlotOverlay(*this);

    bgRgeColor_ = QColor(0x98, 0xfb, 0x98, 0x50);
    reflRgeColor_ = QColor(0x87, 0xce, 0xfa, 0x50);

    auto* ar = axisRect();

    // fix margins
    QFontMetrics fontMetrics(font());
    int em = fontMetrics.width('M'), ascent = fontMetrics.ascent();

    QMargins margins(6 * em, ascent, em, 2 * ascent);
    ar->setAutoMargins(QCP::msNone);
    ar->setMargins(margins);
    overlay_->setMargins(margins.left(), margins.right());

    // colours
    setBackground(palette().background().color());
    ar->setBackground(Qt::white);

    // graphs in the "main" layer; in the display order
    bgGraph_ = addGraph();
    dgramGraph_ = addGraph();
    dgramBgFittedGraph2_ = addGraph();
    dgramBgFittedGraph2_->setVisible(false);
    dgramBgFittedGraph_ = addGraph();

    bgGraph_->setPen(QPen(QColor(0x21, 0xa1, 0x21, 0xff), 2));

    dgramBgFittedGraph_->setPen(QPen(Qt::black, 2));

    dgramBgFittedGraph2_->setLineStyle(QCPGraph::LineStyle::lsNone);
    dgramBgFittedGraph2_->setScatterStyle(
        QCPScatterStyle(QCPScatterStyle::ScatterShape::ssDisc, QColor(255, 0, 0), 4));

    dgramGraph_->setLineStyle(QCPGraph::LineStyle::lsNone);
    dgramGraph_->setScatterStyle(
        QCPScatterStyle(QCPScatterStyle::ScatterShape::ssDisc, Qt::gray, 2));

    // background regions
    addLayer("bg", layer("background"), QCustomPlot::limAbove);
    // reflections
    addLayer("refl", layer("main"), QCustomPlot::limAbove);
    // reflections
    addLayer("marks", layer("refl"), QCustomPlot::limAbove);

    setCurrentLayer("marks");

    guesses_ = addGraph();
    guesses_->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 8));
    guesses_->setLineStyle(QCPGraph::lsNone);
    guesses_->setPen(QPen(Qt::darkGray));

    fits_ = addGraph();
    fits_->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 8));
    fits_->setLineStyle(QCPGraph::lsNone);
    fits_->setPen(QPen(Qt::red));

    connect(gHub, &TheHubSignallingBase::sigReflectionData,
            [this](shp_Reflection reflection) { onReflectionData(reflection); });

    connect(gHub->toggle_showBackground, &QAction::toggled, [this](bool on) {
        showBgFit_ = on;
        updateBg();
    });

    connect(gHub, &TheHubSignallingBase::sigBgChanged, [this]() { updateBg(); });

    tool_ = eTool::NONE;
}

void DiffractogramPlot::setTool(eTool tool) {
    tool_ = tool;
    updateBg();
}

void DiffractogramPlot::plot(
    Curve const& dgram, Curve const& dgramBgFitted, Curve const& bg, curve_vec const& refls,
    uint currReflIndex) {
    if (dgram.isEmpty()) {
        xAxis->setVisible(false);
        yAxis->setVisible(false);

        bgGraph_->clearData();
        dgramGraph_->clearData();
        dgramBgFittedGraph_->clearData();
        dgramBgFittedGraph2_->clearData();

        clearReflLayer();
    } else {
        auto tthRange = dgram.rgeX();

        Range intenRange;
        if (gHub->isFixedIntenDgramScale()) {
            debug::ensure(!diffractogram_.suite().isNull());
            auto lens = gSession->defaultDatasetLens(*diffractogram_.suite());
            intenRange = lens->rgeInten();
        } else {
            intenRange = dgramBgFitted.rgeY();
            intenRange.extendBy(dgram.rgeY());
        }

        xAxis->setRange(tthRange.min, tthRange.max);
        yAxis->setRange(qMin(0., intenRange.min), intenRange.max);
        yAxis->setNumberFormat("g");
        xAxis->setVisible(true);
        yAxis->setVisible(true);

        if (showBgFit_) {
            bgGraph_->setData(bg.xs().sup(), bg.ys().sup());
        } else {
            bgGraph_->clearData();
        }

        dgramGraph_->setData(dgram.xs().sup(), dgram.ys().sup());
        dgramBgFittedGraph_->setData(dgramBgFitted.xs().sup(), dgramBgFitted.ys().sup());
        dgramBgFittedGraph2_->setData(dgramBgFitted.xs().sup(), dgramBgFitted.ys().sup());

        clearReflLayer();
        setCurrentLayer("refl");

        for_i (refls.count()) {
            auto& r = refls.at(i);
            auto* graph = addGraph();
            reflGraph_.append(graph);
            graph->setPen(QPen(Qt::green, i == currReflIndex ? 2 : 1));
            graph->setData(r.xs().sup(), r.ys().sup());
        }
    }

    replot();
}

Range DiffractogramPlot::fromPixels(int pix1, int pix2) {
    return Range::safeFrom(xAxis->pixelToCoord(pix1), xAxis->pixelToCoord(pix2));
}

void DiffractogramPlot::clearBg() {
    gHub->setBgRanges(Ranges());
}

void DiffractogramPlot::addBg(Range const& range) {
    gHub->addBgRange(range);
}

void DiffractogramPlot::remBg(Range const& range) {
    gHub->remBgRange(range);
}

void DiffractogramPlot::setNewReflRange(Range const& range) {
    diffractogram_.setCurrReflNewRange(range);
    updateBg();
}

void DiffractogramPlot::updateBg() {
    clearItems();

    switch (tool_) {
    case eTool::BACKGROUND: {
        Ranges const& rs = gSession->bgRanges();
        for_i (rs.count())
            addBgItem(rs.at(i));
        break;
    }
    case eTool::PEAK_REGION: addBgItem(diffractogram_.currReflRange()); break;
    case eTool::NONE: break;
    }

    diffractogram_.render();
}

void DiffractogramPlot::clearReflLayer() {
    for (auto g : reflGraph_)
        removeGraph(g);
    reflGraph_.clear();
}

eFittingTab DiffractogramPlot::selectedFittingTab() {
    return gHub->fittingTab();
}

void DiffractogramPlot::enterZoom(bool on) {
    overlay_->setHidden(on);
    dgramBgFittedGraph2_->setVisible(on);
}

void DiffractogramPlot::addBgItem(Range const& range) {
    setCurrentLayer("bg");

    QColor color;
    switch (gHub->fittingTab()) {
    case eFittingTab::BACKGROUND: color = bgRgeColor_; break;
    case eFittingTab::REFLECTIONS: color = reflRgeColor_; break;
    default: break;
    }

    auto ir = new QCPItemRect(this);
    ir->setPen(QPen(color));
    ir->setBrush(QBrush(color));
    auto br = ir->bottomRight;
    br->setTypeY(QCPItemPosition::ptViewportRatio);
    br->setCoords(range.max, 1);
    auto tl = ir->topLeft;
    tl->setTypeY(QCPItemPosition::ptViewportRatio);
    tl->setCoords(range.min, 0);
    addItem(ir);
}

void DiffractogramPlot::resizeEvent(QResizeEvent* e) {
    QCustomPlot::resizeEvent(e);
    auto size = e->size();
    overlay_->setGeometry(0, 0, size.width(), size.height());
}

void DiffractogramPlot::onReflectionData(shp_Reflection reflection) {

    guesses_->clearData();
    fits_->clearData();

    if (reflection && diffractogram_.suite()) {
        auto& fun = reflection->peakFunction();

        auto gp = fun.guessedPeak();
        if (gp.isValid()) {
            guesses_->addData(gp.x, gp.y);
            auto gw2 = fun.guessedFWHM() / 2;
            guesses_->addData(gp.x - gw2, gp.y / 2);
            guesses_->addData(gp.x + gw2, gp.y / 2);
        }

        auto fp = fun.fittedPeak();
        if (fp.isValid()) {
            fits_->addData(fp.x, fp.y);
            auto fw2 = fun.fittedFWHM() / 2;
            fits_->addData(fp.x - fw2, fp.y / 2);
            fits_->addData(fp.x + fw2, fp.y / 2);
        }
    }
}

// ************************************************************************** //
//  class Diffractogram
// ************************************************************************** //

Diffractogram::Diffractogram()
    : suite_(nullptr), currReflIndex_(0) {
    setLayout((box_ = boxLayout(Qt::Vertical)));
    box_->addWidget((plot_ = new DiffractogramPlot(*this)));
    auto hb = hbox();
    box_->addLayout(hb);

    hb->addWidget(label("normalize to:"));
    comboNormType_ = new QComboBox;
    comboNormType_->addItems(normStrLst());
    hb->addWidget(comboNormType_);

    connect(comboNormType_, slot(QComboBox, currentIndexChanged, int),
            [this](int index) { // TODO init value from hub?
                gHub->setNorm(eNorm(index));
            });

    hb->addWidget(label(" intensity from:"));
    hb->addWidget((intenSum_ = radioButton("sum")));
    hb->addWidget((intenAvg_ = radioButton("avg ×")));
    hb->addWidget((intenScale_ = spinDoubleCell(gui_cfg::em4_2, 0.001)));
    intenScale_->setDecimals(3);

    connect(intenAvg_, &QRadioButton::toggled, [this](bool on) {
        intenScale_->setEnabled(on);
        intenScale_->setValue(gSession->intenScale());
        gHub->setIntenScaleAvg(on, preal(intenScale_->value()));
    });

    connect(intenScale_, slot(QDoubleSpinBox, valueChanged, double), [this](double val) {
        if (val > 0)
            gHub->setIntenScaleAvg(gSession->intenScaledAvg(), preal(val));
    });

    hb->addStretch();

    actZoom_ = newToggle("zoom");
    enableZoom_ = textButton(actZoom_);
    hb->addWidget(enableZoom_);

    hb->addStretch();

    hb->addWidget(check(gHub->toggle_combinedDgram));
    hb->addWidget(check(gHub->toggle_fixedIntenDgram));

    connect(actZoom_, &QAction::toggled, this, [this](bool on) {
        plot_->setInteraction(QCP::iRangeDrag, on);
        plot_->setInteraction(QCP::iRangeZoom, on);
        plot_->enterZoom(on);
    });

    connect(gHub, &TheHubSignallingBase::sigSuiteSelected,
            [this](QSharedPointer<Suite> suite){ setSuite(suite); });
    connect(gHub, &TheHubSignallingBase::sigGeometryChanged, [this](){ render(); });
    connect(gHub, &TheHubSignallingBase::sigCorrEnabled, [this](){ render(); });
    connect(gHub, &TheHubSignallingBase::sigDisplayChanged, [this](){ render(); });
    connect(gHub, &TheHubSignallingBase::sigGammaRange, [this](){ render(); });
    connect(gHub, &TheHubSignallingBase::sigBgChanged, [this](){ render(); });
    connect(gHub, &TheHubSignallingBase::sigReflectionsChanged, [this](){ render(); });
    connect(gHub, &TheHubSignallingBase::sigNormChanged, [this](){ onNormChanged(); });

    connect(gHub->trigger_clearBackground, &QAction::triggered, [this]() { plot_->clearBg(); });

    connect(gHub, &TheHubSignallingBase::sigFittingTab,
            [this](eFittingTab tab) { onFittingTab(tab); });

    connect(gHub->toggle_selRegions, &QAction::toggled, [this](bool on) {
        using eTool = DiffractogramPlot::eTool;
        auto tool = eTool::NONE;

        if (on)
            switch (gHub->fittingTab()) {
            case eFittingTab::BACKGROUND: tool = eTool::BACKGROUND; break;
            case eFittingTab::REFLECTIONS: tool = eTool::PEAK_REGION; break;
            default: break;
            }

        plot_->setTool(tool);
        });

    connect(gHub, &TheHubSignallingBase::sigReflectionSelected,
            [this](shp_Reflection reflection) {
                currentReflection_ = reflection;
                plot_->updateBg();
            });

    connect(gHub, &TheHubSignallingBase::sigReflectionValues,
            [this](Range const& range, qpair const& peak, fwhm_t fwhm, bool withGuesses) {
                if (currentReflection_) {
                    currentReflection_->setRange(range);
                    if (withGuesses)
                        currentReflection_->invalidateGuesses();
                    else {
                        currentReflection_->setGuessPeak(peak);
                        currentReflection_->setGuessFWHM(fwhm);
                    }
                    plot_->updateBg();
                }
            });

    gHub->toggle_selRegions->setChecked(true);
    gHub->toggle_showBackground->setChecked(true);
    intenAvg_->setChecked(true);
}

void Diffractogram::onNormChanged() {
    intenScale_->setValue(gSession->intenScale()); // TODO own signal
    if (gSession->intenScaledAvg())
        intenAvg_->setChecked(true);
    else
        intenSum_->setChecked(true);
    render();
}

void Diffractogram::onFittingTab(eFittingTab tab) {
    bool on = gHub->toggle_selRegions->isChecked();
    switch (tab) {
    case eFittingTab::BACKGROUND:
        gHub->toggle_selRegions->setIcon(QIcon(":/icon/selRegion"));
        plot_->setTool(
            on ? DiffractogramPlot::eTool::BACKGROUND : DiffractogramPlot::eTool::NONE);
        break;
    case eFittingTab::REFLECTIONS:
        gHub->toggle_selRegions->setIcon(QIcon(":/icon/reflRegion"));
        plot_->setTool(
            on ? DiffractogramPlot::eTool::PEAK_REGION : DiffractogramPlot::eTool::NONE);
        break;
    default: plot_->setTool(DiffractogramPlot::eTool::NONE);
    }
}

void Diffractogram::render() {
    calcDgram();
    calcBackground();
    calcReflections();

    plot_->plot(dgram_, dgramBgFitted_, bg_, refls_, currReflIndex_);
}

void Diffractogram::setSuite(QSharedPointer<Suite> suite) {
    suite_ = suite;
    actZoom_->setChecked(false);
    render();
}

void Diffractogram::calcDgram() {
    dgram_.clear();

    if (!suite_)
        return;

    if (gHub->isCombinedDgram())
        dgram_ = suite_->experiment().avgCurve();
    else {
        auto lens = gSession->defaultDatasetLens(*suite_);
        dgram_ = lens->makeCurve(gSession->gammaRange());
    }
}

void Diffractogram::calcBackground() {
    bg_.clear();
    dgramBgFitted_.clear();

    auto bgPolynom = Polynom::fromFit(gSession->bgPolyDegree(), dgram_, gSession->bgRanges());

    for_i (dgram_.count()) {
        qreal x = dgram_.x(i), y = bgPolynom.y(x);
        bg_.append(x, y);
        dgramBgFitted_.append(x, dgram_.y(i) - y);
    }
}

void Diffractogram::setCurrReflNewRange(Range const& range) {
    if (currentReflection_) {
        currentReflection_->setRange(range);
        currentReflection_->invalidateGuesses();
    }
}

Range Diffractogram::currReflRange() const {
    return currentReflection_ ? currentReflection_->range() : Range();
}

void Diffractogram::calcReflections() {
    refls_.clear();
    currReflIndex_ = 0;

    auto rs = gSession->reflections();
    for_i (rs.count()) {
        auto& r = rs[i];
        if (r == currentReflection_)
            currReflIndex_ = i;

        r->fit(dgramBgFitted_);

        auto& rge = r->range();
        auto& fun = r->peakFunction();

        Curve c;

        for_i (dgramBgFitted_.count()) {
            qreal x = dgramBgFitted_.x(i);
            if (rge.contains(x))
                c.append(x, fun.y(x));
        }

        refls_.append(c);
    }

    gHub->tellReflectionData(currentReflection_);
}

} // namespace panel
} // namespace gui
