// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/dialogs/output_diffractograms.cpp
//! @brief     Implements class DiffractogramsFrame
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "gui/dialogs/output_diffractograms.h"
#include "core/session.h"
#include "gui/base/filedialog.h"
#include "gui/dialogs/dialog_panels.h"
#include "gui/dialogs/tab_save.h"
#include "gui/mainwin.h"
#include <cmath>
#include <QMessageBox>

namespace {

void writeCurve(QTextStream& stream, const Curve& curve, const Cluster* cluster,
                const Range& rgeGma, const QString& separator) {

    ASSERT(rgeGma.isValid());
    const Metadata* md = cluster->avgeMetadata().data();
    ASSERT(md);
    stream << "Comment: " << md->comment << '\n';
    stream << "Date: " << md->date << '\n';
    stream << "Gamma range min: " << rgeGma.min << '\n';
    stream << "Gamma range max: " << rgeGma.max << '\n';

    for_i (Metadata::numAttributes(true))
        stream << Metadata::attributeTag(i, true) << ": "
               << md->attributeValue(i).toDouble() << '\n';

    stream << "Tth" << separator << "Intensity" << '\n';
    for_i (curve.xs().count())
        stream << curve.x(i) << separator << curve.y(i) << '\n';

    stream.flush(); // not sure whether we need this
}

//! Returns templatedName with '%d' replaced by string representation of num.

//!  The string representation of num has leading zeros, and its number of
//!  digits is determined by the maximum value maxNum.

QString numberedName(const QString& templatedName, int num, int maxNum) {
    if (!templatedName.contains("%d"))
        qFatal("path does not contain placeholder %%d");
    QString ret = templatedName;
    int nDigits = (int)log10((double)maxNum)+1;
    ret.replace("%d", QString("%1").arg(num, nDigits, 10, QLatin1Char('0')));
    qDebug() << "PATH " << templatedName << " -> " << ret;
    return ret;
}

} // local method

static const Params::ePanels PANELS = Params::ePanels(Params::GAMMA);


// ************************************************************************** //
//  local class TabDiffractogramsSave
// ************************************************************************** //

//! The main part of DiffractogramsFrame. Extends TabSave by an output content control.

class TabDiffractogramsSave : public TabSave {
public:
    TabDiffractogramsSave();

    bool currentChecked() { return rbCurrent_.isChecked(); }
    bool allSequentialChecked() { return rbAllSequential_.isChecked(); }
    bool allChecked() { return rbAll_.isChecked(); }

private:
    CRadioButton rbCurrent_ {"rbCurrent", "Current diffractogram"};
    CRadioButton rbAllSequential_ {"rbAllSequential", "All diffractograms to numbered files"};
    CRadioButton rbAll_ {"rbAll", "All diffractograms to one file"};
};

TabDiffractogramsSave::TabDiffractogramsSave()
    : TabSave(true)
{
    auto gp = new GridPanel("To save");
    grid_->addWidget(gp, grid_->rowCount(), 0, 1, 2);
    grid_->setRowStretch(grid_->rowCount(), 1);

    QGridLayout* g = gp->grid();
    g->addWidget(&rbCurrent_);
    g->addWidget(&rbAllSequential_);
    g->addWidget(&rbAll_);
    g->addWidget(new XTextButton(actSave), 2, 1);

    rbAll_.setChecked(true);
}

// ************************************************************************** //
//  local class DFrame
// ************************************************************************** //

DFrame::DFrame(const QString& name, const QString& title)
    : QDialog(gGui)
    , CModal(name)
{
    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(title);

    setLayout((box_ = newQ::VBoxLayout()));

    parameterControls_ = newQ::HBoxLayout();
    box_->addLayout(parameterControls_);

    tabs_ = new QTabWidget();
    tabs_->setTabPosition(QTabWidget::North);
    box_->addWidget(tabs_);
    box_->setStretch(box_->count() - 1, 1);

    auto hb = newQ::HBoxLayout();
    box_->addLayout(hb);

    actClose_ = new CTrigger("actClose", "Close");

    hb->addWidget((btnClose_ = new XTextButton(actClose_)));
    hb->addStretch(1);
    hb->addWidget((progressBar_ = new QProgressBar));
    hb->setStretchFactor(progressBar_, 333);
    hb->addStretch(1);

    progressBar_->hide();

    connect(actClose_, &QAction::triggered, [this]() { close(); });
}

// ************************************************************************** //
//  class DiffractogramsFrame
// ************************************************************************** //

