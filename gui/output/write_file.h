// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      gui/output/write_file.h
//! @brief     Declares class WriteFile
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef WRITE_FILE_H
#define WRITE_FILE_H

#include "def/macros.h"
#include <QFile>

class WriteFile : public QFile {
public:
    WriteFile(rcstr path) THROWS;
};

#endif // WRITE_FILE_H
