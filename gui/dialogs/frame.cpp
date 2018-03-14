// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/dialogs/frame.cpp
//! @brief     Implements classes Params, Frame, and local classes ShowColsWidget, TabTable
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "gui/dialogs/frame.h"
#include "core/calc/calc_polefigure.h"
#include "core/session.h"
#include "gui/dialogs/data_table.h"
#include "gui/dialogs/dialog_panels.h"
#include "gui/mainwin.h" // defines _SLOT_
#include <QProgressBar>
#include <QScrollArea>

struct showcol_t {
    QString name;
    CCheckBox* cb;
};

typedef vec<showcol_t> showcol_vec;

// ************************************************************************** //
//  local class ShowColsWidget (only used by TabTable)
// ************************************************************************** //

//! A row of controls for choosing which data columns are to be displayed in a TabTable.

class ShowColsWidget : public QWidget {
public:
    ShowColsWidget(const QString& name, DataView&, showcol_vec&);
private:
    DataView& table_;
    showcol_vec& showCols_;
    QBoxLayout* box_;
    CRadioButton rbHidden_, rbAll_, rbNone_, rbInten_, rbTth_, rbFWHM_;
};

ShowColsWidget::ShowColsWidget(const QString& name, DataView& table, showcol_vec& showCols)
    : table_(table)
    , showCols_(showCols)
    , rbHidden_(name+".rbHidden", "")
    , rbAll_(name+".rbAll", "all")
    , rbNone_(name+".rbNone", "none")
    , rbInten_(name+".rbInten", "Intensity")
    , rbTth_(name+".rbTth", "2θ")
    , rbFWHM_(name+".rbFWHM", "fwhm")
{
    using eReflAttr = PeakInfo::eReflAttr;

    setLayout((box_ = new QVBoxLayout()));

    box_->addWidget(&rbHidden_);
    rbHidden_.hide();

    box_->addWidget(&rbAll_);
    box_->addWidget(&rbNone_);
    box_->addWidget(&rbInten_);
    box_->addWidget(&rbTth_);
    box_->addWidget(&rbFWHM_);
    box_->addSpacing(8);

    for_i (showCols.count()) {
        showcol_t& item = showCols[i];
        box_->addWidget((item.cb = new CCheckBox(name+".cb"+QString::number(i), item.name)));
    }

    auto _all = [this]() {
        for (showcol_t& col : showCols_)
            col.cb->setChecked(true);
    };

    auto _none = [this]() {
        for (showcol_t& col : showCols_)
            col.cb->setChecked(false);
    };

    auto _showInten = [this, _none]() {
        _none();
        showCols_.at(int(eReflAttr::INTEN)).cb->setChecked(true);
    };

    auto _showTth = [this, _none]() {
        _none();
        showCols_.at(int(eReflAttr::TTH)).cb->setChecked(true);
    };

    auto _showFWHM = [this, _none]() {
        _none();
        showCols_.at(int(eReflAttr::FWHM)).cb->setChecked(true);
    };

    auto _updateRbs = [this]() {
        bool isAll = true, isNone = true, isOther = false;
        int nInten = 0, nTth = 0, nFwhm = 0;

        for_i (showCols_.count()) {
            if (!showCols_.at(i).cb->isChecked()) {
                isAll = false;
                continue;
            }

            isNone = false;

            switch (eReflAttr(i)) {
            case eReflAttr::ALPHA:
            case eReflAttr::BETA:
                ++nInten;
                ++nTth;
                ++nFwhm;
                break;
            case eReflAttr::INTEN: ++nInten; break;
            case eReflAttr::TTH: ++nTth; break;
            case eReflAttr::FWHM: ++nFwhm; break;
            default: isOther = true; break;
            }
        }

        rbHidden_.setChecked(true);
        rbNone_.setChecked(isNone);
        rbAll_.setChecked(isAll);

        int const PRESET_SELECTION = 1;

        rbInten_.setChecked(!isOther && PRESET_SELECTION == nInten);
        rbTth_.setChecked(!isOther && PRESET_SELECTION == nTth);
        rbFWHM_.setChecked(!isOther && PRESET_SELECTION == nFwhm);
    };

    for_i (showCols_.count()) {
        QCheckBox* cb = showCols_.at(i).cb;
        connect(cb, &QCheckBox::toggled, [this, _updateRbs, i](bool on) {
            if (on)
                table_.showColumn(i + 1);
            else
                table_.hideColumn(i + 1);

            _updateRbs();
        });
    }

    connect(&rbAll_, &QRadioButton::clicked, _all);
    connect(&rbNone_, &QRadioButton::clicked, _none);
    connect(&rbInten_, &QRadioButton::clicked, _showInten);
    connect(&rbTth_, &QRadioButton::clicked, _showTth);
    connect(&rbFWHM_, &QRadioButton::clicked, _showFWHM);

    rbAll_.click();
}

