// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/base/filedialog.cpp
//! @brief     Implements functions openFileName(s), saveFileName, saveDirName in ns file_dialog
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "core/loaders/loaders.h"
#include "gui/base/controls.h"
#include <QFileSystemModel>
#include <QMessageBox>
#include <QSortFilterProxyModel>

namespace {

typedef const QModelIndex& rcidx;

// ************************************************************************** //
//  class OpenFileProxyModel (file scope)
// ************************************************************************** //

class OpenFileProxyModel : public QSortFilterProxyModel {
public:
    int columnCount(rcidx) const;
    QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const;
    QVariant data(rcidx, int = Qt::DisplayRole) const;

private:
    mutable QHash<QString, QString> memInfo;
};

int OpenFileProxyModel::columnCount(rcidx) const {
    return 2;
}

QVariant OpenFileProxyModel::headerData(int section, Qt::Orientation ori, int role) const {
    if (1 == section && Qt::Horizontal == ori && role == Qt::DisplayRole)
        return "Comment";
    return QSortFilterProxyModel::headerData(section, ori, role);
}

QVariant OpenFileProxyModel::data(rcidx idx, int role) const {
    if (idx.isValid() && 1 == idx.column()) {
        if (Qt::DisplayRole == role) {
            QFileSystemModel* fileModel = qobject_cast<QFileSystemModel*>(sourceModel());
            QModelIndex ix0 =
                fileModel->index(mapToSource(idx).row(), 0, mapToSource(idx.parent()));
            QFileInfo info(fileModel->rootDirectory().filePath(fileModel->fileName(ix0)));
            if (info.isFile()) {
                const QString& path = info.absoluteFilePath();
                auto it = memInfo.find(path);
                if (it != memInfo.end())
                    return *it;
                QString loadInfo = load::loadComment(info);
                memInfo.insert(path, loadInfo);
                return loadInfo;
            }
        }
        return QVariant();
    }
    return QSortFilterProxyModel::data(idx, role);
}

} // local methods


// ************************************************************************** //
//  exported functions
// ************************************************************************** //

namespace file_dialog {

//! Base class for all Steca file dialogs. Enhances CFileDialog by a few settings.
class FileDialog : public CFileDialog {
public:
    FileDialog(QWidget*, const QString&, QDir&, const QString& filter = QString());
    QStringList getFiles();
    QString getFile();
private:
    QDir& dir_;
};

FileDialog::FileDialog(QWidget* parent, const QString& caption, QDir& dir, const QString &filter)
    : CFileDialog(parent, caption, dir.absolutePath(), filter)
    , dir_(dir)
{
    setOption(QFileDialog::DontUseNativeDialog);
    setViewMode(QFileDialog::Detail);
    setConfirmOverwrite(false);
}

QStringList FileDialog::getFiles()
{
    QStringList ret = selectedFiles();
    if (!ret.isEmpty())
        dir_ = QFileInfo(ret.at(0)).absolutePath();
    return ret;
}

QString FileDialog::getFile()
{
    QStringList files = getFiles();
    if (files.isEmpty())
        return "";
    return files.first();
}

//! Opens file for writing; runs dialog before overwriting.
QFile* OutputFile(const QString& name, QWidget* parent, const QString& path, bool check_overwrite)
{
    QFile* ret = new QFile(path);
    if (check_overwrite && ret->exists() &&
        QMessageBox::question(parent, "File exists", "Overwrite "+path+" ?") != QMessageBox::Yes) {
        delete ret;
        return nullptr;
    }
    if (!ret->open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for writing: " << path;
        return nullptr;
    }
    return ret;
}

QStringList openFileNames(QWidget* parent, const QString& caption, QDir& dir,
                          const QString& filter, bool plural)
{
    FileDialog dlg(parent, caption, dir, filter);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setReadOnly(true);
    dlg.setProxyModel(new OpenFileProxyModel);
    dlg.setFileMode(plural ? QFileDialog::ExistingFiles : QFileDialog::ExistingFile);

    if (!dlg.exec())
        return {};
    return dlg.getFiles();
}

QString openFileName(QWidget* parent, const QString& caption, QDir& dir, const QString& filter)
{
    QStringList fileNames = openFileNames(parent, caption, dir, filter, false);
    if (fileNames.isEmpty())
        return "";
    return fileNames.first();
}

QString saveFileName(QWidget* parent, const QString& caption, QDir& dir, const QString& filter)
{
    FileDialog dlg(parent, caption, dir, filter);
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setAcceptMode(QFileDialog::AcceptSave);

    if (!dlg.exec())
        return "";
    return dlg.getFile();
}

QString saveDirName(QWidget* parent, const QString& caption, QDir& dir) {
    FileDialog dlg(parent, caption, dir);
    dlg.setFileMode(QFileDialog::Directory);
    dlg.setAcceptMode(QFileDialog::AcceptSave);

    if (!dlg.exec())
        return "";
    return dlg.getFile();
}

} // namespace file_dialog
