// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      gui/panels/dock_metadata.h
//! @brief     Defines class DockMetadata
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef DOCK_METADATA_H
#define DOCK_METADATA_H

#include "widgets/various_widgets.h"

namespace gui {
namespace panel {

class DockMetadata : public DockWidget {
public:
    DockMetadata();

private:
    class MetadataView* metadataView_;
};

} // namespace panel
} // namespace gui

#endif // DOCK_METADATA_H
