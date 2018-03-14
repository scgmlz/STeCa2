// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/cfg/msg_handler.h
//! @brief     Defines messageHandler.
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef MSG_HANDLER_H
#define MSG_HANDLER_H

#include <QString>

void messageHandler(QtMsgType type, const QMessageLogContext& ctx, const QString& msg);

#endif // MSG_HANDLER_H
