// ************************************************************************** //
//
//  STeCa2:    StressTexCalculator ver. 2
//
//! @file      core_fit_functions.h
//! @brief     Functions for data fitting
//!
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016
//! @authors   Scientific Computing Group at MLZ Garching
//! @authors   Original version: Christian Randau
//! @authors   Version 2: Antti Soininen, Jan Burle, Rebecca Brydon
//
// ************************************************************************** //

#ifndef CORE_FIT_FUNCTIONS_H
#define CORE_FIT_FUNCTIONS_H

#include "types/core_coords.h"
#include "types/core_type_curve.h"
#include "types/core_type_range.h"

namespace core { namespace fit {
//------------------------------------------------------------------------------
/// Abstract function

class Function {
public:
  class Parameter final {
  public:
    Parameter();

    qreal value() const { return value_; }

    Range valueRange() const;  ///< allowed range of values
    void setValueRange(qreal min, qreal max);

    /// checks whether a value/error pair would pass the constraints
    bool checkConstraints(qreal value, qreal error = 0);
    /// conditionally sets the new value/error pair
    bool setValue(qreal value, qreal error = 0, bool force = false);

  public:
    JsonObj saveJson() const;
    void    loadJson(rcJsonObj) THROWS;

    qreal maxErrorPercent() const;
    void setMaxErrorPercent(const qreal& maxErrorPercent);

  private:
    qreal value_;

    // constraints; TODO maybe not all needed?

    /// allowed range of values
    /// if !isValid() -> means the same as <value,value>, i.e. fixed value
    Range range_;

    /// maximum change allowed; NaN -> no check
    qreal maxDelta_;
    qreal maxDeltaPercent_;  // REVIEW - needed?

    /// maximum error allowed; NaN -> no check
    qreal maxError_;
    qreal maxErrorPercent_;  // REVIEW - needed?
  };

public:
  static Function* factory(rcstr);
  static Function* factory(rcJsonObj) THROWS;

  Function();
  virtual ~Function() {}

  virtual uint parameterCount() const = 0;

  virtual Parameter&       parameterAt(uint) = 0;
  virtual Parameter const& parameterAt(uint i) const {
    return const_cast<Function*>(this)->parameterAt(i);
  }

  /// evaluate the function y = f(x), with given (parValues) or own parameters
  virtual qreal y(qreal x, qreal const* parValues = nullptr) const = 0;

  /// partial derivative / parameter, with given (parValues) or own parameters
  virtual qreal dy(qreal x, uint parIndex,
                   qreal const* parValues = nullptr) const = 0;

public:
  virtual JsonObj saveJson() const;
  virtual void    loadJson(rcJsonObj) THROWS;
};

#ifndef QT_NO_DEBUG
// debug prints
QDebug& operator<<(QDebug&, Function const&);
#endif

//------------------------------------------------------------------------------
/// abstract function with parameters

class SimpleFunction : public Function {
  SUPER(SimpleFunction, Function)
public:
  SimpleFunction();

  void       setParameterCount(uint);
  uint       parameterCount() const;
  Parameter& parameterAt(uint);

  virtual void reset();

public:
  JsonObj saveJson() const;
  void    loadJson(rcJsonObj) THROWS;

protected:
  QVector<Parameter> parameters_;
  qreal parValue(uint parIndex, qreal const* parValues) const;
  void setValue(uint parIndex, qreal val);
};

//------------------------------------------------------------------------------
/// concrete function that is a sum of other functions

class SumFunctions final : public Function {
  SUPER(SumFunctions, Function)
public:
  SumFunctions();
  ~SumFunctions();

  /// add a (new) Function instance, take its ownership (for delete)
  void addFunction(Function*);

  /// aggregate parameter list for all added functions
  uint       parameterCount() const;
  Parameter& parameterAt(uint);

  qreal y(qreal x, qreal const* parValues = nullptr) const;
  qreal dy(qreal x, uint parIndex, qreal const* parValues = nullptr) const;

public:
  JsonObj saveJson() const;
  void    loadJson(rcJsonObj) THROWS;

protected:
  /// summed functions
  QVector<Function*> functions_;
  /// the aggregate parameter list
  QVector<Parameter*> allParameters_;
  /// look up the original function for a given aggregate parameter index
  QVector<Function*> function4parIndex_;
  /// the starting index of parameters of a summed function, given the aggregate
  /// parameter index
  QVector<uint> firstParIndex4parIndex_;
};

//------------------------------------------------------------------------------
/// a polynom(ial)

class Polynom : public SimpleFunction {
  SUPER(Polynom, SimpleFunction)
public:
  Polynom(uint degree = 0);

  uint degree() const;
  void setDegree(uint);

  qreal y(qreal x, qreal const* parValues = nullptr) const;
  qreal dy(qreal x, uint parIndex, qreal const* parValues = nullptr) const;

  qreal avgY(rcRange, qreal const* parValues = nullptr) const;

  void           fit(rcCurve, rcRanges);
  static Polynom fromFit(uint degree, rcCurve, rcRanges);

public:
  JsonObj saveJson() const;
  void    loadJson(rcJsonObj) THROWS;
};

//------------------------------------------------------------------------------
/// Abstract peak function

class PeakFunction : public SimpleFunction {
  SUPER(PeakFunction, SimpleFunction)
public:
  static PeakFunction* factory(ePeakType);

