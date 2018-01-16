// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/base/tree_views.h
//! @brief     Defines classes TreeView, ListView, MultiListView
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef TREE_VIEWS_H
#define TREE_VIEWS_H

#include "core/typ/vec.h"
#include <QTreeView>

//! Abstract tree widget, base class of ListView and Table

class TreeView : public QTreeView {
public:
    TreeView();
protected:
    int sizeHintForColumn(int) const; //!< make narrow columns
};

//! A (tree-)list view. Single selection.

class ListView : public TreeView {
public:
    ListView() {}
    void setModel(class TableModel* model);
protected:
    virtual class TableModel* model() const;
    void updateSingleSelection();
    void selectRow(int);
};

//! A (tree-)list view. Multiple selection.

class MultiListView : public ListView {
public:
    MultiListView();
protected:
    void selectRows(vec<int>);
};

#endif // TREE_VIEWS_H
