// ************************************************************************************************
//
//  Steca: stress and texture calculator
//
//! @file      gui/actions/triggers.h
//! @brief     Defines class Triggers
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************************************

#ifndef TRIGGERS_H
#define TRIGGERS_H

#include "gui/base/controls.h"

//! Collection of trigger actions, for use as member of MainWin.

class Triggers : private QObject {
public:
    Triggers();
    CTrigger about {"about", "About Steca"};
    CTrigger addFiles {"addFiles", "Add files...", ":/icon/add", Qt::CTRL | Qt::Key_O};
    CTrigger addPeak {"addPeak", "Add peak", ":/icon/add"};
    CTrigger checkUpdate {"checkUpdate", "Check for update"};
    CTrigger clearBackground {"clearBackground", "Clear background regions", ":/icon/clear"};
    CTrigger clearPeaks {"clearPeaks", "Clear peaks", ":/icon/clear"};
    CTrigger clearSession {"clearSession", "Clear session"};
    CTrigger corrFile {"corrFile", "Add correction file", ":/icon/add",
            Qt::SHIFT | Qt::CTRL | Qt::Key_O};
    CTrigger exportDfgram {"exportDfgram", "Export diffractogram(s)...", ":/icon/filesave" };
    CTrigger exportPolefig {"exportPolefig", "Export pole figure...", ":/icon/filesave" };
    CTrigger exportBigtable {"exportBigtable", "Export fit result table...", ":/icon/filesave" };
    CTrigger exportDiagram {"exportDiagram", "Export diagram...", ":/icon/filesave" };
    CTrigger spawnTable {"spawnTable", "Spawn table...", ":/icon/window" };
    CTrigger spawnDiagram {"spawnDiagram", "Spawn diagram...", ":/icon/window" };
    CTrigger spawnPolefig {"spawnPolefig", "Spawn pole figure...", ":/icon/window" };
    CTrigger loadSession {"loadSession", "Load session..."};
    CTrigger online  {"online", "Open docs in external browser"};
    CTrigger quit {"quit", "Quit", "", QKeySequence::Quit};
    CTrigger removeFile {"removeFile", "Remove highlighted file", ":/icon/rem",
            QKeySequence::Delete};
    CTrigger removePeak {"removePeak", "Remove peak", ":/icon/rem"};
    CTrigger saveSession {"saveSession", "Save session..."};
    CTrigger viewReset {"viewReset", "Reset"};
};

#endif // TRIGGERS_H
