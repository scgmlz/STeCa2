// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/panels/subframe_diffractogram.cpp
//! @brief     Implements class SubframeDiffractogram
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "subframe_diffractogram.h"
#include "core/session.h"
#include "gui/panels/panel_diffractogram.h"
#include "gui/base/layout.h"
#include "gui/base/controls.h"
#include "gui/actions/toggles.h"
#include "gui/actions/triggers.h"


// ************************************************************************** //
//  local class Diffractogram
// ************************************************************************** //

//! A diffractogram display, with associated controls, for use in SubframeDiffractogram.

class Diffractogram final : public QWidget {
public:
    Diffractogram();

    QBoxLayout* box() const { return box_; }

private:
    void onNormChanged();
    void onHighlight();

    class DiffractogramPlot* plot_;
    QBoxLayout* box_;
    CComboBox comboNormType_{"normTyp", {"none", "monitor", "Δ monitor", "Δ time", "background"}};
    CRadioButton intenSum_{"intenSum", "sum"};
    CRadioButton intenAvg_{"intenAvg", "avg ×"};
    CDoubleSpinBox intenScale_{"intenScale", 6, 0.001};
    QAction* actZoom_{ new CToggle("actZoom", "zoom", false) };
    XTextButton enableZoom_{actZoom_};
    CCheckBox combine_{"dgram:combine", &gGui->toggles->combinedDgram};
    CCheckBox fixInten_{"dgram:fixInten", &gGui->toggles->fixedIntenDgram};
};


Diffractogram::Diffractogram() {

    setLayout((box_ = newQ::VBoxLayout()));
    box_->addWidget((plot_ = new DiffractogramPlot(*this)));
    auto hb = newQ::HBoxLayout();
    box_->addLayout(hb);

    hb->addWidget(new QLabel("normalize to:"));
    hb->addWidget(&comboNormType_);

    connect(&comboNormType_, _SLOT_(QComboBox, currentIndexChanged, int), [this](int index) {
            gSession->setNorm(eNorm(index)); });

    hb->addWidget(new QLabel(" intensity from:"));
    hb->addWidget(&intenSum_);
    hb->addWidget(&intenAvg_);
    hb->addWidget(&intenScale_);
    intenScale_.setDecimals(3);
    hb->addStretch();

    connect(&intenAvg_, &QRadioButton::toggled, [this](bool on) {
        intenScale_.setEnabled(on);
        intenScale_.setValue(gSession->intenScale());
        gSession->setIntenScaleAvg(on, intenScale_.value());
    });

    connect(&intenScale_, _SLOT_(QDoubleSpinBox, valueChanged, double), [this](double val) {
        if (val > 0)
            gSession->setIntenScaleAvg(gSession->intenScaledAvg(), val);
    });


    hb->addWidget(&enableZoom_);
    hb->addStretch();


    hb->addWidget(new XIconButton(&gGui->toggles->showBackground));
    hb->addWidget(&combine_);
    hb->addWidget(&fixInten_);

    connect(actZoom_, &QAction::toggled, this, [this](bool on) {
        plot_->setInteraction(QCP::iRangeDrag, on);
        plot_->setInteraction(QCP::iRangeZoom, on);
        plot_->enterZoom(on);
    });

    connect(gSession, &Session::sigDataHighlight, this, &Diffractogram::onHighlight);
    connect(gSession, &Session::sigNorm, this, &Diffractogram::onNormChanged);

    gGui->toggles->showBackground.setChecked(true);
    intenAvg_.setChecked(true);
}

void Diffractogram::onNormChanged() {
    intenScale_.setValue(gSession->intenScale()); // TODO own signal
    if (gSession->intenScaledAvg())
        intenAvg_.setChecked(true);
    else
        intenSum_.setChecked(true);
    plot_->renderAll();
}

void Diffractogram::onHighlight() {
    actZoom_->setChecked(false);
    plot_->renderAll();
}

// ************************************************************************** //
//  class SubframeDiffractogram
// ************************************************************************** //

SubframeDiffractogram::SubframeDiffractogram() {
    setTabPosition(QTabWidget::North);
    auto* tab = new QWidget();
    addTab(tab, "Diffractogram");
    tab->setLayout(newQ::VBoxLayout());
    tab->layout()->addWidget(new Diffractogram());
}
