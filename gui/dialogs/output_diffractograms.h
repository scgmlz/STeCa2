// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/dialogs/output_diffractograms.h
//! @brief     Defines class DiffractogramsFrame
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef OUTPUT_DIFFRACTOGRAMS_H
#define OUTPUT_DIFFRACTOGRAMS_H

#include "frame.h"
#include <QWidget>

//! The modal dialog for saving diffractograms.

class DiffractogramsFrame : public QDialog, private CModal {
public:
    DiffractogramsFrame();
    ~DiffractogramsFrame();
    void onCommand(const QStringList&);

private:
    QAction *actClose_;
    QToolButton *btnClose_;
    QProgressBar* progressBar_;
    class PanelGammaSlices* panelGammaSlices;
    class PanelGammaRange* panelGammaRange;
    class TabDiffractogramsSave* tabSave_;

    void save();
    void saveCurrent();
    void saveAll(bool oneFile);
};

#endif // OUTPUT_DIFFRACTOGRAMS_H
