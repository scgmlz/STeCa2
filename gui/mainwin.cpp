//  ***********************************************************************************************
//
//  Steca: stress and texture calculator
//
//! @file      gui/mainwin.cpp
//! @brief     Implements class MainWin
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
//  ***********************************************************************************************

#include "mainwin.h"
#include "core/typ/async.h"
#include "core/algo/fitting.h"
#include "core/algo/interpolate_polefig.h"
#include "core/session.h"
#include "core/def/settings.h"
#include "gui/state.h"
#include "gui/actions/menus.h"
#include "gui/actions/image_trafo_actions.h"
#include "gui/actions/toggles.h"
#include "gui/actions/triggers.h"
#include "gui/panels/subframe_dfgram.h"
#include "gui/panels/subframe_files.h"
#include "gui/panels/mainframe.h"
#include "gui/panels/subframe_clusters.h"
#include "gui/panels/subframe_metadata.h"
#include "gui/panels/subframe_setup.h"
#include "gui/dialogs/file_dialog.h"
#include <QStatusBar>
#include <QStringBuilder> // for ".." % ..
#include <iostream> // debug

MainWin* gGui; //!< global pointer to _the_ main window

namespace {
const QString dataFormats {"Data files (*.dat *.mar*);;All files (*.*)"};
}

//  ***********************************************************************************************
//! @class MainWin

MainWin::MainWin()
{
    gGui = this;

    triggers = new Triggers;
    toggles = new Toggles;
    state = new GuiState;
    imageTrafoActions = new ImageTrafoActions;
    menus_ = new Menus(menuBar());

    setWindowIcon(QIcon(":/icon/retroStier"));
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    // inbound signals
    QObject::connect(gSession, &Session::sigFiles, this, &MainWin::updateAbilities);
    QObject::connect(gSession, &Session::sigCorr, this, &MainWin::updateAbilities);
    QObject::connect(gSession, &Session::sigPeaks, this, &MainWin::updateAbilities);
    QObject::connect(gSession, &Session::sigBaseline, this, &MainWin::updateAbilities);

    QObject::connect(gSession, &Session::sigDoFits, this, &MainWin::runFits);
    QObject::connect(gSession, &Session::sigInterpol, this, &MainWin::runInterpolation);

    setAttribute(Qt::WA_DeleteOnClose, true);
    initLayout();
    readSettings();
    updateAbilities();

    setContentsMargins(5,5,5,5);
    statusBar()->addWidget(&progressBar);
}

MainWin::~MainWin()
{
    saveSettings();
    // the following deletions are obligatory to prevent a crash upon closing this MainWin:
    delete imageTrafoActions;
    delete triggers;
    delete toggles;
    // whereas all the following only reduces the number of perfectly inconsequential leaks:
    delete menus_;
    delete mainframe_;
    delete frameDfgram_;
    delete frameSetup_;
    delete dockMetadata_;
    delete dockClusters_;
    delete dockFiles_;
    gGui = nullptr;
}

void MainWin::initLayout()
{
    addDockWidget(Qt::LeftDockWidgetArea, (dockFiles_ = new SubframeFiles()));
    addDockWidget(Qt::LeftDockWidgetArea, (dockClusters_ = new SubframeClusters()));
    addDockWidget(Qt::LeftDockWidgetArea, (dockMetadata_ = new SubframeMetadata()));

    splTop_.setChildrenCollapsible(false);
    splTop_.addWidget(frameSetup_ = new SubframeSetup());
    splTop_.addWidget(mainframe_ = new Mainframe());
    splTop_.setStretchFactor(1, 1);

    splMain_.setChildrenCollapsible(false);
    splMain_.addWidget(&splTop_);
    splMain_.addWidget(frameDfgram_ = new SubframeDfgram());
    splMain_.setStretchFactor(1, 1);
    setCentralWidget(&splMain_);

    statusBar();
}

