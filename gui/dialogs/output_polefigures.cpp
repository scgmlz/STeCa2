// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/dialogs/output_polefigures.cpp
//! @brief     Implements class PoleFiguresFrame
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "gui/dialogs/output_polefigures.h"
#include "core/session.h"
#include "gui/base/filedialog.h"
#include "gui/cfg/colors.h"
#include "gui/mainwin.h"
#include "gui/dialogs/dialog_panels.h"
#include "gui/dialogs/tab_save.h"
#include <qmath.h>
#include <QPainter>

// ************************************************************************** //
//  local class TabGraph
// ************************************************************************** //

//! Tab in PoleFiguresFrame, to display the pole figure.

class TabGraph : public QWidget {
public:
    TabGraph(Params&);
    void set(PeakInfos);

private:
    Params& params_;
    QGridLayout* grid_;
    void update();

    PeakInfos rs_;
    void paintEvent(QPaintEvent*);

    QPointF p(deg alpha, deg beta) const;
    deg alpha(const QPointF&) const;
    deg beta(const QPointF&) const;

    void circle(QPointF c, qreal r);

    void paintGrid();
    void paintPoints();

    // valid during paintEvent
    QPainter* p_;
    QPointF c_;
    qreal r_;

    bool flat_;
    qreal alphaMax_, avgAlphaMax_;

    CCheckBox cbFlat_;
};

TabGraph::TabGraph(Params& params)
    : params_(params)
    , flat_(false)
    , alphaMax_(90)
    , avgAlphaMax_(0)
    , cbFlat_("cbFlat#", "no intensity")
{
    setLayout((grid_ = new QGridLayout()));
    ASSERT(params_.panelInterpolation);

    grid_->addWidget(&cbFlat_, 0, 0);

    grid_->setRowStretch(grid_->rowCount(), 1);
    grid_->setColumnStretch(grid_->columnCount(), 1);

    connect(
        &params_.panelInterpolation->avgAlphaMax, _SLOT_(QDoubleSpinBox, valueChanged, double),
        [this]() { update(); });

    connect(&cbFlat_, &QCheckBox::toggled, [this]() { update(); });

    update();
}

void TabGraph::set(PeakInfos rs) {
    rs_ = rs;
    update();
}

