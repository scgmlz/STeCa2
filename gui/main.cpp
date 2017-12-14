// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      gui/main.cpp
//! @brief     Implements ...
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "app.h"
#include "../manifest.h"
#include "mainwin.h"
#include <tclap/CmdLine.h> // templated command line argument parser, in 3rdparty directory
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char* argv[]) {

    TCLAP::CmdLine cmd("Stress and texture calculator", ' ',
#include "../VERSION"
                       , true);
    cmd.parse(argc, argv);

    QApplication app(argc, argv);

    app.setApplicationName(APPLICATION_NAME);
    app.setApplicationVersion(
#include "../VERSION"
        );
    app.setOrganizationName(ORGANIZATION_NAME);
    app.setOrganizationDomain(ORGANIZATION_DOMAIN);

#if defined(Q_OS_OSX)
    app.setStyle(QStyleFactory::create("Macintosh"));
#elif defined(Q_OS_WIN)
    app.setStyle(QStyleFactory::create("Fusion"));
#else
    app.setStyle(QStyleFactory::create("Fusion"));
#endif

    gui::MainWin mainWin;
    mainWin.show();

    return app.exec();
}
