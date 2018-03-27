// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/dialogs/popup_diagram.h
//! @brief     Defines class PopupDiagram
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef POPUP_DIAGRAM_H
#define POPUP_DIAGRAM_H

#include "gui/capture_and_replay/enhance_widgets.h"

//! Popup display of the present XY diagram.

class PopupDiagram : public CModelessDialog {
public:
    PopupDiagram();
private:
    class PlotDiagram* plot_;
};

#endif // POPUP_DIAGRAM_H