// ************************************************************************** //
//  local class TabTable (only used by Frame implementation)
// ************************************************************************** //

//! A DataView that can be scrolled and that has a ShowColsWidget for selecting data columns.

//! Used as a tab in several output dialogs.

class TabTable : public QWidget {
public:
    TabTable(const QString& name, const QStringList& headers,
             const QStringList& outHeaders, const cmp_vec&);
    ~TabTable();
    DataView* table;
private:
    ShowColsWidget* showColumnsWidget_;
    showcol_vec showCols_;
};

TabTable::TabTable(const QString& name, const QStringList& headers,
                   const QStringList& outHeaders, const cmp_vec& cmps)
{
    QGridLayout* grid_ = new QGridLayout();
    setLayout(grid_);
    ASSERT(headers.count() == cmps.count());
    int numCols = headers.count();

    grid_->addWidget((table = new DataView(numCols)), 0, 0);
    grid_->setColumnStretch(0, 1);

    table->setColumns(headers, outHeaders, cmps);

    for_i (numCols) {
        showcol_t item;
        item.name = headers.at(i);
        showCols_.append(item);
    }

    auto scrollArea = new QScrollArea;
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidget((showColumnsWidget_ = new ShowColsWidget(name, *table, showCols_)));

    grid_->addWidget(scrollArea, 0, 1);
}

TabTable::~TabTable() {
    delete showColumnsWidget_;
    delete table;
}


// ************************************************************************** //
//  class Params
// ************************************************************************** //

Params::Params(ePanels panels)
    : panelPeak(nullptr)
    , panelGammaSlices(nullptr)
    , panelGammaRange(nullptr)
    , panelPoints(nullptr)
    , panelInterpolation(nullptr)
    , panelDiagram(nullptr) {

    setLayout((box_ = new QHBoxLayout()));

    if (REFLECTION & panels)
        box_->addWidget((panelPeak = new PanelPeak()));

    ASSERT(panels & GAMMA);
    if (GAMMA & panels) {
        box_->addWidget((panelGammaSlices = new PanelGammaSlices()));
        box_->addWidget((panelGammaRange = new PanelGammaRange()));
    }

    if (POINTS & panels)
        box_->addWidget((panelPoints = new PanelPoints()));

    if (INTERPOLATION & panels)
        box_->addWidget((panelInterpolation = new PanelInterpolation()));

    if (DIAGRAM & panels)
        box_->addWidget((panelDiagram = new PanelDiagram()));

    box_->addStretch();

    if (panelGammaSlices)
        panelGammaSlices->updateValues();

    if (panelGammaRange)
        panelGammaRange->updateValues();
}


// ************************************************************************** //
//  class Frame
// ************************************************************************** //

