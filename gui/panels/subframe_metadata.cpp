// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/panels/subframe_metadata.cpp
//! @brief     Implements class SubframeMetadata, with local model and view
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "core/session.h"
#include "gui/panels/subframe_metadata.h"
#include "gui/base/model_view.h"


// ************************************************************************** //
//  local class MetadataModel
// ************************************************************************** //

//! The model for MetadatView.

class MetadataModel : public CheckTableModel {
public:
    MetadataModel() : CheckTableModel("meta") {}

    void reset();

    int columnCount() const final { return NUM_COLUMNS; }
    int rowCount() const final { return Metadata::numAttributes(false); }
    int highlighted() const final { return 0; }// gSession->dataset().highlight().clusterIndex(); }
    void setHighlight(int i) final { ; } //gSession->dataset().highlight().setCluster(i); }
    bool activated(int row) const { return gSession->metaSelected(row); }
    void setActivated(int row, bool on) { gSession->setMetaSelected(row, on); }

    QVariant data(const QModelIndex&, int) const;
    QVariant headerData(int, Qt::Orientation, int) const { return {}; }

    enum { COL_CHECK = 1, COL_TAG, COL_VALUE, NUM_COLUMNS };
};

QVariant MetadataModel::data(const QModelIndex& index, int role) const {
    int row = index.row();
    if (row < 0 || rowCount() <= row)
        return {};
    int col = index.column();
    switch (role) {
    case Qt::CheckStateRole:
        if (col==COL_CHECK)
            return activated(row) ? Qt::Checked : Qt::Unchecked;
        break;
    case Qt::DisplayRole:
        switch (col) {
        case COL_TAG:
            return Metadata::attributeTag(row, false);
        case COL_VALUE:
            const Cluster* highlight = gSession->dataset().highlight().cluster();
            if (!highlight)
                return "-";
            return highlight->avgeMetadata()->attributeStrValue(row);
        }
        break;
    }
    return {};
}


// ************************************************************************** //
//  local class MetadataView
// ************************************************************************** //

//! Main item in SubframeMetadata: View and control the list of Metadata.

class MetadataView : public CheckTableView {
public:
    MetadataView();

private:
    void currentChanged(const QModelIndex& current, const QModelIndex&) override final {
        gotoCurrent(current); }
    int sizeHintForColumn(int) const final;
    MetadataModel* model() { return static_cast<MetadataModel*>(model_); }
};

MetadataView::MetadataView()
    : CheckTableView(new MetadataModel())
{
    connect(gSession, &Session::sigClusters, this, &TableView::onData);
    connect(gSession, &Session::sigMetaSelection, this, &TableView::onHighlight);
    connect(this, &MetadataView::clicked, model(), &CheckTableModel::onClicked);
}

int MetadataView::sizeHintForColumn(int col) const {
    switch (col) {
    case MetadataModel::COL_CHECK:
        return 2*mWidth();
    default:
        return 3*mWidth();
    }
}

// ************************************************************************** //
//  class SubframeMetadata
// ************************************************************************** //

SubframeMetadata::SubframeMetadata() : DockWidget("Metadata", "dock-metadata") {
    box_.addWidget((metadataView_ = new MetadataView()));
}