  PeakFunction();
  PeakFunction* clone() const;

  virtual ePeakType type() const = 0;

  rcRange      range() const { return range_; }
  virtual void setRange(rcRange);

  virtual void setGuessedPeak(rcXY);
  virtual void setGuessedFWHM(qreal);

  rcXY  guessedPeak() const { return guessedPeak_; }
  qreal guessedFWHM() const { return guessedFWHM_; }

  virtual XY    fittedPeak() const = 0;
  virtual qreal fittedFWHM() const = 0;

  void reset();

  bool         fit(rcCurve);
  virtual bool fit(rcCurve, rcRange);

protected:
  Curve prepareFit(rcCurve, rcRange);

public:
  JsonObj saveJson() const;
  void    loadJson(rcJsonObj) THROWS;

protected:
  Range range_;
  XY    guessedPeak_;
  qreal guessedFWHM_;
};

//------------------------------------------------------------------------------

class Raw : public PeakFunction {
  SUPER(Raw, PeakFunction)
public:
  Raw();

  ePeakType type() const { return ePeakType::RAW; }

  qreal y(qreal x, qreal const* parValues = nullptr) const;
  qreal dy(qreal x, uint parIndex, qreal const* parValues = nullptr) const;

  XY    fittedPeak() const;
  qreal fittedFWHM() const;

  void setRange(rcRange);
  bool fit(rcCurve, rcRange range);

private:
  Curve fittedCurve_;  ///< saved from fitting
  void  prepareY();

  mutable int   x_count_;
  mutable qreal dx_;
  mutable qreal sum_y_;

public:
  JsonObj saveJson() const;
};

//------------------------------------------------------------------------------

class Gaussian : public PeakFunction {
  SUPER(Gaussian, PeakFunction)
public:
  enum { parAMPL, parXSHIFT, parSIGMA };

  Gaussian(qreal ampl = 1, qreal xShift = 0, qreal sigma = 1);

  ePeakType type() const { return ePeakType::GAUSSIAN; }

  qreal y(qreal x, qreal const* parValues = nullptr) const;
  qreal dy(qreal x, uint parIndex, qreal const* parValues = nullptr) const;

  void setGuessedPeak(rcXY);
  void setGuessedFWHM(qreal);

  XY    fittedPeak() const;
  qreal fittedFWHM() const;

public:
  JsonObj saveJson() const;
};

//------------------------------------------------------------------------------

class CauchyLorentz : public PeakFunction {
  SUPER(CauchyLorentz, PeakFunction)
public:
  enum { parAMPL, parXSHIFT, parGAMMA };

  CauchyLorentz(qreal ampl = 1, qreal xShift = 0, qreal gamma = 1);

  ePeakType type() const { return ePeakType::LORENTZIAN; }

  qreal y(qreal x, qreal const* parValues = nullptr) const;
  qreal dy(qreal x, uint parIndex, qreal const* parValues = nullptr) const;

  void setGuessedPeak(rcXY);
  void setGuessedFWHM(qreal);

  XY    fittedPeak() const;
  qreal fittedFWHM() const;

public:
  JsonObj saveJson() const;
};

//------------------------------------------------------------------------------

class PseudoVoigt1 : public PeakFunction {
  SUPER(PseudoVoigt1, PeakFunction)
public:
  enum { parAMPL, parXSHIFT, parSIGMAGAMMA, parETA };

  PseudoVoigt1(qreal ampl = 1, qreal xShift = 0, qreal sigmaGamma = 1,
               qreal eta = 0.1);

  ePeakType type() const { return ePeakType::PSEUDOVOIGT1; }

  qreal y(qreal x, qreal const* parValues = nullptr) const;
  qreal dy(qreal x, uint parIndex, qreal const* parValues = nullptr) const;

  void setGuessedPeak(rcXY);
  void setGuessedFWHM(qreal);

  XY    fittedPeak() const;
  qreal fittedFWHM() const;

public:
  JsonObj saveJson() const;
};

//------------------------------------------------------------------------------

class PseudoVoigt2 : public PeakFunction {
  SUPER(PseudoVoigt2, PeakFunction)
public:
  enum { parAMPL, parXSHIFT, parSIGMA, parGAMMA, parETA };

  PseudoVoigt2(qreal ampl = 1, qreal xShift = 0, qreal sigma = 1,
               qreal gamma = 1, qreal eta = 0.1);

  ePeakType type() const { return ePeakType::PSEUDOVOIGT2; }

  qreal y(qreal x, qreal const* parValues = nullptr) const;
  qreal dy(qreal x, uint parIndex, qreal const* parValues = nullptr) const;

  void setGuessedPeak(rcXY);
  void setGuessedFWHM(qreal);

  XY    fittedPeak() const;
  qreal fittedFWHM() const;

public:
  JsonObj saveJson() const;
};

//------------------------------------------------------------------------------
}}
#endif  // CORE_FIT_FUNCTIONS_H
