// ************************************************************************** //
//
//  STeCa2:    StressTexCalculator ver. 2
//
//! @file      core_async.cpp
//!
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016
//! @authors   Scientific Computing Group at MLZ Garching
//! @authors   Original version: Christian Randau
//! @authors   Version 2: Antti Soininen, Jan Burle, Rebecca Brydon
//
// ************************************************************************** //

#include "core_async.h"
#include <QtWidgets/QProgressBar>

//------------------------------------------------------------------------------

TakesLongTime::TakesLongTime() {
  if (handler) handler(true);
}

TakesLongTime::~TakesLongTime() {
  if (handler) handler(false);
}

void (*TakesLongTime::handler)(bool) = nullptr;

//------------------------------------------------------------------------------

Progress::Progress(uint total) : total_(total), i_(0) {
  if (bar) {
    bar->setRange(0, total_);
    bar->setValue(i_);
    bar->show();
  }
}

Progress::~Progress() {
  if (bar) bar->hide();
}

void Progress::setProgress(uint i) {
  if (bar) bar->setValue((i_ = qBound(0u, i, total_)));
}

void Progress::step() {
  setProgress(i_ + 1);
}

QProgressBar* Progress::bar;

// eof
