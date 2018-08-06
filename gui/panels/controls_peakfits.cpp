//  ***********************************************************************************************
//
//  Steca: stress and texture calculator
//
//! @file      gui/panels/controls_peakfits.cpp
//! @brief     Implements class ControlsPeakfits, and local classes
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
//  ***********************************************************************************************

#include "controls_peakfits.h"
#include "core/session.h"
#include "qcr/widgets/views.h"
#include "qcr/widgets/tables.h"
#include "gui/mainwin.h"
#include "gui/state.h"
#include "gui/actions/triggers.h"
#include "gui/view/range_control.h"
#include "qcr/base/debug.h"
#define _SLOT_(Class, method, argType) static_cast<void (Class::*)(argType)>(&Class::method)

namespace {
QString safeRealText(double val, int prec=4) {
    return qIsFinite(val) ? QString::number(val, 'g', prec) : "NaN"; }
QString par2text(const FitParameter& par) {
    return safeRealText(par.value()) + "+-" + safeRealText(par.error()); }
} // local methods

//  ***********************************************************************************************
//! @class PeaksModel (local scope)

//! Model for table of peaks.

class PeaksModel : public TableModel {
public:
    PeaksModel() : TableModel("peaks") {}

    int columnCount() const final { return NUM_COLUMNS; }
    int rowCount() const final { return gSession->peaks.count(); }
    int highlighted() const final { return gSession->peaks.selectedIndex(); }
    void onHighlight(int row) final { gSession->peaks.select(row); }

    QVariant data(const QModelIndex&, int) const;

    enum { COL_ID = 1, COL_TYPE, COL_RANGE, NUM_COLUMNS };
};

QVariant PeaksModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    if (row < 0 || rowCount() <= row)
        return {};
    const Peak& peak = gSession->peaks.at(row);
    switch (role) {
    case Qt::DisplayRole: {
        int col = index.column();
        if (col < 1)
            return {};
        switch (col) {
        case COL_ID:
            return QString::number(row + 1);
        case COL_TYPE:
            return peak.functionName();
        case COL_RANGE:
            if (!peak.range().isValid())
                return "undefined";
            else if (peak.range().isEmpty())
                return "empty";
            return peak.range().to_s();
        default:
            return {};
        }
    }
    case Qt::ForegroundRole: {
        if (peak.range().isEmpty())
            return QColor(Qt::red);
        return QColor(Qt::black);
    }
    case Qt::BackgroundRole: {
        if (row==highlighted())
            return QColor(Qt::cyan);
        return QColor(Qt::white);
    }
    default:
        return {};
    }
}


//  ***********************************************************************************************
//! @class PeakfitOutcomeView (local scope)

//! Displays outcome of raw analysis and fit of current peak for current dfgram.

class PeakfitOutcomeView : public QcrWidget {
public:
    PeakfitOutcomeView();
private:
    void remake();
    void enableRaw(bool);
    void enableFit(bool);
    QcrLineDisplay showFitOutcomeX_ {"fittedX", 12, true};
    QcrLineDisplay showFitOutcomeD_ {"fittedD", 12, true};
    QcrLineDisplay showFitOutcomeY_ {"fittedY", 12, true};
    QcrLineDisplay showRawOutcomeX_ {"rawX", 5, true};
    QcrLineDisplay showRawOutcomeY_ {"rawY", 5, true};
    QcrLineDisplay showRawOutcomeD_  {"rawD", 5, true};
};

PeakfitOutcomeView::PeakfitOutcomeView()
    : QcrWidget("peakfitOutcomeView")
{
    auto* grid = new QGridLayout;
    grid->addWidget(new QLabel("direct"), 0, 1);
    grid->addWidget(new QLabel("fitted"), 0, 2);

    grid->addWidget(new QLabel("centre"), 1, 0);
    grid->addWidget(&showRawOutcomeX_, 1, 1);
    grid->addWidget(&showFitOutcomeX_, 1, 2);
    grid->addWidget(new QLabel("deg"), 1, 3);

    grid->addWidget(new QLabel("fwhm"), 2, 0);
    grid->addWidget(&showRawOutcomeD_, 2, 1);
    grid->addWidget(&showFitOutcomeD_, 2, 2);
    grid->addWidget(new QLabel("deg"), 2, 3);

    grid->addWidget(new QLabel("intens"), 3, 0);
    grid->addWidget(&showRawOutcomeY_, 3, 1);
    grid->addWidget(&showFitOutcomeY_, 3, 2);

    grid->setColumnStretch(4, 1);
    setLayout(grid);

    setRemake( [this]() { remake(); } );
}

