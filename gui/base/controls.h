// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/base/controls.h
//! @brief     Defines functions that return new Qt objects
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef CONTROLS_H
#define CONTROLS_H

#include "core/def/macros.h"
#include "gui/capture_and_replay/enhance_widgets.h"
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QToolButton>


//! A trigger, for use in buttons or menu entries, that can also be activated by console command.
class CTrigger : public QAction, private CSettable {
public:
    CTrigger() = delete;
    CTrigger(const QString& name, const QString& text, const QString& iconFile="");
    CTrigger(const QString& name, const QString& text, const QString& iconFile,
             const QKeySequence& shortcut);
    void onCommand(const QStringList&) THROWS;
private:
    QString tooltip_;
};

//! A Toggle, for use in buttons or menu entries, that can also be switched by console command.
class CToggle : public QAction, private CSettable {
public:
    CToggle() = delete;
    CToggle(const QString& name, const QString& text, bool on, const QString& iconFile="");
    CToggle(const QString& name, const QString& text, bool on, const QString& iconFile,
            const QKeySequence& shortcut);
    void onCommand(const QStringList&) THROWS;
private:
    QString tooltip_;
};

//! QToolButton with text display and associated QAction.
class XTextButton : public QToolButton {
public:
    XTextButton(QAction*);
};

//! QToolButton with icon and associated QAction.
class XIconButton : public QToolButton {
public:
    XIconButton(QAction*);
};

//! Named QSpinBox that can be set by console command.
class CSpinBox : public QSpinBox, private CSettable {
public:
    CSpinBox(const QString& name, int ndigits, bool withDot, int min = INT_MIN, int max = INT_MAX,
             const QString& tooltip="");
    void onCommand(const QStringList&) THROWS;
};

//! Named QDoubleSpinBox that can be set by console command.
class CDoubleSpinBox : public QDoubleSpinBox, private CSettable {
public:
    CDoubleSpinBox(const QString& name, int ndigits, qreal min = LLONG_MIN, qreal max = LLONG_MAX);
    void onCommand(const QStringList&) THROWS;
};

//! Named QCheckBox that can be set by console command.
class CCheckBox : public QCheckBox, private CSettable {
public:
    CCheckBox(const QString& name, QAction*);
    CCheckBox(const QString& name, const QString& text);
    void onCommand(const QStringList&) THROWS;
};

//! Named QRadioButton that can be set by console command.
class CRadioButton : public QRadioButton, private CSettable {
public:
    CRadioButton(const QString& name, const QString& text);
    void onCommand(const QStringList&) THROWS;
};

//! Named QComboBox that can be set by console command.
class CComboBox : public QComboBox, private CSettable {
public:
    CComboBox(const QString& name, const QStringList& items = {});
    void onCommand(const QStringList&) THROWS;
};

//! QFileDialog, for modal use, with console commands to select files and to close the dialog.
class CFileDialog : public QFileDialog, private CModal, CSettable {
public:
    CFileDialog(QWidget *parent = Q_NULLPTR, const QString &caption = QString(),
                const QString &directory = QString(), const QString &filter = QString());
    ~CFileDialog();
    int exec() override;
    void onCommand(const QStringList&) THROWS;
};

#endif // CONTROLS_H
