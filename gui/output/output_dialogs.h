// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      gui/output/output_dialogs.h
//! @brief     Defines ...
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef OUTPUT_DIALOGS_H
#define OUTPUT_DIALOGS_H

#include "actions.h"
#include "calc/calc_reflection_info.h"
#include "panels/panel.h"
#include "typ/typ_log.h"

class QProgressBar;

namespace gui {
namespace output {

/* Note that some data members are public, to simplify the code. Be careful. */

class Panel : public panel::GridPanel {
    CLASS(Panel) SUPER(panel::GridPanel) public : using super::super;
};

class PanelReflection : public Panel {
    CLASS(PanelReflection) SUPER(Panel) public : PanelReflection(TheHub&);

    QComboBox* cbRefl;
};

class PanelGammaSlices : public Panel {
    CLASS(PanelGammaSlices) SUPER(Panel) public : PanelGammaSlices(TheHub&);

    QSpinBox* numSlices;
    QDoubleSpinBox* stepGamma;

    void updateValues();

private:
    gma_rge rgeGma_;
};

class PanelGammaRange : public Panel {
    CLASS(PanelGammaRange) SUPER(Panel) public : PanelGammaRange(TheHub&);

    QCheckBox* cbLimitGamma;
    QDoubleSpinBox *minGamma, *maxGamma;

    void updateValues();

private:
    gma_rge rgeGma_;
};

class PanelPoints : public Panel {
    CLASS(PanelPoints) SUPER(Panel) public : PanelPoints(TheHub&);

    QRadioButton *rbCalc, *rbInterp;
};

class PanelInterpolation : public Panel {
    CLASS(PanelInterpolation) SUPER(Panel) public : PanelInterpolation(TheHub&);

    QDoubleSpinBox *stepAlpha, *stepBeta, *idwRadius;
    QDoubleSpinBox *avgAlphaMax, *avgRadius;
    QSpinBox* avgThreshold;
};

class PanelDiagram : public Panel {
    CLASS(PanelDiagram) SUPER(Panel) public : PanelDiagram(TheHub&);

    QComboBox *xAxis, *yAxis;
};

class PanelFitError : public Panel {
    CLASS(PanelFitError) SUPER(Panel) public : PanelFitError(TheHub&);
};

class Params : public QWidget, protected RefHub {
    CLASS(Params)
    SUPER(QWidget)
    public : enum ePanels {
        REFLECTION = 0x01,
        GAMMA = 0x02,
        POINTS = 0x04,
        INTERPOLATION = 0x08,
        DIAGRAM = 0x10,
    };

    Params(TheHub&, ePanels);
    ~Params();

    PanelReflection* panelReflection;
    PanelGammaSlices* panelGammaSlices;
    PanelGammaRange* panelGammaRange;
    PanelPoints* panelPoints;
    PanelInterpolation* panelInterpolation;
    PanelDiagram* panelDiagram;

    str saveDir, saveFmt;

private:
    void readSettings();
    void saveSettings() const;

    QBoxLayout* box_;
};

class Table : public TreeView, protected RefHub {
    CLASS(Table) SUPER(TreeView) public : Table(TheHub&, uint numDataColumns);

    void setColumns(str_lst::rc headers, str_lst::rc outHeaders, typ::cmp_vec::rc);
    str_lst const outHeaders() { return outHeaders_; }

    void clear();
    void addRow(typ::row_t::rc, bool sort);

    void sortData();

    uint rowCount() const;
    typ::row_t::rc row(uint) const;

private:
    scoped<class TableModel*> model_;
    str_lst outHeaders_;
};

class Tabs : public panel::TabsPanel {
    CLASS(Tabs) SUPER(panel::TabsPanel) public : Tabs(TheHub&);
};

class Tab : public QWidget, protected RefHub {
    CLASS(Tab) SUPER(QWidget) public : Tab(TheHub&, Params&);

protected:
    Params& params_;

    GridLayout* grid_;
};

class TabTable : public Tab {
    CLASS(TabTable)
    SUPER(Tab)
    public
        : TabTable(TheHub&, Params&, str_lst::rc headers, str_lst::rc outHeaders, typ::cmp_vec::rc);

private:
    struct showcol_t {
        str name;
        QCheckBox* cb;
    };

    typedef typ::vec<showcol_t> showcol_vec;

private:
    class ShowColsWidget : public QWidget {
        CLASS(ShowColsWidget)
        SUPER(QWidget) public : ShowColsWidget(Table&, showcol_vec&);

    private:
        Table& table_;
        showcol_vec& showCols_;

        QBoxLayout* box_;
        QRadioButton *rbHidden_, *rbAll_, *rbNone_, *rbInten_, *rbTth_, *rbFWHM_;
    };

public:
    Table* table;

private:
    ShowColsWidget* showColumnsWidget_;
    showcol_vec showCols_;
};

class TabSave : public Tab {
    CLASS(TabSave)
    SUPER(Tab) public : TabSave(TheHub&, Params&, bool withTypes);

    str filePath(bool withSuffix);
    str separator() const;

    Action *actBrowse, *actSave;

protected:
    str fileSetSuffix(rcstr);

    QLineEdit *dir_, *file_;
    QRadioButton *rbDat_, *rbCsv_;
};

class Frame : public QFrame, protected RefHub {
    CLASS(Frame)
    SUPER(QFrame) public : Frame(TheHub&, rcstr title, Params*, QWidget*);

protected:
    QAction *actClose_, *actCalculate_, *actInterpolate_;
    QToolButton *btnClose_, *btnCalculate_, *btnInterpolate_;

    QProgressBar* pb_;

    QBoxLayout* box_;
    Params* params_;
    Tabs* tabs_;

    typ::vec<calc::ReflectionInfos> calcPoints_, interpPoints_;

    Table* table_;

    void calculate();
    void interpolate();

    virtual void displayReflection(uint reflIndex, bool interpolated);

protected:
    uint getReflIndex() const;
    bool getInterpolated() const;

    void logMessage(rcstr) const;
    void logSuccess(bool) const;
    bool logCheckSuccess(rcstr path, bool) const;
};

//------------------------------------------------------------------------------
}
}
#endif // OUTPUT_DIALOGS_H
