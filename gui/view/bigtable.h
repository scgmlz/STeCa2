//  ***********************************************************************************************
//
//  Steca: stress and texture calculator
//
//! @file      gui/view/bigtable.h
//! @brief     Defines classes DataView
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
//  ***********************************************************************************************

#ifndef BIGTABLE_H
#define BIGTABLE_H

#include "core/typ/variant.h"
#include "gui/base/model_view.h"
#include <memory>

//! Model for the DataView view.

class DataModel : public TableModel {
public:
    DataModel();
    DataModel(DataModel&) = delete;

    void refresh();
    void onColumnMove(int from, int to);
    void setSortColumn(int);
    void sortData();
    void setHighlight(int i) final { ; }        // unused

    QVariant data(const QModelIndex&, int) const;
    QVariant headerData(int, Qt::Orientation, int) const;
    int columnCount() const final { return numCols_ + 1; }
    int rowCount() const final { return rows_.count(); }
    const QVector<QVariant>& row(int) const;
    QStringList getHeaders() const;
    QVector<QVector<const QVariant*>> getData() const;
    int highlighted() const final { return 0; } // unused

private:
    QStringList headers_;
    QVector<int> colIndexMap_;
    QVector<VariantComparator*> comparators_;

    // TODO: Treat the row number column as any other column, so that it can also be exported.
    // Then replace XRow by QVector<QVariant>.
    struct XRow {
        XRow() {} // see https://bugreports.qt.io/browse/QTBUG-67357
        XRow(int n_, const QVector<QVariant>& row_) : n(n_), row(row_) {}
        int n; // number of row = contents of first columns
        QVector<QVariant> row;
    };

    int numCols_;
    int sortColumn_ {-1};
    QVector<XRow> rows_;
};

//! A data table view, for use in the 'Points' tab of an output Frame.

class DataView : public TableView {
public:
    DataView();
    DataView(DataView&) = delete;

    void refresh();

private:
    void updateShownColumns();
    void keyPressEvent(QKeyEvent *event);

    DataModel* model() const { return dynamic_cast<DataModel*>(model_); }
    QString exportSelection() const;
};

#endif // BIGTABLE_H