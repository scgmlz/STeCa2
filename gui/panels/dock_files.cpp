// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      gui/panels/dock_files.cpp
//! @brief     Implements class DockFiles; defines and implements class FileViews
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "dock_files.h"
#include "models.h"
#include "session.h"
#include "thehub.h"
#include "widgets/tree_views.h" // inheriting from
#include "widgets/widget_makers.h"
#include <QAction>
#include <QHeaderView>

namespace gui {
namespace panel {

// ************************************************************************** //
//  class FilesView (definition)
// ************************************************************************** //

class FilesView : public MultiListView {
public:
    FilesView();

protected:
    FilesModel* model() const { return static_cast<FilesModel*>(MultiListView::model()); }

    void selectionChanged(QItemSelection const&, QItemSelection const&);
    void removeSelected();
    void recollect();
};

// ************************************************************************** //
//  class FilesView (implementation)
// ************************************************************************** //

FilesView::FilesView() : MultiListView() {
    setModel(gHub->filesModel);
    debug::ensure(dynamic_cast<FilesModel*>(MultiListView::model()));

    header()->hide();

    connect(gHub->trigger_removeFile, &QAction::triggered, [this]() { removeSelected(); });

    connect(gHub, &TheHubSignallingBase::sigFilesChanged,
            [this]() { selectRows({}); recollect(); });

    connect(gHub, &TheHubSignallingBase::sigFilesSelected,
            [this]() { selectRows(gSession->collectedFromFiles()); });
}

void FilesView::selectionChanged(QItemSelection const& selected, QItemSelection const& deselected) {
    MultiListView::selectionChanged(selected, deselected);
    recollect();
}

void FilesView::removeSelected() {
    auto indexes = selectedIndexes();

    // backwards
    for (int i = indexes.count(); i-- > 0;)
        model()->removeFile(to_u(indexes.at(i).row()));

    selectRows({});
    recollect();
}

void FilesView::recollect() {
    uint_vec rows;
    for (auto& index : selectionModel()->selectedRows())
        if (index.isValid())
            rows.append(to_u(index.row()));

    gHub->collectDatasetsFromFiles(rows);
}

// ************************************************************************** //
//  class DocFiles
// ************************************************************************** //

DockFiles::DockFiles() : DockWidget("Files", "dock-files", Qt::Vertical) {

    auto h = hbox();
    box_->addLayout(h);

    h->addStretch();
    h->addWidget(iconButton(gHub->trigger_addFiles));
    h->addWidget(iconButton(gHub->trigger_removeFile));

    box_->addWidget((filesView_ = new FilesView()));

    h = hbox();
    box_->addLayout(h);

    h->addWidget(label("Correction file"));

    h = hbox();
    box_->addLayout(h);

    h->addWidget((corrFile_ = new LineView()));
    h->addWidget(iconButton(gHub->toggle_enableCorr));
    h->addWidget(iconButton(gHub->trigger_remCorr));

    connect(gHub, &TheHubSignallingBase::sigCorrFile,
            [this](QSharedPointer<Datafile const> file) {
                corrFile_->setText(file.isNull() ? "" : file->fileName()); });
}

} // namespace panel
} // namespace gui