void MainWin::updateAbilities()
{
    bool hasFile = gSession->dataset().countFiles();
    bool hasCorr = gSession->hasCorrFile();
    bool hasPeak = gSession->peaks().count();
    bool hasBase = gSession->baseline().ranges().count();
    triggers->corrFile.setIcon(QIcon(hasCorr ? ":/icon/rem" : ":/icon/add"));
    toggles->enableCorr.setChecked(gSession->corrset().isEnabled());
    QString text = QString(hasCorr ? "Remove" : "Add") + " correction file";
    triggers->corrFile.setText(text);
    triggers->corrFile.setToolTip(text.toLower());
    triggers->removeFile.setEnabled(hasFile);
    triggers->removePeak.setEnabled(hasPeak);
    triggers->clearBackground.setEnabled(hasBase);
    triggers->exportDfgram.setEnabled(hasFile);
    triggers->exportBigtable.setEnabled(hasFile && hasPeak);
    triggers->exportDiagram.setEnabled(hasFile && hasPeak);
    menus_->export_->setEnabled(hasFile);
    menus_->image_->setEnabled(hasFile);
    menus_->dgram_->setEnabled(hasFile);
}


//! Stores native defaults as initialState_, then reads from config file.
void MainWin::readSettings()
{
    if (initialState_.isEmpty())
        initialState_ = saveState();
    XSettings s("MainWin");
    restoreGeometry(s.value("geometry").toByteArray());
    restoreState(s.value("state").toByteArray());
}

void MainWin::saveSettings() const
{
    XSettings s("MainWin");
    s.setValue("geometry", saveGeometry()); // this mainwindow's widget geometry
    s.setValue("state", saveState()); // state of this mainwindow's toolbars and dockwidgets
}

void MainWin::viewReset()
{
    restoreState(initialState_);
#ifndef Q_OS_OSX
    toggles->fullScreen.setChecked(false);
#endif
    toggles->viewStatusbar.setChecked(true);
    toggles->viewClusters.setChecked(true);
    toggles->viewFiles.setChecked(true);
    toggles->viewMetadata.setChecked(true);
}

void MainWin::loadSession()
{
    QString fileName = file_dialog::queryImportFileName(
        this, "Load session", sessionDir_, "Session files (*.ste)");
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    if (!(file.open(QIODevice::ReadOnly | QIODevice::Text))) {
        qWarning() << ("Cannot open file for reading: " % fileName);
        return;
    }
    try {
        TakesLongTime __("loadSession");
        gSession->sessionFromJson(file.readAll());
    } catch(Exception& ex) {
        qWarning() << "Could not load session from file " << fileName << ":\n"
                   << ex.msg() << "\n"
                   << "The application may now be in an inconsistent state.\n"
                   << "Please consider to quit the application, and start afresh.\n";
        gSession->clear();
    }
}

void MainWin::saveSession()
{
    QString fileName = file_dialog::queryExportFileName(
        this, "Save session", sessionDir_, "Session files (*.ste)");
    if (!fileName.endsWith(".ste"))
        fileName += ".ste";
    QFileInfo fileInfo(fileName);
    QFile* file = file_dialog::openFileConfirmOverwrite("file", this, fileInfo.filePath());
    if (!file)
        return;
    const int result = file->write(gSession->serializeSession());
    delete file;
    if (!(result >= 0))
        qWarning() << "Could not write session";
}

void MainWin::addFiles()
{
    QStringList fileNames
        = file_dialog::queryImportFileNames(this, "Add files", dataDir_, dataFormats);
    repaint();
    if (fileNames.isEmpty())
        return;
    TakesLongTime __("addFiles");
    gSession->dataset().addGivenFiles(fileNames);
}

void MainWin::loadCorrFile()
{
    if (gSession->corrset().hasFile()) {
        gSession->corrset().removeFile();
    } else {
        QString fileName = file_dialog::queryImportFileName(
            this, "Set correction file", dataDir_, dataFormats);
        if (fileName.isEmpty())
            return;
        gSession->corrset().loadFile(fileName);
    }
}

void MainWin::runFits()
{
    if (!gSession->peaks().count()) {
        gSession->setDirectPeakInfos({});
        gSession->setInterpolatedPeakInfos({});
        return;
    }
    algo::projectIntensities(&gGui->progressBar);
    algo::rawFits(&gGui->progressBar);
    algo::interpolateInfos(&gGui->progressBar);
    EMITS("MainWin::runFits", gSession->sigRawFits());
}

void MainWin::runInterpolation()
{
    algo::interpolateInfos(&gGui->progressBar);
    EMITS("MainWin::runInterpolation", gSession->sigRawFits());
}
