// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      core/calc/peak.h
//! @brief     Defines classes Peak and Peaks
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef PEAK_H
#define PEAK_H

#include <memory>
#include "core/fit/fit_fun.h"
#include "core/typ/types.h" // for fwhm_t
#include <QSharedPointer> // no auto rm

//! Wraps a PeakFunction (pimpl idiom)

class Peak {
public:
    Peak() = delete;
    Peak(const QString& functionName = "Raw");

    static Peak* from_json(const JsonObj&) THROWS;

    void setPeakFunction(const QString&);
    void setRange(const Range&);
    void invalidateGuesses();
    void setGuessPeak(const qpair& peak);
    void setGuessFWHM(fwhm_t fwhm);
    void fit(const Curve&);

    const PeakFunction& peakFunction() const;
    QString functionName() const { return peakFunction_->name(); }
    bool isRaw() const { return peakFunction_->isRaw(); }
    const Range& range() const { return peakFunction_->range(); }
    JsonObj to_json() const;

private:
    std::unique_ptr<PeakFunction> peakFunction_; //!< pimpl (pointer to implementation)
};


//! All user defined peaks, of which one is selected to be acted on by default.

class Peaks {
public:
    void clear();
    void add(const QString&);
    void add(const QJsonObject& obj);
    void remove();
    void select(int i);

    const Peak& at(int i) const { return *peaks_.at(i); }
    Peak& at(int i) { return *peaks_.at(i); } // used only once

    int count() const { return peaks_.size(); }
    QStringList names() const;
    QJsonArray toJson() const;
    Peak* selectedPeak() { return count() ? peaks_[selected_] : nullptr; };
    int selectedIndex() { return selected_; };

private:
    void add(Peak* peak);
    int selected_ {-1};
    std::vector<Peak*> peaks_;
};

#endif // PEAK_H