DiffractogramsFrame::DiffractogramsFrame()
    : DFrame("dgram", "Diffractograms")
{
    tabs_->removeTab(0);

    tabs_->addTab(&tab, "Save");
    tab.setLayout(newQ::VBoxLayout());
    tabSave_ = new TabDiffractogramsSave();
    tab.layout()->addWidget(tabSave_);
    connect(tabSave_->actSave, &QAction::triggered, [this]() { save(); });

    show();
}

DiffractogramsFrame::~DiffractogramsFrame() {
    qDebug() << "~DiffractogramsFrame";
    delete tabSave_;
    delete panelGammaSlices;
    delete panelGammaRange;
}

void DiffractogramsFrame::save() {
    if (tabSave_->currentChecked())
        saveCurrent();
    else if (tabSave_->allSequentialChecked())
        saveAll(false);
    else if (tabSave_->allChecked())
        saveAll(true);
    else
        qFatal("Invalid call of DiffractogramsFrame::saveDiffractogramOutput");
}

void DiffractogramsFrame::saveCurrent() {
    QString path = tabSave_->filePath(true);
    if (path.isEmpty())
        return;
    QFile* file = file_dialog::OutputFile("file", this, path);
    if (!file)
        return;
    QTextStream stream(file);
    const Cluster* cluster = gSession->dataset().highlight().cluster();
    ASSERT(cluster);
    shp_SequenceLens lens = gSession->defaultClusterLens(*cluster);
    const Curve& curve = lens->makeCurve();
    if (curve.isEmpty())
        qFatal("curve is empty");
    writeCurve(stream, curve, cluster, lens->rgeGma(), tabSave_->separator());
}

void DiffractogramsFrame::saveAll(bool oneFile) {

    const Experiment& expt = gSession->experiment();

    // In one-file mode, start output stream; in multi-file mode, only do prepations.
    QString path = tabSave_->filePath(true, !oneFile);
    if (path.isEmpty())
        return;
    QTextStream* stream = nullptr;
    if (oneFile) {
        QFile* file = file_dialog::OutputFile("file", this, path);
        if (!file)
            return;
        stream = new QTextStream(file);
    } else {
        // check whether any of the numbered files already exists
        QStringList existingFiles;
        for_i (expt.size()) {
            QString currPath = numberedName(path, i, expt.size()+1);
            if (QFile(currPath).exists())
                existingFiles << QFileInfo(currPath).fileName();
        }
        if (existingFiles.size() &&
            QMessageBox::question(this, existingFiles.size()>1 ? "Files exist" : "File exists",
                                  "Overwrite files " + existingFiles.join(", ") + " ?") !=
            QMessageBox::Yes)
            return;
    }

    ASSERT(params_->panelGammaSlices);
    int gmaSlices = params_->panelGammaSlices->numSlices.value();

    ASSERT(params_->panelGammaRange);
    const PanelGammaRange* pr = params_->panelGammaRange;
    Range rgeGma;
    if (pr->cbLimitGamma.isChecked())
        rgeGma.safeSet(pr->minGamma.value(), pr->maxGamma.value());

    Progress progress(expt.size(), progressBar_);

    int picNum = 0;
    int fileNum = 0;
    for (const Cluster* cluster : expt.clusters()) {
        ++picNum;
        progress.step();

        shp_SequenceLens lens = gSession->defaultClusterLens(*cluster);

        Range rge = (gmaSlices > 0) ? lens->rgeGma() : Range::infinite();
        if (rgeGma.isValid())
            rge = rge.intersect(rgeGma);

        gmaSlices = qMax(1, gmaSlices);
        const qreal step = rge.width() / gmaSlices;
        for_i (gmaSlices) {
            if (!oneFile) {
                QFile* file = file_dialog::OutputFile("file",
                    this, numberedName(path, ++fileNum, expt.size()+1), false);
                if (!file)
                    return;
                delete stream;
                stream = new QTextStream(file);
            }
            ASSERT(stream);

            const qreal min = rge.min + i * step;
            const Range gmaStripe(min, min + step);
            const Curve& curve = lens->makeCurve(gmaStripe);
            ASSERT(!curve.isEmpty());
            *stream << "Picture Nr: " << picNum << '\n';
            if (gmaSlices > 1)
                *stream << "Gamma slice Nr: " << i+1 << '\n';
            writeCurve(*stream, curve, cluster, gmaStripe, tabSave_->separator());
        }
    }
    delete stream;
}