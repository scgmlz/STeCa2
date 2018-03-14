// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/base/controls.cpp
//! @brief     Implements functions that return new Qt objects
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "gui/base/controls.h"
#include "gui/base/displays.h"
#include "gui/capture_and_replay/console.h"
#include "gui/mainwin.h" // for _SLOT_
#include <QApplication> // for qApp for new Action
#include <QtGlobal> // to define Q_OS_WIN

// ************************************************************************** //
//  class Trigger
// ************************************************************************** //

CTrigger::CTrigger(const QString& name, const QString& text, const QString& iconFile)
    : QAction(text, qApp)
    , CSettable(name)
    , tooltip_(text.toLower())
{
    if (iconFile!="")
        setIcon(QIcon(iconFile));
    QObject::connect(this, &QAction::triggered, [name]()->void {
            gConsole->log(name+" trigger"); });
    QObject::connect(this, &QAction::changed, [this, name]()->void {
            QString txt = tooltip_;
            if (!isEnabled())
                txt += "\nThis trigger is currently inoperative.";
            setToolTip(txt); });
    emit changed();
};

CTrigger::CTrigger(
    const QString& name, const QString& text, const QString& iconFile, const QKeySequence& shortcut)
    : CTrigger(name, text, iconFile)
{
    setShortcut(shortcut);
}

void CTrigger::onCommand(const QStringList& args)
{
    if (args[0]!="trigger")
        THROW("Unexpected command");
    trigger();
}

// ************************************************************************** //
//  class Toggle
// ************************************************************************** //

CToggle::CToggle(const QString& name, const QString& text, bool on, const QString& iconFile)
    : QAction(text, qApp)
    , CSettable(name)
    , tooltip_(text.toLower())
{
    if (iconFile!="")
        setIcon(QIcon(iconFile));
    setCheckable(true);
    setChecked(on);
    QObject::connect(this, &QAction::toggled, [name](bool val)->void {
            gConsole->log(name+" switch "+(val ? "on" : "off")); });
    QObject::connect(this, &QAction::changed, [this, name]()->void {
            QString txt = tooltip_;
            if (!isEnabled())
                txt += "\nThis toggle is currently inoperative.";
            else if (isChecked())
                txt += "\nThis toggle is currently checked. Click to uncheck.";
            else
                txt += "\nThis toggle is currently unchecked. Click to check.";
            setToolTip(txt); });
    emit changed();
};

CToggle::CToggle(const QString& name, const QString& text, bool on, const QString& iconFile,
                 const QKeySequence& shortcut)
    : CToggle(name, text, on, iconFile)
{
    setShortcut(shortcut);
}

void CToggle::onCommand(const QStringList& args)
{
    if (args[0]!="switch")
        THROW("Unexpected command");
    if      (args.size()<2)
        THROW("Missing argument to command 'switch'");
    else if (args[1]=="on")
        setChecked(true);
    else if (args[1]=="off")
        setChecked(false);
    else
        THROW("Invalid argument to command 'switch'");
}

// ************************************************************************** //
//  classes with no console connection
// ************************************************************************** //

XTextButton::XTextButton(QAction* action) {
    setDefaultAction(action);
    setToolButtonStyle(Qt::ToolButtonTextOnly);
}

XIconButton::XIconButton(QAction* action) {
    setDefaultAction(action);
    setToolButtonStyle(Qt::ToolButtonIconOnly);
}

// ************************************************************************** //
//  control widget classes with console connection
// ************************************************************************** //

// A QSpinBox controls an integer value. Therefore normally we need no extra width for a dot.
// However, sometimes we want to make a QSpinBox exactly as wide as a given QDoubleSpinBox,
// for nice vertical alignement. Then we use withDot=true.
CSpinBox::CSpinBox(const QString& _name, int ndigits, bool withDot, int min, int max,
                   const QString& tooltip)
    : CSettable(_name)
{
    widgetUtils::setWidth(this, 2+ndigits, withDot);
    setMinimum(min);
    setMaximum(max > min ? max : min);
    if (tooltip!="")
        setToolTip(tooltip);
    connect(this, _SLOT_(QSpinBox, valueChanged, int), [this](int val)->void {
            gConsole->log2(hasFocus(), name()+" set "+QString::number(val)); });
}

