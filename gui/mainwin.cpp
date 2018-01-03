// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      gui/mainwin.cpp
//! @brief     Implements class MainWin
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "mainwin.h"
#include "../manifest.h"
#include "popup/about.h"
#include "popup/filedialog.h"
#include "output/output_diagrams.h"
#include "output/output_diffractograms.h"
#include "output/output_polefigures.h"
#include "panels/dock_dataset.h"
#include "panels/dock_files.h"
#include "panels/dock_metadata.h"
#include "panels/tabs_diffractogram.h"
#include "panels/tabs_images.h"
#include "panels/tabs_setup.h"
#include "session.h"
#include "cfg/settings.h"
#include "thehub.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QMenuBar>
#include <QMessageBox>
#include <QNetworkReply>
#include <QSplitter>
#include <QStatusBar>
#include <QStringBuilder> // for ".." % ..

gui::TheHub* gHub;

namespace gui {

MainWin::MainWin() {
    qDebug() << "MainWin/";
    gHub = new TheHub();
    setWindowIcon(QIcon(":/icon/retroStier"));
    QDir::setCurrent(QDir::homePath());

    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    initMenus();
    initLayout();
    initStatusBar();
    connectActions();

    readSettings();
}

void MainWin::initMenus() {
    auto separator = [this]() {
        auto act = new QAction(this);
        act->setSeparator(true);
        return act;
    };

    auto* mbar = menuBar();
#ifdef Q_OS_OSX
    mbar->setNativeMenuBar(false); // REVIEW
#else
    mbar->setNativeMenuBar(true);
#endif

    menuFile_ = mbar->addMenu("&File");
    menuImage_ = mbar->addMenu("&Image");
    menuDgram_ = mbar->addMenu("Di&ffractogram");
    menuOutput_ = mbar->addMenu("&Output");
    menuView_ = mbar->addMenu("&View");
    menuHelp_ = mbar->addMenu("&Help");

    addActions(
        menuFile_,
        {
            gHub->trigger_addFiles, gHub->trigger_removeFile, separator(), gHub->toggle_enableCorr, gHub->trigger_remCorr,
            separator(), gHub->trigger_loadSession,
            gHub->trigger_saveSession, // TODO add: gHub->trigger_clearSession,
        });

    addActions(
        menuFile_,
        {
#ifndef Q_OS_OSX // Mac puts Quit into the Apple menu
            separator(),
#endif
            gHub->trigger_quit,
        });

    addActions(
        menuView_,
        {
            gHub->toggle_viewFiles, gHub->toggle_viewDatasets, gHub->toggle_viewMetadata, separator(),
#ifndef Q_OS_OSX
            gHub->toggle_fullScreen,
#endif
            gHub->toggle_viewStatusbar, separator(), gHub->trigger_viewReset,
        });

    addActions(
        menuImage_,
        {
            gHub->trigger_rotateImage, gHub->toggle_mirrorImage, gHub->toggle_fixedIntenImage, gHub->toggle_linkCuts,
            gHub->toggle_showOverlay, gHub->toggle_stepScale, gHub->toggle_showBins,
        });

    addActions(
        menuDgram_,
        {
            gHub->toggle_selRegions, gHub->toggle_showBackground, gHub->trigger_clearBackground, gHub->trigger_clearReflections,
            separator(), gHub->trigger_addReflection, gHub->trigger_remReflection, separator(), gHub->toggle_combinedDgram,
            gHub->toggle_fixedIntenDgram,
        });

    addActions(
        menuOutput_,
        {
            gHub->trigger_outputPolefigures, gHub->trigger_outputDiagrams, gHub->trigger_outputDiffractograms,
        });

    addActions(
        menuHelp_,
        {
            gHub->trigger_about,
#ifndef Q_OS_OSX
            separator(), // Mac puts About into the Apple menu
#endif
            gHub->trigger_online, gHub->trigger_checkUpdate,
        });
    qDebug() << "/MainWin";
}

void MainWin::addActions(QMenu* menu, QList<QAction*> actions) {
    debug::ensure(menu);
    menu->addActions(actions);
    str prefix = str("%1: ").arg(menu->title().remove('&'));
    for (auto action : actions)
        action->setToolTip(prefix + action->toolTip());
}

void MainWin::initLayout() {
    addDockWidget(Qt::LeftDockWidgetArea, (dockFiles_ = new panel::DockFiles()));
    addDockWidget(Qt::LeftDockWidgetArea, (dockDatasets_ = new panel::DockDatasets()));
    addDockWidget(Qt::LeftDockWidgetArea, (dockDatasetInfo_ = new panel::DockMetadata()));

    auto splMain = new QSplitter(Qt::Vertical);
    splMain->setChildrenCollapsible(false);

    auto splTop = new QSplitter(Qt::Horizontal);
    splTop->setChildrenCollapsible(false);

    setCentralWidget(splMain);

    splMain->addWidget(splTop);
    splMain->addWidget(new panel::TabsDiffractogram());
    splMain->setStretchFactor(1, 1);

    splTop->addWidget(new panel::TabsSetup());
    splTop->addWidget(new panel::TabsImages());
    splTop->setStretchFactor(1, 1);
}

void MainWin::initStatusBar() {
    statusBar();
}

void MainWin::connectActions() {
    auto connectTrigger = [this](QAction* action, void (MainWin::*fun)()) {
        QObject::connect(action, &QAction::triggered, this, fun);
    };

    auto connectToggle = [this](QAction* action, void (MainWin::*fun)(bool)) {
        QObject::connect(action, &QAction::toggled, this, fun);
    };

    connectTrigger(gHub->trigger_loadSession, &MainWin::loadSession);
    connectTrigger(gHub->trigger_saveSession, &MainWin::saveSession);
    connectTrigger(gHub->trigger_clearSession, &MainWin::clearSession);

    connectTrigger(gHub->trigger_addFiles, &MainWin::addFiles);
    connectTrigger(gHub->toggle_enableCorr, &MainWin::enableCorr);

    connectTrigger(gHub->trigger_quit, &MainWin::close);

    connectTrigger(gHub->trigger_outputPolefigures, &MainWin::outputPoleFigures);
    connectTrigger(gHub->trigger_outputDiagrams, &MainWin::outputDiagrams);
    connectTrigger(gHub->trigger_outputDiffractograms, &MainWin::outputDiffractograms);

    connectTrigger(gHub->trigger_about, &MainWin::about);
    connectTrigger(gHub->trigger_online, &MainWin::online);
    QObject::connect(gHub->trigger_checkUpdate, &QAction::triggered, [this]() {checkUpdate();});

    connectToggle(gHub->toggle_viewStatusbar, &MainWin::viewStatusbar);
#ifndef Q_OS_OSX
    connectToggle(gHub->toggle_fullScreen, &MainWin::viewFullScreen);
#endif

    connectToggle(gHub->toggle_viewFiles, &MainWin::viewFiles);
    connectToggle(gHub->toggle_viewDatasets, &MainWin::viewDatasets);
    connectToggle(gHub->toggle_viewMetadata, &MainWin::viewMetadata);

    connectTrigger(gHub->trigger_viewReset, &MainWin::viewReset);
}

void MainWin::about() {
    AboutBox(this).exec();
}

void MainWin::online() {
    QDesktopServices::openUrl(QUrl(STECA2_PAGES_URL));
}

void MainWin::checkUpdate(bool completeReport) {

    QNetworkRequest req;

    str ver = qApp->applicationVersion();
    str qry = ver % "\t| " % QSysInfo::prettyProductName();
    req.setUrl(QUrl(str(STECA2_VERSION_URL) % "?" % qry));
    auto reply = netMan_.get(req);

    connect(reply, &QNetworkReply::finished, [this, completeReport, reply]() {
        if (QNetworkReply::NoError != reply->error()) {
            messageDialog("Network Error", reply->errorString());
        } else {
            str ver = qApp->applicationVersion();
            str lastVer = reply->readAll().trimmed();

            str name = qApp->applicationName();

            if (ver != lastVer)
                messageDialog(
                    str("%1 update").arg(name),
                    str("<p>The latest released %1 version is %2. You have "
                        "version %3.</p>"
                        "<p><a href='%4'>Get new %1</a></p>")
                        .arg(name, lastVer, ver, STECA2_DOWNLOAD_URL));
            else if (completeReport)
                messageDialog(
                    str("%1 update").arg(name),
                    str("<p>You have the latest released %1 version (%2).</p>").arg(name).arg(ver));
        }
    });
}

void MainWin::messageDialog(rcstr title, rcstr text) {
    QMessageBox::information(this, title, text);
}

void MainWin::show() {
    QMainWindow::show();
    onShow();
}

void MainWin::close() {
    QMainWindow::close();
}

void MainWin::addFiles() {
    QStringList fileNames = file_dialog::openFileNames(
        this, "Add files", QDir::current().absolutePath(),
        "Data files (*.dat *.mar*);;All files (*.*)");
    update();
    if (!fileNames.isEmpty()) {
        QDir::setCurrent(QFileInfo(fileNames.at(0)).absolutePath());
        gHub->addGivenFiles(fileNames);
    }
}

void MainWin::enableCorr() {
    str fileName;
    if (!gSession->hasCorrFile()) {
        fileName = file_dialog::openFileName(
            this, "Set correction file", QDir::current().absolutePath(),
            "Data files (*.dat *.mar*);;All files (*.*)");
        update();
    }
    if (!fileName.isEmpty()) {
        QDir::setCurrent(QFileInfo(fileName).absolutePath());
        gHub->setCorrFile(fileName);
    }
}

void MainWin::loadSession() {
    str fileName = file_dialog::openFileName(
        this, "Load session", QDir::current().absolutePath(),
        "Session files (*.ste);;All files (*.*)");
    update();
    if (fileName.isEmpty()) {
        TR("load session aborted");
        return;
    }
    try {
        TR("going to load session from file '"+fileName+"'");
        gHub->sessionFromFile(QFileInfo(fileName));
    } catch(Exception& ex) {
        qWarning() << "Could not load session from file " << fileName << ":\n"
                   << ex.msg() << "\n"
                   << "The application may now be in an inconsistent state.\n"
                   << "Please consider to quit the application, and start afresh.\n";
        clearSession();
    }
}

void MainWin::saveSession() {
    str fileName = file_dialog::saveFileName(
        this, "Save session", QDir::current().absolutePath(),
        "Session files (*.ste);;All files (*.*)");
    update();
    if (!fileName.endsWith(".ste"))
        fileName += ".ste";
    gHub->saveSession(QFileInfo(fileName));
}

void MainWin::clearSession() {
    gHub->clearSession();
}

void MainWin::outputPoleFigures() {
    auto popup = new output::PoleFiguresFrame("Pole Figures", this);
    popup->show();
}

void MainWin::outputDiagrams() {
    auto popup = new output::DiagramsFrame("Diagrams", this);
    popup->show();
}

void MainWin::outputDiffractograms() {
    auto popup = new output::DiffractogramsFrame("Diffractograms", this);
    popup->show();
}

void MainWin::closeEvent(QCloseEvent* event) {
    onClose();
    event->accept();
}

void MainWin::onShow() {
    checkActions();
    gHub->clearSession();

#ifdef DEVELOPMENT
    // automatic actions - load files & open dialog
    // helps with development

    auto safeLoad = [this](rcstr fileName) {
        QFileInfo info(fileName);
        if (info.exists())
            gHub->loadSession(info);
    };
    safeLoad("/home/jan/C/+dev/fz/data/0.ste");
    gHub->actions.outputPolefigures->trigger();
#endif

    Settings s("config");
    auto ver = qApp->applicationVersion();
    if (s.readStr("current version") != ver) {
        // new version
        s.saveStr("current version", ver);
        s.saveBool("startup check update", true);
        s.saveBool("startup about", true);
    }

    if (s.readBool("startup check update", true))
        checkUpdate(false);
    if (s.readBool("startup about", true))
        about();
}

void MainWin::onClose() {
    saveSettings();
}

void MainWin::readSettings() {
    if (initialState_.isEmpty())
        initialState_ = saveState();

    Settings s("MainWin");
    restoreGeometry(s.value("geometry").toByteArray());
    restoreState(s.value("state").toByteArray());
}

void MainWin::saveSettings() {
    Settings s("MainWin");
    s.setValue("geometry", saveGeometry());
    s.setValue("state", saveState());
}

void MainWin::checkActions() {
    gHub->toggle_viewStatusbar->setChecked(statusBar()->isVisible());

#ifndef Q_OS_OSX
    gHub->toggle_fullScreen->setChecked(isFullScreen());
#endif

    gHub->toggle_viewFiles->setChecked(dockFiles_->isVisible());
    gHub->toggle_viewDatasets->setChecked(dockDatasets_->isVisible());
    gHub->toggle_viewMetadata->setChecked(dockDatasetInfo_->isVisible());
}

void MainWin::viewStatusbar(bool on) {
    statusBar()->setVisible(on);
    gHub->toggle_viewStatusbar->setChecked(on);
}

void MainWin::viewFullScreen(bool on) {
    if (on)
        showFullScreen();
    else
        showNormal();

#ifndef Q_OS_OSX
    gHub->toggle_fullScreen->setChecked(on);
#endif
}

void MainWin::viewFiles(bool on) {
    dockFiles_->setVisible(on);
    gHub->toggle_viewFiles->setChecked(on);
}

void MainWin::viewDatasets(bool on) {
    dockDatasets_->setVisible(on);
    gHub->toggle_viewDatasets->setChecked(on);
}

void MainWin::viewMetadata(bool on) {
    dockDatasetInfo_->setVisible(on);
    gHub->toggle_viewMetadata->setChecked(on);
}

void MainWin::viewReset() {
    restoreState(initialState_);
    viewStatusbar(true);
    viewFullScreen(false);
    viewFiles(true);
    viewDatasets(true);
    viewMetadata(true);
}

} // namespace gui
