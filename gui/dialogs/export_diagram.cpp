//  ***********************************************************************************************
//
//  Steca: stress and texture calculator
//
//! @file      gui/dialogs/export_diagram.cpp
//! @brief     Implements class ExportDiagram
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
//  ***********************************************************************************************

#include "export_diagram.h"
#include "core/session.h"
#include "core/def/idiomatic_for.h"
#include "gui/dialogs/exportfile_dialogfield.h"
#include "gui/mainwin.h"
#include "gui/state.h"

//  ***********************************************************************************************
//! @class ExportDiagram

ExportDiagram::ExportDiagram()
    : CModal("exportDiagram")
    , QDialog(gGui)
{
    fileField_ = new ExportfileDialogfield(this, true, [this]()->void{save();});

    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle("Export diagram data");

    setLayout(fileField_);
}

void ExportDiagram::save()
{
    QFile* file = fileField_->file();
    if (!file)
        return;
    QTextStream stream(file);

    QString separator = fileField_->separator();

    // get data
    const int xi = int(gGui->state->diagramX->currentIndex());
    const int yi = int(gGui->state->diagramY->currentIndex());
    QVector<double> xs, ys, ysLow, ysHig;
    gSession->peakInfos().get4(xi, yi, xs, ys, ysLow, ysHig);
    if (!xs.count()) {
        qWarning() << "no data available";
        return;
    }

    // write data table
    for_i (xs.count()) {
        stream << xs.at(i) << separator << ys.at(i);
        if (ysLow.count())
            stream << separator << ysLow.at(i) << separator << ysHig.at(i);
        stream << '\n';
    }

    close();
}
