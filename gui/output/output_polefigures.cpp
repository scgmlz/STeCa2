// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      gui/output/output_polefigures.cpp
//! @brief     Implements ...
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "output_polefigures.h"
#include "calc/calc_polefigure.h"
#include "colors.h"
#include "thehub.h"

#include <QPainter>
#include <QTextStream>
#include <qmath.h>

namespace gui {
namespace output {

TabGraph::TabGraph(TheHub& hub, Params& params)
    : super(hub, params), flat_(false), alphaMax_(90), avgAlphaMax_(0) {
    ENSURE(params_.panelInterpolation)

    grid_->addWidget((cbFlat_ = check("no intensity")), 0, 0);

    grid_->addRowStretch();
    grid_->addColumnStretch();

    connect(
        params_.panelInterpolation->avgAlphaMax, slot(QDoubleSpinBox, valueChanged, double),
        [this]() { update(); });

    connect(cbFlat_, &QCheckBox::toggled, [this]() { update(); });

    update();
}

void TabGraph::set(calc::ReflectionInfos rs) {
    rs_ = rs;
    update();
}

void TabGraph::update() {
    avgAlphaMax_ = params_.panelInterpolation->avgAlphaMax->value();
    flat_ = cbFlat_->isChecked();
    super::update();
}

void TabGraph::paintEvent(QPaintEvent*) {
    int w = size().width(), h = size().height();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(w / 2, h / 2);

    p_ = &painter;
    c_ = QPointF(0, 0);
    r_ = qMin(w, h) / 2;

    paintGrid();
    paintPoints();
}

QPointF TabGraph::p(deg alpha, deg beta) const {
    qreal r = r_ * alpha / alphaMax_;

    rad betaRad = beta.toRad();
    return QPointF(r * cos(betaRad), -r * sin(betaRad));
}

TabGraph::deg TabGraph::alpha(QPointF const& p) const {
    return sqrt(p.x() * p.x() + p.y() * p.y()) / r_ * alphaMax_;
}

TabGraph::deg TabGraph::beta(QPointF const& p) const {
    deg b = rad(atan2(p.y(), p.x())).toDeg();
    return b <= 0 ? -b : 360 - b;
}

void TabGraph::circle(QPointF c, qreal r) {
    p_->drawEllipse(c, r, r);
}

void TabGraph::paintGrid() {
    QPen penMajor(Qt::gray), penMinor(Qt::lightGray);

    for (int alpha = 10; alpha <= 90; alpha += 10) {
        qreal r = r_ / alphaMax_ * alpha;
        p_->setPen(!(alpha % 30) ? penMajor : penMinor);
        circle(c_, r);
    }

    for (int beta = 0; beta < 360; beta += 10) {
        p_->setPen(!(beta % 30) ? penMajor : penMinor);
        p_->drawLine(p(10, beta), p(90, beta));
    }

    QPen penMark(Qt::darkGreen);
    p_->setPen(penMark);
    circle(c_, r_ * avgAlphaMax_ / alphaMax_);
}

void TabGraph::paintPoints() {
    qreal rgeMax = rs_.rgeInten().max;

    for (auto& r : rs_) {
        qreal inten = r.inten();

        if (qIsFinite(inten)) { // nan comes from interpolartion
            auto pp = p(r.alpha(), r.beta());
            if (flat_) {
                auto color = QColor(Qt::blue);
                p_->setPen(color);
                p_->setBrush(color);
                circle(pp, .5);
            } else {
                inten /= rgeMax;
                auto color = QColor(intenGraph(inten, 1));
                p_->setPen(color);
                p_->setBrush(color);
                circle(pp, inten * r_ / 60); // TODO scale to max inten
            }
        }
    }
}

TabPoleFiguresSave::TabPoleFiguresSave(TheHub& hub, Params& params) : super(hub, params, false) {
    auto hb = hbox();
    grid_->addLayout(hb, grid_->rowCount(), 0);
    grid_->addRowStretch();

    auto p1 = new panel::GridPanel(hub, "Output data");
    auto p2 = new panel::GridPanel(hub, "To save");

    hb->addWidget(p1);
    hb->addWidget(p2);
    hb->addStretch();

    {
        auto g = p1->grid();
        g->addWidget((outputInten_ = check("Intensity pole figure")));
        g->addWidget((outputTth_ = check("Peak position pole figure")));
        g->addWidget((outputFWHM_ = check("TWHM pole figure")));
        g->addRowStretch();
    }

    {
        auto g = p2->grid();
        g->addWidget((rbSelectedRefl_ = radioButton("Selected reflection")));
        g->addWidget((rbAllRefls_ = radioButton("All reflections")));
        g->addWidget(textButton(actSave), 2, 1);
        g->addRowStretch();
    }

    rbSelectedRefl_->setChecked(true);
    outputInten_->setChecked(true);
}

bool TabPoleFiguresSave::onlySelectedRefl() const {
    return rbSelectedRefl_->isChecked();
}

bool TabPoleFiguresSave::outputInten() const {
    return outputInten_->isChecked();
}

bool TabPoleFiguresSave::outputTth() const {
    return outputTth_->isChecked();
}

bool TabPoleFiguresSave::outputFWHM() const {
    return outputFWHM_->isChecked();
}

void TabPoleFiguresSave::rawReflSettings(bool on) {
    outputTth_->setEnabled(on);
    outputFWHM_->setEnabled(on);
}

static const Params::ePanels PANELS =
    Params::ePanels(Params::REFLECTION | Params::GAMMA | Params::POINTS | Params::INTERPOLATION);

PoleFiguresFrame::PoleFiguresFrame(TheHub& hub, rcstr title, QWidget* parent)
    : super(hub, title, new Params(hub, PANELS), parent) {
    tabGraph_ = new TabGraph(hub, *params_);
    tabs_->addTab("Graph", Qt::Vertical).box().addWidget(tabGraph_);

    tabSave_ = new TabPoleFiguresSave(hub, *params_);
    tabs_->addTab("Save", Qt::Vertical).box().addWidget(tabSave_);

    //  connect(params()->cbRefl, slot(QComboBox,currentIndexChanged,int),
    //  [this]() {
    //    int index = params()->currReflIndex();
    //    if (index>=0) {
    //      bool on = fit::ePeakType::RAW !=
    //      hub_.reflections().at(to_u(index))->type();
    //      tabSave_->rawReflSettings(on);
    //    }
    //  });

    connect(
        tabSave_->actSave, &QAction::triggered, [this]() { logSuccess(savePoleFigureOutput()); });

    //  params()->cbRefl->currentIndexChanged(0);
}

void PoleFiguresFrame::displayReflection(uint reflIndex, bool interpolated) {
    super::displayReflection(reflIndex, interpolated);
    if (!interpPoints_.isEmpty() && !calcPoints_.isEmpty())
        tabGraph_->set((interpolated ? interpPoints_ : calcPoints_).at(reflIndex));

    bool on = fit::ePeakType::RAW != hub_.reflections().at(reflIndex)->type();
    tabSave_->rawReflSettings(on);
}

bool PoleFiguresFrame::savePoleFigureOutput() {
    auto& reflections = hub_.reflections();
    if (reflections.isEmpty())
        return false;

    str path = tabSave_->filePath(false);
    if (path.isEmpty())
        return false;

    if (tabSave_->onlySelectedRefl())
        return writePoleFigureOutputFiles(path, getReflIndex());

    // all reflections
    bool res = true;
    for_i (reflections.count())
        res = logCheckSuccess(path, writePoleFigureOutputFiles(path, i)) && res;

    return res;
}

static str const OUT_FILE_TAG(".refl%1");
static int const MAX_LINE_LENGTH_POL(9);

bool PoleFiguresFrame::writePoleFigureOutputFiles(rcstr filePath, uint index) {
    auto refl = hub_.reflections().at(index);
    calc::ReflectionInfos reflInfo;

    if (getInterpolated())
        reflInfo = interpPoints_.at(index);
    else
        reflInfo = calcPoints_.at(index);

    auto type = refl->type();

    str path = str(filePath + OUT_FILE_TAG).arg(index + 1);

    bool check = false;
    uint numSavedFiles = 0;

    if (tabSave_->outputInten()) {
        qreal_vec output;
        for_i (reflInfo.count())
            output.append(reflInfo.at(i).inten());

        auto intenFilePath = path + ".inten";
        writeListFile(intenFilePath, reflInfo, output);
        writePoleFile(intenFilePath, reflInfo, output);
        writeErrorMask(intenFilePath, reflInfo, output);

        check = true;
        numSavedFiles += 3;
    }

    if (tabSave_->outputTth() && fit::ePeakType::RAW != type) {
        qreal_vec output;
        for_i (reflInfo.count())
            output.append(reflInfo.at(i).tth());

        auto tthFilePath = filePath + ".tth";
        writeListFile(tthFilePath, reflInfo, output);
        writePoleFile(tthFilePath, reflInfo, output);

        check = true;
        numSavedFiles += 2;
    }

    if (tabSave_->outputFWHM() && fit::ePeakType::RAW != type) {
        qreal_vec output;
        for_i (reflInfo.count())
            output.append(reflInfo.at(i).fwhm());

        auto fwhmFilePath = filePath + ".fwhm";
        writeListFile(fwhmFilePath, reflInfo, output);
        writePoleFile(fwhmFilePath, reflInfo, output);

        check = true;
        numSavedFiles += 2;
    }

    if (numSavedFiles > 0)
        logMessage(str("%1 files have been saved").arg(numSavedFiles));

    return check;
}

void PoleFiguresFrame::writeErrorMask(
    rcstr filePath, calc::ReflectionInfos reflInfo, qreal_vec::rc output) {
    WriteFile file(filePath + ".errorMask");
    QTextStream stream(&file);

    for (uint j = 0, jEnd = reflInfo.count(); j < jEnd; j += 9) {
        uint max = j + MAX_LINE_LENGTH_POL;
        for (uint i = j; i < max; i++) {
            if (qIsNaN(output.at(i)))
                stream << "0"
                       << " ";
            else
                stream << "1"
                       << " ";
        }
        stream << '\n';
    }
}

void PoleFiguresFrame::writePoleFile(
    rcstr filePath, calc::ReflectionInfos reflInfo, qreal_vec::rc output) {
    WriteFile file(filePath + ".pol");
    QTextStream stream(&file);

    for (uint j = 0, jEnd = reflInfo.count(); j < jEnd; j += 9) {
        uint max = j + MAX_LINE_LENGTH_POL;
        for (uint i = j; i < max; i++) {
            if (qIsNaN(output.at(i)))
                stream << " -1 "
                       << " ";
            else
                stream << output.at(i) << " ";
        }
        stream << '\n';
    }
}

void PoleFiguresFrame::writeListFile(
    rcstr filePath, calc::ReflectionInfos reflInfo, qreal_vec::rc output) {
    WriteFile file(filePath + ".lst");
    QTextStream stream(&file);

    for_i (reflInfo.count()) {
        stream << qreal(reflInfo.at(i).alpha()) << " " << qreal(reflInfo.at(i).beta()) << " "
               << output.at(i) << '\n';
    }
}
}
}
