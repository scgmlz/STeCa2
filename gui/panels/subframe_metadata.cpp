//  ***********************************************************************************************
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
//  ***********************************************************************************************

#include "gui/panels/subframe_metadata.h"
#include "core/session.h"
#include "qcr/widgets/tables.h"

//  ***********************************************************************************************
//! @class MetatableModel (local scope)

//! The model for MetadatView.

class MetatableModel : public CheckTableModel {
public:
    MetatableModel() : CheckTableModel{"meta"} {}

    enum { COL_CHECK = 1, COL_TAG, COL_VALUE, NUM_COLUMNS };

private:
    int highlighted() const final { return highlighted_; }
    void onHighlight(int i) final { highlighted_ = i; }
    bool activated(int row) const { return gSession->params.smallMetaSelection.isSelected(row); }
    void setActivated(int row, bool on) { gSession->params.smallMetaSelection.set(row, on); }

    int columnCount() const final { return NUM_COLUMNS; }
    int rowCount() const final { return meta::numAttributes(true); }

    QVariant entry(int, int) const final;
    QVariant headerData(int, Qt::Orientation, int) const { return {}; }
    QColor foregroundColor(int, int) const final;
    void onClicked(const QModelIndex& cell) final;
    QVariant data(const QModelIndex &index, int role) const final;

    int highlighted_ {0};
};

QVariant MetatableModel::entry(int row, int col) const
{
    switch (col) {
    case COL_TAG:
        return meta::niceTag(row);
    case COL_VALUE:
        const Cluster* highlight = gSession->currentCluster();
        if (!highlight)
            return "-";
        return highlight->avgMetadata().attributeStrValue(row);
    }
    return "";
}

QColor MetatableModel::foregroundColor(int row, int col) const
{
    switch (meta::getMetaMode(row)) {
    case metaMode::CONSTANT:
        return QColor(Qt::black);
    case metaMode::FILE_DEPENDENT:
        return QColor(Qt::darkMagenta);
    case metaMode::MEASUREMENT_DEPENDENT:
        return QColor(Qt::darkBlue);
    }
}

void MetatableModel::onClicked(const QModelIndex& cell)
{
    TableModel::setHighlightedCell(cell);
    int row = cell.row();
    int col = cell.column();
    if (col==1 && meta::getMetaMode(row) != metaMode::CONSTANT) {
        activateAndLog(row, !activated(row));
        gRoot->remakeAll();
    }
}

QVariant MetatableModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    int col = index.column();
    if (col < 0 || col >= columnCount() || row < 0 || row >= rowCount())
        return {};
    if (role != Qt::CheckStateRole)
        return CheckTableModel::data(index, role);
    else {
        if (col==1 && meta::getMetaMode(row)!=metaMode::CONSTANT)
            return state(row);
        else
            return {};
    }
}

//  ***********************************************************************************************
//! @class MetatableView (local scope)

//! Main item in SubframeMetadata: View and control the list of Metadata.

class MetatableView : public CheckTableView {
public:
    MetatableView();
private:
    MetatableModel* model() { return static_cast<MetatableModel*>(model_); }
};

MetatableView::MetatableView()
    : CheckTableView{new MetatableModel}
{
    setColumnWidth(0, 0);
    setColumnWidth(1,  .5*mWidth());
    setColumnWidth(2, 6. *mWidth());
    setColumnWidth(3, 7.5*mWidth());
}

//  ***********************************************************************************************
//! @class SubframeMetadata

SubframeMetadata::SubframeMetadata()
    : QcrDockWidget{"metadata"}
{
    for (int i=0; i<meta::size(); ++i)
        gSession->params.smallMetaSelection.vec.push_back({false});
    setFeatures(DockWidgetMovable);
    setWindowTitle("Metadata");
    setWidget(new MetatableView);
    setRemake([this](){setEnabled(gSession->hasData());});
}
