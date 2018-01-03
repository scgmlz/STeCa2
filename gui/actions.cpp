// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      gui/actions.cpp
//! @brief     Implements functions newTrigger, newToggle
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "actions.h"
#include <QAction>
#include <QApplication>

namespace gui {

QAction* newTrigger(rcstr text, rcstr iconFile) {
    QAction* ret = new QAction(text, qApp);
    ret->setToolTip(text.toLower());
    if (iconFile!="")
        ret->setIcon(QIcon(iconFile));
    return ret;
};

QAction* newToggle(rcstr text, rcstr iconFile) {
    QAction* ret = new QAction(text, qApp);
    ret->setToolTip(text.toLower());
    if (iconFile!="")
        ret->setIcon(QIcon(iconFile));
    ret->setCheckable(true);
    return ret;
};

} // namespace gui