void PeakfitOutcomeView::remake()
{
    const Peak* peak = gSession->peaks.selectedPeak();
    const Cluster* cluster = gSession->highlightedCluster().cluster();
    bool proceed = peak && cluster;
    enableRaw(proceed);
    enableFit(proceed && !peak->isRaw());
    if (!proceed)
        return;

    int jP = gSession->peaks.selectedIndex();
    const RawOutcome& outcome = cluster->currentDfgram().getRawOutcome(jP);
    showRawOutcomeX_.setText(safeRealText(outcome.getCenter()));
    showRawOutcomeD_.setText(safeRealText(outcome.getFwhm()));
    showRawOutcomeY_.setText(safeRealText(outcome.getIntensity()));

    if (peak->isRaw())
        return;
    const PeakFunction& peakFit = cluster->currentDfgram().getPeakFit(jP);
    showFitOutcomeX_.setText(par2text(peakFit.getCenter()));
    showFitOutcomeD_.setText(par2text(peakFit.getFwhm()));
    showFitOutcomeY_.setText(par2text(peakFit.getIntensity()));
}

void PeakfitOutcomeView::enableRaw(bool on)
{
    showRawOutcomeX_.setEnabled(on);
    showRawOutcomeD_.setEnabled(on);
    showRawOutcomeY_.setEnabled(on);
    if (!on) {
        showRawOutcomeX_.setText("");
        showRawOutcomeD_.setText("");
        showRawOutcomeY_.setText("");
    }
}

void PeakfitOutcomeView::enableFit(bool on)
{
    showFitOutcomeX_.setEnabled(on);
    showFitOutcomeD_.setEnabled(on);
    showFitOutcomeY_.setEnabled(on);
    if (!on) {
        showFitOutcomeX_.setText("");
        showFitOutcomeD_.setText("");
        showFitOutcomeY_.setText("");
    }
}

//  ***********************************************************************************************
//! @class ControlsPeakfits

ControlsPeakfits::ControlsPeakfits()
    : QcrWidget{"peaks"}
    , comboReflType_ {"reflTyp", Peak::keys}
{
    // outbound connections
    connect(&gGui->triggers->peakRemove, &QAction::triggered, []() {
            gSession->peaks.removeSelected();
            emit gSession->sigPeaks();
            gRoot->remakeAll("removePeak"); });
    connect(&gGui->triggers->peaksClear, &QAction::triggered, []() {
            gSession->peaks.clear();
            emit gSession->sigPeaks();
            gRoot->remakeAll("clearPeaks"); });

    // TODO move this to core
    comboReflType_.cell()->setHook( [](int i) {
            const QString& peakFunctionName = Peak::keys[i];
            Peaks::defaultFunctionName = peakFunctionName;
            if (gSession->peaks.selectedPeak())
                gSession->peaks.selectedPeak()->setPeakFunction(peakFunctionName);
            emit gSession->sigPeaks();
            gRoot->remakeAll("reflType");
        } );

    // layout
    topControls_.addStretch();
    topControls_.addWidget(new QcrIconTriggerButton(&gGui->triggers->peakAdd));
    topControls_.addWidget(new QcrIconTriggerButton(&gGui->triggers->peakRemove));
    topControls_.addWidget(new QcrIconTriggerButton(&gGui->triggers->peaksClear));

    auto* box = new QVBoxLayout;
    box->addLayout(&topControls_);
    box->addWidget(new TableView(new PeaksModel()));
    box->addWidget(&comboReflType_);
    box->addWidget(new RangeControl("peak", []()->Range*{
                return gSession->peaks.selectedRange(); }));
    box->addWidget(new PeakfitOutcomeView);
    box->addStretch(1000);
    setLayout(box);
}

void ControlsPeakfits::hideEvent(QHideEvent*)
{
    gGui->state->editingPeakfits = false;
}

void ControlsPeakfits::showEvent(QShowEvent*)
{
    gGui->state->editingPeakfits = true;
}
