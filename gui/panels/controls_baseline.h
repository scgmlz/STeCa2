// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/panels/controls_baseline.h
//! @brief     Defines class SubframeSetup
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef CONTROLS_BASELINE_H
#define CONTROLS_BASELINE_H

#include "gui/base/controls.h"

//! A widget with controls to change the baseline fitting.

class ControlsBaseline : public QWidget {
public:
    ControlsBaseline();
private:
    CSpinBox spinDegree_ {"degree", 4, false, 0, 4,
            "Degree of the polynomial used to fit the baseline"};
};

#endif // CONTROLS_BASELINE_H
