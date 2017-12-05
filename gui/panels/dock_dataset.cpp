// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      gui/panels/dock_dataset.cpp
//! @brief     Implements ...
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "dock_dataset.h"
#include "gui_cfg.h"
#include "thehub.h"
#include "views.h"

namespace gui {
namespace panel {

class DatasetView : public views::ListView {
    CLASS(DatasetView) SUPER(views::ListView) public : DatasetView(TheHub&);

protected:
    void currentChanged(QModelIndex const&, QModelIndex const&);

    using Model = models::DatasetsModel;
    Model* model() const { return static_cast<Model*>(super::model()); }
};

DatasetView::DatasetView(TheHub& hub) : super(hub) {
    setModel(&hub.datasetsModel);
    EXPECT(dynamic_cast<Model*>(super::model()))

    onSigDatasetsChanged([this]() {
        tellDatasetSelected(data::shp_Dataset()); // first de-select
        selectRow(0);
    });
}

void DatasetView::currentChanged(QModelIndex const& current, QModelIndex const& previous) {
    super::currentChanged(current, previous);
    tellDatasetSelected(model()->data(current, Model::GetDatasetRole).value<data::shp_Dataset>());
}

DockDatasets::DockDatasets(TheHub& hub)
    : super("Datasets", "dock-datasets", Qt::Vertical), RefHub(hub) {
    box_->addWidget((datasetView_ = new DatasetView(hub)));

    auto h = hbox();
    box_->addLayout(h);

    h->addWidget(label("Combine:"));
    h->addWidget(combineDatasets_ = spinCell(gui_cfg::em4, 1));
    combineDatasets_->setToolTip("Combine and average number of datasets");

    connect(combineDatasets_, slot(QSpinBox, valueChanged, int), [this](int num) {
        hub_.combineDatasetsBy(pint(qMax(1, num)));
    });

    onSigDatasetsChanged(
        [this]() { combineDatasets_->setValue(to_i(uint(hub_.datasetsGroupedBy()))); });
}
}
}
