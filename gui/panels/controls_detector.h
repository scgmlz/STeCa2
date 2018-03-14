// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/panels/controls_detector.h
//! @brief     Defines class ControlsDetector
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef CONTROLS_DETECTOR_H
#define CONTROLS_DETECTOR_H

#include <QVBoxLayout>
#include <QWidget>

//! Aggregated control widgets that govern the detector setup.

class ControlsDetector : public QWidget {
public:
    ControlsDetector();
private:
    QVBoxLayout vbox_;
};

#endif // CONTROLS_DETECTOR_H