void CSpinBox::onCommand(const QStringList& args) {
    if (args[0]!="set")
        THROW("Unexpected command");
    if      (args.size()<2)
        THROW("Missing argument to command 'set'");
    setValue(TO_INT(args[1]));
}

CDoubleSpinBox::CDoubleSpinBox(const QString& _name, int ndigits, qreal min, qreal max)
    : CSettable(_name)
{
    widgetUtils::setWidth(this, 2+ndigits, true);
    ASSERT(min<=max);
    setMinimum(min);
    setMaximum(max);
    connect(this, _SLOT_(QDoubleSpinBox, valueChanged, double), [this](double val)->void {
            gConsole->log2(hasFocus(), name()+" set "+QString::number(val)); });
}

void CDoubleSpinBox::onCommand(const QStringList& args) {
    if (args[0]!="set")
        THROW("Unexpected command");
    if      (args.size()<2)
        THROW("Missing argument to command 'set'");
    setValue(TO_DOUBLE(args[1]));
}

CCheckBox::CCheckBox(const QString& _name, QAction* action)
    : QCheckBox(action ? action->text().toLower() : "")
    , CSettable(_name)
{
    if (!action)
        return;
    connect(this, &QCheckBox::toggled, [action](bool on) { action->setChecked(on); });
    connect(action, &QAction::toggled, [this](bool on) { setChecked(on); });
    setToolTip(action->toolTip());
    setChecked(action->isChecked());
    connect(this, _SLOT_(QCheckBox, stateChanged, int), [this](int val)->void {
            gConsole->log2(hasFocus(), name()+" set "+QString::number(val)); });
}

CCheckBox::CCheckBox(const QString& name, const QString& text)
    : CCheckBox(name, {})
{
    setText(text);
}

void CCheckBox::onCommand(const QStringList& args) {
    if (args[0]!="set")
        THROW("Unexpected command");
    if      (args.size()<2)
        THROW("Missing argument to command 'set'");
    setChecked(TO_INT(args[1]));
}

CRadioButton::CRadioButton(const QString& _name, const QString& text)
    : QRadioButton(text)
    , CSettable(_name)
{
    connect(this, _SLOT_(QRadioButton, toggled, bool), [this](bool val)->void {
            gConsole->log2(hasFocus(), name()+" switch "+(val?"on":"off")); });
}

void CRadioButton::onCommand(const QStringList& args) {
    if (args[0]!="switch")
        THROW("Unexpected command");
    if      (args.size()<2)
        THROW("Missing argument to command 'switch'");
    else if (args[1]=="on")
        setChecked(true);
    else if (args[1]=="off")
        setChecked(false);
    else
        THROW("Invalid argument to command 'switch'");
}

CComboBox::CComboBox(const QString& _name, const QStringList& items)
    : CSettable(_name)
{
    addItems(items);
    connect(this, _SLOT_(QComboBox, currentIndexChanged, int), [this](int val)->void {
            gConsole->log2(hasFocus(), name()+" choose "+QString::number(val)); });
}

void CComboBox::onCommand(const QStringList& args) {
    if (args[0]!=" choose ")
        THROW("Unexpected command");
    if      (args.size()<2)
        THROW("Missing argument to command 'choose'");
    setCurrentIndex(TO_INT(args[1]));
}

// ************************************************************************** //
//  class CFileDialog
// ************************************************************************** //

CFileDialog::CFileDialog(QWidget *parent, const QString &caption,
                         const QString &directory, const QString &filter)
    : QFileDialog(parent, caption, directory, filter)
    , CModal("fdia")
    , CSettable("fdia")
{
}

CFileDialog::~CFileDialog() {
    gConsole->log(" select "+selectedFiles().join(';'));
}

int CFileDialog::exec() {
    if (gConsole->hasCommandsOnStack()) {
        open();
        gConsole->commandsFromStack();
        close();
        return QDialog::Accepted;
    } else
        return QFileDialog::exec();
}

void CFileDialog::onCommand(const QStringList& args) {
    if        (args[0]=="close") {
        accept();
    } else if (args[0]=="select") {
        if (args.size()<2)
            THROW("Missing argument to command 'select'");
        QStringList list = args[1].split(';');
        QString tmp = '"' + list.join("\" \"") + '"';
        selectFile(tmp);
    } else
        THROW("Unexpected command");
}
