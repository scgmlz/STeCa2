// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/capture_and_replay/console.h
//! @brief     Declares class Console
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef CONSOLE_H
#define CONSOLE_H

#include "core/typ/singleton.h"
#include "gui/capture_and_replay/enhance_widgets.h"
#include <stack>
#include <QTextStream>

extern class Console* gConsole; //!< global

//! Reads commands from stdin, and emits signal transmitLine
class Console : public QObject, public ISingleton<Console>
{
    Q_OBJECT
public:
    Console();
    ~Console();
    void learn(const QString& name, CSettable*);
    void forget(const QString& name);
    void readFile(const QString& fName);
    void call(const QString&);
    bool hasCommandsOnStack() const { return !commandLifo_.empty(); }
    void commandsFromStack();
    void log(const QString&);
    void log2(bool, const QString&);
private:
    class CommandRegistry& registry() { return *registryStack_.top(); }
    QTextStream log_;
    enum class Caller { gui, cli, stack, sys } caller_ { Caller::gui };
    enum class Result : int { ok, err, suspend };
    Result exec(QString);
    class QSocketNotifier *notifier_;
    std::stack<class CommandRegistry*> registryStack_;
    std::deque<QString> commandLifo_;
private slots:
    void readLine();
};

#endif // CONSOLE_H