void TabGraph::update() {
    avgAlphaMax_ = params_.panelInterpolation->avgAlphaMax.value();
    flat_ = cbFlat_.isChecked();
    QWidget::update();
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

//! Point in floating-point precision
QPointF TabGraph::p(deg alpha, deg beta) const {
    qreal r = r_ * alpha / alphaMax_;
    rad betaRad = beta.toRad();
    return QPointF(r * cos(betaRad), -r * sin(betaRad));
}

deg TabGraph::alpha(const QPointF& p) const {
    return sqrt(p.x() * p.x() + p.y() * p.y()) / r_ * alphaMax_;
}

deg TabGraph::beta(const QPointF& p) const {
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

    /*
[Jan:] As I read the code: the body of the 'for' loop (for all points) is guarded by
'if (qIsFinite(inten))'. NaNs are not finite, so they do not get painted.

Inside the outer 'if' (for finite inten) is 'if (flat_) ... else' where the 'then'
branch paints all points blue and the same size (.5), and the 'else' branch paints
them in various colours and size according to intensity.

The 'flat_' flag is controlled by the check box that is in the corner of the pole graph.

NaNs (intensities) do not occur in computed points, only in interpolated points,
when interpolation fails.
    */
    for (const PeakInfo& r : rs_) {
        qreal inten = r.inten();
        if (!qIsFinite(inten)) // nan comes from interpolation
            continue;
        const QPointF& pp = p(r.alpha(), r.beta());
        if (flat_) {
            QColor color(Qt::blue);
            p_->setPen(color);
            p_->setBrush(color);
            circle(pp, .5);
        } else {
            inten /= rgeMax;
            QColor color = colormap::intenGraph(inten, 1);
            p_->setPen(color);
            p_->setBrush(color);
            circle(pp, inten * r_ / 60); // TODO scale to max inten
        }
    }
}

// ************************************************************************** //
//  local class TabPoleFiguresSave
// ************************************************************************** //

//! Tab in PoleFiguresFrame, to save the pole figure data.

class TabPoleFiguresSave : public TabSave {
public:
    TabPoleFiguresSave();

    bool onlySelectedRefl() const;
    bool outputInten() const;
    bool outputTth() const;
    bool outputFWHM() const;

    void rawReflSettings(bool on);

private:
    CCheckBox outputInten_ {"outputInten#", "Intensity pole figure"};
    CCheckBox outputTth_ {"outputTth#", "Peak position pole figure"};
    CCheckBox outputFWHM_ {"outputFWHM#", "TWHM pole figure"};
    CRadioButton rbSelectedRefl_ {"rbSelectedRefl#", "Selected peak"};
    CRadioButton rbAllRefls_ {"rbAllRefls#", "All peaks"};
};

TabPoleFiguresSave::TabPoleFiguresSave()
    : TabSave(false)
{
    auto hb = new QHBoxLayout();
    grid_->addLayout(hb, grid_->rowCount(), 0);
    grid_->setRowStretch(grid_->rowCount(), 1);

    auto p1 = new GridPanel("Output data");
    auto p2 = new GridPanel("To save");

    hb->addWidget(p1);
    hb->addWidget(p2);
    hb->addStretch();

    {
        QGridLayout* g = &p1->grid_;
        g->addWidget(&outputInten_);
        g->addWidget(&outputTth_);
        g->addWidget(&outputFWHM_);
        g->setRowStretch(g->rowCount(), 1);
    }

    {
        QGridLayout* g = &p2->grid_;
        g->addWidget(&rbSelectedRefl_);
        g->addWidget(&rbAllRefls_);
        g->addWidget(new XTextButton(actSave), 2, 1);
        g->setRowStretch(g->rowCount(), 1);
    }

    rbSelectedRefl_.setChecked(true);
    outputInten_.setChecked(true);
}

bool TabPoleFiguresSave::onlySelectedRefl() const {
    return rbSelectedRefl_.isChecked();
}

bool TabPoleFiguresSave::outputInten() const {
    return outputInten_.isChecked();
}

bool TabPoleFiguresSave::outputTth() const {
    return outputTth_.isChecked();
}

bool TabPoleFiguresSave::outputFWHM() const {
    return outputFWHM_.isChecked();
}

void TabPoleFiguresSave::rawReflSettings(bool on) {
    outputTth_.setEnabled(on);
    outputFWHM_.setEnabled(on);
}


// ************************************************************************** //
//  class PoleFiguresFrame
// ************************************************************************** //

static const Params::ePanels PANELS =
    Params::ePanels(Params::REFLECTION | Params::GAMMA | Params::POINTS | Params::INTERPOLATION);

PoleFiguresFrame::PoleFiguresFrame()
    : Frame("polfig#", "Pole figure", new Params(PANELS))
{
    {
        auto* tab = new QWidget();
        tabs_->addTab(tab, "Graph");
        tab->setLayout(new QVBoxLayout());
        tabGraph_ = new TabGraph(*params_);
        tab->layout()->addWidget(tabGraph_);
    }
    {
        auto* tab = new QWidget();
        tabs_->addTab(tab, "Save");
        tab->setLayout(new QVBoxLayout());
        tabSave_ = new TabPoleFiguresSave();
        tab->layout()->addWidget(tabSave_);
        connect( tabSave_->actSave, &QAction::triggered, [this]() { savePoleFigureOutput(); });
    }
    show();
}

PoleFiguresFrame::~PoleFiguresFrame() {
    delete tabSave_;
    delete tabGraph_;
}

void PoleFiguresFrame::displayPeak(int reflIndex, bool interpolated) {
    Frame::displayPeak(reflIndex, interpolated);
    if (!interpPoints_.isEmpty() && !calcPoints_.isEmpty())
        tabGraph_->set((interpolated ? interpPoints_ : calcPoints_).at(reflIndex));
    tabSave_->rawReflSettings(!gSession->peaks().at(reflIndex).isRaw());
}

void PoleFiguresFrame::savePoleFigureOutput() {
    int reflCount = gSession->peaks().count();
    ASSERT(reflCount); // user should not get here if no peak is defined
    QString path = tabSave_->filePath(false);
    if (path.isEmpty()) {
        qWarning() << "cannot save pole figure: file path is empty";
        return;
    }
    if (tabSave_->onlySelectedRefl()) {
        writePoleFigureOutputFiles(path, getReflIndex());
        return;
    }
    // all peaks
    for_i (reflCount) // TODO collect output into one message
        writePoleFigureOutputFiles(path, i);
}

static QString const OUT_FILE_TAG(".refl%1");
static int const MAX_LINE_LENGTH_POL(9);

void PoleFiguresFrame::writePoleFigureOutputFiles(const QString& filePath, int index) {
    PeakInfos reflInfo;
    if (getInterpolated())
        reflInfo = interpPoints_.at(index);
    else
        reflInfo = calcPoints_.at(index);
    bool withFit = !gSession->peaks().at(index).isRaw();
    QString path = QString(filePath + OUT_FILE_TAG).arg(index + 1);
    bool check = false;
    int numSavedFiles = 0;

    if (tabSave_->outputInten()) {
        vec<qreal> output;
        for_i (reflInfo.count())
            output.append(reflInfo.at(i).inten());
        const QString intenFilePath = path + ".inten";
        writeListFile(intenFilePath, reflInfo, output);
        writePoleFile(intenFilePath, reflInfo, output);
        writeErrorMask(intenFilePath, reflInfo, output);
        check = true;
        numSavedFiles += 3;
    }

    if (tabSave_->outputTth() && withFit) {
        vec<qreal> output;
        for_i (reflInfo.count())
            output.append(reflInfo.at(i).tth());
        const QString tthFilePath = filePath + ".tth";
        writeListFile(tthFilePath, reflInfo, output);
        writePoleFile(tthFilePath, reflInfo, output);
        check = true;
        numSavedFiles += 2;
    }

    if (tabSave_->outputFWHM() && withFit) {
        vec<qreal> output;
        for_i (reflInfo.count())
            output.append(reflInfo.at(i).fwhm());
        const QString fwhmFilePath = filePath + ".fwhm";
        writeListFile(fwhmFilePath, reflInfo, output);
        writePoleFile(fwhmFilePath, reflInfo, output);
        check = true;
        numSavedFiles += 2;
    }

    if (numSavedFiles > 0) {
        if (check)
            qDebug() /* qInfo() TODO restore */ << numSavedFiles << " files have been saved";
        else
            qWarning() << "something went wrong, yet " << numSavedFiles << " files have been saved";
    } else
        qWarning() << "no files saved";
}

void PoleFiguresFrame::writeErrorMask(
    const QString& filePath, PeakInfos reflInfo, const vec<qreal>& output) {
    QFile* file = file_dialog::OutputFile("file", this, filePath);
    if (!file)
        return;
    QTextStream stream(file);

    for (int j = 0, jEnd = reflInfo.count(); j < jEnd; j += 9) {
        int max = j + MAX_LINE_LENGTH_POL;
        for (int i = j; i < max; i++) {
            if (qIsNaN(output.at(i)))
                stream << "0 ";
            else
                stream << "1 ";
        }
        stream << '\n';
    }
}

void PoleFiguresFrame::writePoleFile(
    const QString& filePath, PeakInfos reflInfo, const vec<qreal>& output)
{
    QFile* file = file_dialog::OutputFile("file", this, filePath);
    if (!file)
        return;
    QTextStream stream(file);

    for (int j = 0, jEnd = reflInfo.count(); j < jEnd; j += 9) {
        int max = j + MAX_LINE_LENGTH_POL;
        for (int i = j; i < max; i++) {
            if (qIsNaN(output.at(i)))
                stream << " -1  ";
            else
                stream << output.at(i) << " ";
        }
        stream << '\n';
    }
}

void PoleFiguresFrame::writeListFile(
    const QString& filePath, PeakInfos reflInfo, const vec<qreal>& output)
{
    QFile* file = file_dialog::OutputFile("file", this, filePath);
    QTextStream stream(file);

    for_i (reflInfo.count()) {
        stream << qreal(reflInfo.at(i).alpha()) << " " << qreal(reflInfo.at(i).beta()) << " "
               << output.at(i) << '\n';
    }
}
