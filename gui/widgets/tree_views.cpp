// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      gui/widgets/tree_views.cpp
//! @brief     Implements classes ListView, MultiListView
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "tree_views.h"
#include "def/idiomatic_for.h"
#include "models.h"

namespace gui {

// ************************************************************************** //
//  class TreeView
// ************************************************************************** //

TreeView::TreeView() {
    setAlternatingRowColors(true);
}

int TreeView::sizeHintForColumn(int) const {
    return 3 * fontMetrics().width('m');
}

// ************************************************************************** //
//  auxiliary class TreeListView
// ************************************************************************** //

AuxView::AuxView() {
    setSelectionBehavior(SelectRows);
}

void AuxView::setModel(QAbstractItemModel* model) {
    TreeView::setModel(model);
    hideColumn(0); // this should look like a list; 0th column is tree-like

    if (model) {
        connect(model, &QAbstractItemModel::modelReset, [this, model]() {
            for_i (model->columnCount())
                resizeColumnToContents(i);
        });
    }
}

// ************************************************************************** //
//  class ListView
// ************************************************************************** //

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
void ListView::setModel(TableModel* model) {
    AuxView::setModel(model);
}
#pragma GCC diagnostic pop

TableModel* ListView::model() const {
    return static_cast<TableModel*>(AuxView::model());
}

void ListView::updateSingleSelection() {
    int row = currentIndex().row();
    model()->signalReset();
    selectRow(row);
}

void ListView::selectRow(int row) {
    setCurrentIndex(model()->index(row, 0));
}

// ************************************************************************** //
//  class MultiListView
// ************************************************************************** //

MultiListView::MultiListView() : ListView() {
    setSelectionMode(ExtendedSelection);
}

void MultiListView::selectRows(uint_vec rows) {
    auto m = model();
    int cols = m->columnCount();

    QItemSelection is;
    for (uint row : rows)
        is.append(QItemSelectionRange(m->index(to_i(row), 0), m->index(to_i(row), cols - 1)));

    selectionModel()->select(is, QItemSelectionModel::ClearAndSelect);
}

} // namespace gui
