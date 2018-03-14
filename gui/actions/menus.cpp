// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/actions/menus.cpp
//! @brief     Implements class Menus
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "menus.h"
#include "gui/mainwin.h"
#include "gui/actions/triggers.h"
#include "gui/actions/toggles.h"

//! Initialize the menu bar.
Menus::Menus(QMenuBar* mbar)
    : mbar_(mbar)
{
#ifdef Q_OS_OSX
    mbar->setNativeMenuBar(false);
#else
    mbar->setNativeMenuBar(true);
#endif

    Triggers* triggers = gGui->triggers;
    Toggles* toggles = gGui->toggles;

    actionsToMenu(
        "&File",
        {   &triggers->addFiles,
                &triggers->removeFile,
                separator(),
                &triggers->corrFile,
                &toggles->enableCorr,
                separator(),
                &triggers->loadSession,
                &triggers->saveSession,
                &triggers->clearSession,
#ifndef Q_OS_OSX // Mac puts Quit into the Apple menu
                separator(),
#endif
                &triggers->quit,
        });

    image_ = actionsToMenu(
        "&Image",
        {   &triggers->rotateImage,
                &toggles->mirrorImage,
                &toggles->fixedIntenImage,
                &toggles->linkCuts,
                &toggles->showOverlay,
                &toggles->showBins,
        });

    dgram_ = actionsToMenu(
        "&Diffractogram",
        {   &toggles->showBackground,
                &triggers->clearBackground,
                &triggers->clearPeaks,
                separator(),
                &triggers->addPeak,
                &triggers->removePeak,
                separator(),
                &toggles->combinedDgram,
                &toggles->fixedIntenDgram,
        });

    output_ = actionsToMenu(
        "&Output",
        {   &triggers->outputPolefigures,
                &triggers->outputDiagrams,
                &triggers->outputDiffractograms,
        });

    actionsToMenu(
        "&View",
        {   &toggles->viewFiles,
                &toggles->viewClusters,
                &toggles->viewMetadata,
                separator(),
#ifndef Q_OS_OSX
                &toggles->fullScreen,
#endif
                &toggles->viewStatusbar,
                separator(),
                &triggers->viewReset,
        });

    actionsToMenu(
        "&Help",
        {   &triggers->about, // Mac puts About into the Apple menu
                &triggers->online,
                &triggers->checkUpdate,
        });
}

QAction* Menus::separator() const {
    QAction* ret = new QAction(mbar_);
    ret->setSeparator(true);
    return ret;
};

QMenu* Menus::actionsToMenu(const char* menuName, QList<QAction*> actions) {
    QMenu* menu = mbar_->addMenu(menuName);
    menu->addActions(actions);
    QString prefix = QString("%1: ").arg(menu->title().remove('&'));
    for (auto action : actions)
        action->setToolTip(prefix + action->toolTip());
    return menu;
};
