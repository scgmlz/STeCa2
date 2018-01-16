// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/base/various_widgets.h
//! @brief     Defines classes BoxWidget, DockWidget
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef VARIOUS_WIDGETS_H
#define VARIOUS_WIDGETS_H

#include "core/typ/str.h"
#include <QBoxLayout>
#include <QDockWidget>

//! a widget with a box layout

class BoxWidget : public QWidget {
public:
    BoxWidget(Qt::Orientation);
    QBoxLayout& box() const { return *box_; }

protected:
    QBoxLayout* box_;
};

//! a dock widget that acts as BoxWidget

class DockWidget : public QDockWidget {
public:
    DockWidget(rcstr name, rcstr objectName);

protected:
    QBoxLayout* box_;
};

#endif // VARIOUS_WIDGETS_H
