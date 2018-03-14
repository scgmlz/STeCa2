// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/panels/subframe_metadata.h
//! @brief     Defines class SubframeMetadata
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef SUBFRAME_METADATA_H
#define SUBFRAME_METADATA_H

#include "gui/base/layout.h"

//! Part of the main window that shows the metadata of the selected dataset
class SubframeMetadata : public DockWidget {
public:
    SubframeMetadata();
private:
    class MetadataView* metadataView_;
};

#endif // SUBFRAME_METADATA_H
