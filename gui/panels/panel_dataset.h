// ************************************************************************** //
//
//  STeCa2:    StressTextureCalculator ver. 2
//
//! @file      panel_dataset.h
//! @brief     File selection panel.
//!
//! @homepage  http://apps.jcns.fz-juelich.de/steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016
//! @authors   Scientific Computing Group at MLZ Garching
//! @authors   Rebecca Brydon, Jan Burle,  Antti Soininen
//! @authors   Based on the original STeCa by Christian Randau
//
// ************************************************************************** //

#ifndef PANEL_DATASET_H
#define PANEL_DATASET_H

#include "data/data_dataset.h"
#include "calc/calc_lens.h"
#include "panel.h"
#include "views.h"

namespace gui { namespace panel {
//------------------------------------------------------------------------------

class DockDatasets : public DockWidget, protected RefHub {
  CLS(DockDatasets) SUPER(DockWidget)
public:
  DockDatasets(TheHub&);

  QSpinBox *combineDatasets_;

private:
  class DatasetView *datasetView_;
};

//------------------------------------------------------------------------------

class DockDatasetInfo : public DockWidget, protected RefHub {
  CLS(DockDatasetInfo) SUPER(DockWidget)
public:
  DockDatasetInfo(TheHub&);

private:
  class Info : public QWidget {
  public:
    Info(models::checkedinfo_vec&);

  private:
    QGridLayout *grid_;
  };

  Info *info_;
  models::checkedinfo_vec metaInfo_;
};

//------------------------------------------------------------------------------

class Dataset;

class ImageWidget : public QWidget, protected RefHub {
  CLS(ImageWidget) SUPER(QWidget)
public:
  ImageWidget(TheHub&, Dataset&);

  void setPixmap(QPixmap const&);
  void setShowOverlay(bool);
  void setScale(uint);

  QSize sizeHint() const;

protected:
  Dataset &dataset_;
  bool     showOverlay_;
  QPixmap  original_, scaled_;
  uint     scale_;

  void paintEvent(QPaintEvent*);
};

//------------------------------------------------------------------------------

class DatasetOptions1 : public BoxPanel {
  Q_OBJECT
  CLS(DatasetOptions1) SUPER(BoxPanel)
public:
  DatasetOptions1(TheHub&);

private:
  QSpinBox       *spinOffsetI_, *spinOffsetJ_;
  QDoubleSpinBox *spinDistance_, *spinPixelSize_;

  // REVIEW
  void setTo(TheHub&);
  void setFrom(TheHub&);
};

class DatasetOptions2 : public BoxPanel {
  Q_OBJECT
  CLS(DatasetOptions2) SUPER(BoxPanel)
public:
  DatasetOptions2(TheHub&);

signals:
  void imageScale(uint);

private:
  QSpinBox  *marginLeft_, *marginTop_, *marginRight_, *marginBottom_;
  QSpinBox  *spinImageScale_;
  QComboBox *comboNormType_;

  void setFrom(TheHub&);
};

//------------------------------------------------------------------------------

// RENAME gui::panel::Dataset -> ???
class Dataset : public TabsPanel {
  CLS(Dataset) SUPER(TabsPanel)
public:
  Dataset(TheHub&);

  void setImageScale(uint);

private:
  QPixmap makePixmap(calc::shp_ImageLens);
  void    setDataset(data::shp_Dataset);
  void    render();

  data::shp_Dataset dataset_;
  ImageWidget *dataImageWidget_, *corrImageWidget_;
};

//------------------------------------------------------------------------------
}}
#endif // PANEL_DATASET_H