Frame::Frame(const QString& name, const QString& title, Params* params)
    : CModelessDialog(gGui, name)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(title);

    setLayout((box_ = new QVBoxLayout()));

    ASSERT(params);
    box_->addWidget((params_ = params));

    tabs_ = new QTabWidget();
    tabs_->setTabPosition(QTabWidget::North);
    box_->addWidget(tabs_);
    box_->setStretch(box_->count() - 1, 1);

    auto hb = new QHBoxLayout();
    box_->addLayout(hb);

    actClose_ = new CTrigger("actClose#", "Close");
    actCalculate_ = new CTrigger("actCalculate#", "Calculate");
    actInterpolate_ = new CTrigger("actInterpolate#", "Interpolate");

    hb->addWidget((btnClose_ = new XTextButton(actClose_)));
    hb->addStretch(1);
    hb->addWidget((progressBar_ = new QProgressBar));
    hb->setStretchFactor(progressBar_, 333);
    hb->addStretch(1);
    hb->addWidget((btnCalculate_ = new XTextButton(actCalculate_)));
    hb->addWidget((btnInterpolate_ = new XTextButton(actInterpolate_)));

    progressBar_->hide();

    connect(actClose_, &QAction::triggered, [this]() { close(); });
    connect(actCalculate_, &QAction::triggered, [this]() { calculate(); });
    connect(actInterpolate_, &QAction::triggered, [this]() { interpolate(); });

    if (params_->panelPeak) {
        connect(&params_->panelPeak->cbRefl, _SLOT_(QComboBox, currentIndexChanged, int),
                [this](){ updatePeak(); });
    }

    if (params_->panelPoints) {
        ASSERT(params_->panelPeak);
        connect(&params_->panelPoints->rbInterp, &QRadioButton::toggled, [this](){ updatePeak(); });
    }

    // tabs

    auto* tabPoints = new QWidget();
    tabs_->addTab(tabPoints, "Points");
    tabPoints->setLayout(new QVBoxLayout());

    tabTable_ = new TabTable(
        name, PeakInfo::dataTags(false), PeakInfo::dataTags(true), PeakInfo::dataCmps());
    tabPoints->layout()->addWidget(tabTable_);
    table_ = tabTable_->table;

    int reflCount = gSession->peaks().count();
    calcPoints_.resize(reflCount);
    interpPoints_.resize(reflCount);
}

Frame::~Frame() {
    delete tabTable_;
}

void Frame::calculate() {
    TakesLongTime __;

    calcPoints_.clear();
    interpPoints_.clear();

    int reflCount = gSession->peaks().count();
    if (!reflCount)
        return;

    const PanelGammaSlices* ps = params_->panelGammaSlices;
    int gammaSlices = ps->numSlices.value();

    const PanelGammaRange* pr = params_->panelGammaRange;
    Range rgeGamma;
    if (pr->cbLimitGamma.isChecked())
        rgeGamma.safeSet(pr->minGamma.value(), pr->maxGamma.value());

    Progress progress(reflCount, progressBar_);

    for_i (reflCount)
        calcPoints_.append(
            gSession->makePeakInfos(
                gSession->peaks().at(i), gammaSlices, rgeGamma, &progress));

    interpolate();
}

void Frame::interpolate() {
    TakesLongTime __;

    interpPoints_.clear();

    const PanelInterpolation* pi = params_->panelInterpolation;
    if (pi) {
        deg alphaStep = pi->stepAlpha.value();
        deg betaStep = pi->stepBeta.value();
        qreal idwRadius = pi->idwRadius.value();

        qreal avgRadius = pi->avgRadius.value();
        qreal avgAlphaMax = pi->avgAlphaMax.value();
        qreal avgTreshold = pi->avgThreshold.value() / 100.0;

        Progress progress(calcPoints_.count(), progressBar_);

        for_i (calcPoints_.count())
            interpPoints_.append(interpolateInfos(
                calcPoints_.at(i), alphaStep, betaStep, idwRadius, avgAlphaMax, avgRadius,
                avgTreshold, &progress));
    } else {
        for_i (calcPoints_.count())
            interpPoints_.append(PeakInfos());
    }

    updatePeak();
}

void Frame::updatePeak() {
    displayPeak(getReflIndex(), getInterpolated());
}

// virtual, overwritten by some output frames, and called back by the overwriting function
void Frame::displayPeak(int reflIndex, bool interpolated) {
    table_->clear();

    ASSERT(calcPoints_.count() == interpPoints_.count());
    if (calcPoints_.count() <= reflIndex)
        return;

    for (const PeakInfo& r : (interpolated ? interpPoints_ : calcPoints_).at(reflIndex))
        table_->addRow(r.data(), false);

    table_->sortData();
}

int Frame::getReflIndex() const {
    ASSERT(params_->panelPeak);
    int reflIndex = params_->panelPeak->cbRefl.currentIndex();
    if (!(reflIndex >= 0)) qFatal("invalid peak index");
    return reflIndex;
}

bool Frame::getInterpolated() const {
    const PanelPoints* pi = params_->panelPoints;
    return pi ? pi->rbInterp.isChecked() : false;
}
