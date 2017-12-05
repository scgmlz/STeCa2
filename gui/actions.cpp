// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      gui/actions.cpp
//! @brief     Implements ...
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //


#include "actions.h"
#include "app.h"
#include "thehub.h"

namespace gui {
//------------------------------------------------------------------------------

Action::Action(rcstr text, QObject* parent): super(text,parent) {
  setToolTip(text.toLower());
}

Action& Action::text(rcstr text) {
  setText(text);
  tip(text);
  return *this;
}

Action& Action::tip(rcstr tip) {
  setToolTip(tip.toLower());
  return *this;
}

Action& Action::key(QKeySequence key) {
  setShortcut(key);
  return *this;
}

Action& Action::icon(rcstr iconFile) {
  setIcon(QIcon(iconFile));
  return *this;
}

Action& Action::alt(rcstr /*text2*/) {
  return *this;
}

//------------------------------------------------------------------------------

TriggerAction::TriggerAction(rcstr text, QObject* parent)
: super(text,parent)
{
}

//------------------------------------------------------------------------------

ToggleAction::ToggleAction(rcstr text, QObject* parent)
: super(text,parent), text1_(text)
{
  setCheckable(true);
}

Action& ToggleAction::alt(rcstr text2) {
  text2_ = text2;
  connect(this,&Cls::toggled,[this](bool on) {
    rcstr text = on ? text2_ : text1_;
    setText(text);
    setToolTip(text);
  });

  return super::alt(text2);
}

//------------------------------------------------------------------------------

Actions::Actions(TheHub& hub): super(hub) {
  using QKey = QKeySequence;

  // create actions

  trg(about,                "About && Configuration...");
  trg(online,               "Online documentation...");
  trg(checkUpdate,          "Check for update...");
  trg(quit,                 "Quit");

  tgl(viewStatusbar,        "Statusbar");
  tgl(viewFiles,            "Files");
  tgl(viewDatasets,         "Datasets");
  tgl(viewDatasetInfo,      "Metadata");
  trg(viewReset,            "Reset");
#ifndef Q_OS_OSX
  tgl(fullScreen,           "FullScreen");
#endif

  trg(loadSession,          "Load session...");
  trg(saveSession,          "Save session...");
  trg(clearSession,         "Clear session (to defaults)");

  trg(addFiles,             "Add files...")                 .icon(":/icon/add");
  trg(remFile,              "Remove selected file(s)")      .icon(":/icon/rem");
  tgl(enableCorr,           "Enable correction file...")    .icon(":/icon/useCorrection");
  trg(remCorr,              "Remove correction file")       .icon(":/icon/clear");

  trg(rotateImage,          "Rotate")                       .icon(":/icon/rotate0");
  tgl(mirrorImage,          "Mirror")                       .icon(":/icon/mirrorHorz");
  tgl(linkCuts,             "Link cuts")                    .icon(":/icon/link");
  tgl(showOverlay,          "Show overlay")                 .icon(":/icon/crop");
  tgl(stepScale,            "Scale in steps")               .icon(":/icon/steps");
  tgl(showBins,             "Show bins")                    .icon(":/icon/angle");

  tgl(fixedIntenImage,      "Global intensity scale")       .icon(":/icon/scale");
  tgl(fixedIntenDgram,      "Fixed intensity scale");

  tgl(combinedDgram,        "All datasets");

  tgl(selRegions,           "Select regions")               .icon(":/icon/selRegion");
  tgl(showBackground,       "Show fitted background")       .icon(":/icon/showBackground");
  trg(clearBackground,      "Clear background regions")     .icon(":/icon/clear");
  trg(clearReflections,     "Clear reflections")            .icon(":/icon/clear");

  trg(addReflection,        "Add reflection")               .icon(":/icon/add");
  trg(remReflection,        "Remove reflection")            .icon(":/icon/rem");

  trg(outputPolefigures,    "Pole figures...");
  trg(outputDiagrams,       "Diagrams...");
  trg(outputDiffractograms, "Diffractograms...");

  // key shortcuts

  quit            -> key(QKey::Quit);

  viewStatusbar   -> key(Qt::Key_F12);
  viewFiles       -> key(Qt::Key_F8);
  viewDatasets    -> key(Qt::Key_F9);
  viewDatasetInfo -> key(Qt::Key_F10);

#ifndef Q_OS_OSX
  fullScreen      -> key(Qt::Key_F11);
#endif

  addFiles        -> key(Qt::CTRL|Qt::Key_O);
  remFile         -> key(QKey::Delete);
  enableCorr      -> key(Qt::SHIFT|Qt::CTRL|Qt::Key_C);

  rotateImage     -> key(Qt::CTRL|Qt::Key_R);

  // handle signals

  onSigFilesSelected([this]() {
    remFile->setEnabled(!hub_.collectedFromFiles().isEmpty());
  });

  onSigCorrFile([this](data::shp_File file) {
    remCorr->setEnabled(!file.isNull());
  });

  onSigCorrEnabled([this](bool on) {
    enableCorr->setChecked(on);
  });

  auto deselect = [this]() {
    fixedIntenImage->setChecked(false);
    fixedIntenDgram->setChecked(false);
    combinedDgram->setChecked(false);
  };

  onSigGeometryChanged([deselect]() {
    deselect();
  });

  onSigDatasetsChanged([deselect]() {
    deselect();
  });

  onSigCorrEnabled([deselect]() {
    deselect();
  });
}

Action& Actions::trg(Action* &action, rcstr text) {
  return *(action = new TriggerAction(text,&hub_));
}

Action& Actions::tgl(Action* &action, rcstr text) {
  return *(action = new ToggleAction(text,&hub_));
}

//------------------------------------------------------------------------------
}
// eof
