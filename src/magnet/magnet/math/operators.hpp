/*  dynamo:- Event driven molecular dynamics simulator 
    http://www.dynamomd.org
    Copyright (C) 2011  Marcus N Campbell Bannerman <m.bannerman@gmail.com>

    This program is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 3 as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

namespace magnet {
  namespace math {
    namespace detail {
      typedef enum {
	ADD,
	MULTIPLY,
	DIVIDE,
      } Op_t;
    }
    /*! \brief Symbolic representation of a binary operator. 
      
      When dealing with multiple symbols (Polynomial or Sin terms), it
      is convenient to have a representation of operators between
      them. This class represents these operations.
    */
    template<class LHStype, class RHStype, detail::Op_t Op>
    struct BinaryOp {
      LHStype _l;
      RHStype _r;
      
      BinaryOp(LHStype l, RHStype r): _l(l), _r(r) {}
      
      template<class R>
      auto operator()(const R& x) const -> decltype(_l(x) + _r(x)) {
	switch (Op){
	case detail::ADD: return _l(x) + _r(x);
	case detail::MULTIPLY: return _l(x) * _r(x);
	case detail::DIVIDE: return _l(x) / _r(x);
	}
      }
    };

    /*! \brief Provides expansion (and simplification) of symbolic
      functions.

      The purpose of this function is to reduce the complexity of
      symbolic expressions to accelerate any successive
      evaluations. This should not change the calculated values, but
      should optimise for use under repeated evaluations.

      The default operation is to do nothing.
    */
    template<class T> const T& expand(const T& f) { return f; }

    /*! \relates BinaryOp
      \brief Helper function for creation of addition BinaryOp types.
    */
    template<class LHS, class RHS>
    BinaryOp<LHS, RHS, detail::ADD> add(const LHS& l, const RHS& r)
    { return BinaryOp<LHS, RHS, detail::ADD>(l, r); }

    /*! \relates BinaryOp
      \brief Helper function for creation of subtraction BinaryOp types.
    */
    template<class LHS, class RHS>
    BinaryOp<LHS, RHS, detail::ADD> subtract(const LHS& l, const RHS& r)
    { return BinaryOp<LHS, RHS, detail::ADD>(l, -r); }

    /*! \relates BinaryOp
      \brief Helper function for creation of multiply BinaryOp types.
    */
    template<class LHS, class RHS>
    BinaryOp<LHS, RHS, detail::MULTIPLY> multiply(const LHS& l, const RHS& r)
    { return BinaryOp<LHS, RHS, detail::MULTIPLY>(l, r); }

    /*! \relates BinaryOp
      \brief Helper function for creation of multiply BinaryOp types.
    */
    template<class LHS, class RHS>
    BinaryOp<LHS, RHS, detail::ADD> divide(const LHS& l, const RHS& r)
    { return BinaryOp<LHS, RHS, detail::DIVIDE>(l, r); }

    /*! \relates BinaryOp
      \name BinaryOp algebra
      \{
    */

    /*! \brief Left-handed multiplication operator for BinaryOp types. */
    template<class LHS, class RHS, detail::Op_t Op, class RRHS>
    auto operator*(const BinaryOp<LHS, RHS, Op>& l, const RRHS& r) -> decltype(multiply(l,r))
    { return multiply(l,r); }

    /*! \brief Right-handed multiplication operator for BinaryOp types. */
    template<class LHS, class RHS, detail::Op_t Op, class LLHS>
    auto operator*(const LLHS& l, const BinaryOp<LHS, RHS, Op>& r) -> decltype(multiply(l,r))
    { return multiply(l,r); }

    /*! \brief Multiplication operator for two BinaryOp types. */
    template<class LHS1, class RHS1, detail::Op_t Op1, class LHS2, class RHS2, detail::Op_t Op2>
    auto operator*(const BinaryOp<LHS1, RHS1, Op1>& l, const BinaryOp<LHS2, RHS2, Op2>& r) -> decltype(multiply(l,r))
    { return multiply(l,r); }

    /*! \brief Left-handed addition operator for BinaryOp types. */
    template<class LHS, class RHS, detail::Op_t Op, class RRHS>
    auto operator+(const BinaryOp<LHS, RHS, Op>& l, const RRHS& r) -> decltype(add(l,r))
    { return add(l,r); }

    /*! \brief Right-handed addition operator for BinaryOp types. */
    template<class LHS, class RHS, detail::Op_t Op, class LLHS>
    auto operator+(const LLHS& l, const BinaryOp<LHS, RHS, Op>& r) -> decltype(add(l,r))
    { return add(l,r); }

    /*! \brief Addition operator for two BinaryOp types. */
    template<class LHS1, class RHS1, detail::Op_t Op1, class LHS2, class RHS2, detail::Op_t Op2>
    auto operator+(const BinaryOp<LHS1, RHS1, Op1>& l, const BinaryOp<LHS2, RHS2, Op2>& r) -> decltype(add(l,r))
    { return add(l,r); }

    /*! \brief Left-handed subtraction operator for BinaryOp types. */
    template<class LHS, class RHS, detail::Op_t Op, class RRHS>
    auto operator-(const BinaryOp<LHS, RHS, Op>& l, const RRHS& r) -> decltype(subtract(l,r))
    { return subtract(l,r); }

    /*! \brief Right-handed subtraction operator for BinaryOp types. */
    template<class LHS, class RHS, detail::Op_t Op, class LLHS>
    auto operator-(const LLHS& l, const BinaryOp<LHS, RHS, Op>& r) -> decltype(subtract(l,r))
    { return subtract(l,r); }

    /*! \brief Subtraction operator for two BinaryOp types. */
    template<class LHS1, class RHS1, detail::Op_t Op1, class LHS2, class RHS2, detail::Op_t Op2>
    auto operator-(const BinaryOp<LHS1, RHS1, Op1>& l, const BinaryOp<LHS2, RHS2, Op2>& r) -> decltype(subtract(l,r))
    { return subtract(l,r); }

    /*! \brief Expand addition BinaryOp types.

      If the classes have specialised operators for addition, then the
      decltype lookup will succeed and the addition is shunted to
      those classes. If not, this lookup will fail to expand the
      addition and it is instead carried out by the BinaryOp class.
    */
    template<class LHS, class RHS>
    auto expand(const BinaryOp<LHS, RHS, detail::ADD>& f) -> decltype(expand(f._l) + expand(f._r)) {
      return expand(f._l) + expand(f._r);
    }

    /*! \brief Expand multiplication BinaryOp types.

      If the classes have specialised operators for multiplication, then the
      decltype lookup will succeed and the addition is shunted to
      those classes. If not, this lookup will fail to expand the
      addition and it is instead carried out by the BinaryOp class.
    */
    template<class LHS, class RHS>
    auto expand(const BinaryOp<LHS, RHS, detail::MULTIPLY>& f) -> decltype(expand(f._l) * expand(f._r)) {
      return expand(f._l) * expand(f._r);
    }

    /*! \brief Derivatives of Addition operations.
     */
    template<class LHS, class RHS>
    auto derivative(const BinaryOp<LHS, RHS, detail::ADD>& f) -> decltype(derivative(f._l) + derivative(f._r))
    { return derivative(f._l) + derivative(f._r); }

    /*! \brief Derivatives of Multiplication operations.
     */
    template<class LHS, class RHS>
    auto derivative(const BinaryOp<LHS, RHS, detail::MULTIPLY>& f) -> decltype(add(multiply(derivative(f._l),f._r),multiply(derivative(f._r), f._l)))
    { return add(multiply(derivative(f._l),f._r),multiply(derivative(f._r), f._l)); }


    /*! \brief Determines the max and min over a certain range. */
    template<class LHS, class RHS, class Real>
    auto minmax(const BinaryOp<LHS, RHS, detail::ADD>& f, const Real x_min, const Real x_max) -> std::pair<decltype(minmax(f._l, x_min, x_max).first + minmax(f._r, x_min, x_max).first), decltype(minmax(f._l, x_min, x_max).second + minmax(f._r, x_min, x_max).second)>
    {
      typedef std::pair<decltype(minmax(f._l, x_min, x_max).first + minmax(f._r, x_min, x_max).first), decltype(minmax(f._l, x_min, x_max).second + minmax(f._r, x_min, x_max).second)> RetType;
      auto lv = minmax(f._l, x_min, x_max);
      auto rv = minmax(f._r, x_min, x_max);
      return RetType(lv.first + rv.first, lv.second + rv.second);
    }

    /*! \brief Determines the max and min over a certain range. */
    template<class LHS, class RHS, class Real>
    auto minmax(const BinaryOp<LHS, RHS, detail::MULTIPLY>& f, const Real x_min, const Real x_max) -> std::pair<decltype(minmax(f._l, x_min, x_max).first * minmax(f._r, x_min, x_max).first), decltype(minmax(f._l, x_min, x_max).second * minmax(f._r, x_min, x_max).second)>
    {
      typedef std::pair<decltype(minmax(f._l, x_min, x_max).first * minmax(f._r, x_min, x_max).first), decltype(minmax(f._l, x_min, x_max).second * minmax(f._r, x_min, x_max).second)> RetType;
      auto lv = minmax(f._l, x_min, x_max);
      auto rv = minmax(f._r, x_min, x_max);
      return RetType(lv.first * rv.first, lv.second * rv.second);
    }
    /*! \} */

    /*! \relates BinaryOp
      \name BinaryOp input/output operators
      \{
    */
    /*! \brief Writes a human-readable representation of the BinaryOp to the output stream. */
    template<class LHS, class RHS, detail::Op_t Op>
    inline std::ostream& operator<<(std::ostream& os, const BinaryOp<LHS, RHS, Op>& op) {
      os << "{" << op._l;
      switch (Op){
      case detail::ADD:      os << " + "; break;
      case detail::MULTIPLY: os << " * "; break;
      case detail::DIVIDE:   os << " / "; break;
      }
      os << op._r << "}";
      return os;
    }
    /*! \} */

    namespace {
      /*! \brief Generic implementation of the eval routine for PowerOp.
	
	As the types of non-arithmetic arguments to PowerOp might
	change with each round of multiplication, we must be careful
	to accommodate this using templated looping. This class
	achieves this.
      */
      template<size_t Power>
      struct PowerOpEval {
	template<class Arg_t>
	static auto eval(Arg_t x) -> decltype(PowerOpEval<Power-1>::eval(x) * x) {
	  return PowerOpEval<Power-1>::eval(x) * x;
	}
      };

      template<>
      struct PowerOpEval<1> {
	template<class Arg_t>
	static Arg_t eval(Arg_t x) {
	  return x;
	}
      };

      template<>
      struct PowerOpEval<0> {
	template<class Arg_t>
	static double eval(Arg_t x) {
	  return 1;
	}
      };
    }

    /*! \brief Symbolic representation of a (positive) power operator.
     */
    template<class Arg, size_t Power>
    struct PowerOp {
      Arg _arg;
      
      PowerOp(Arg a): _arg(a) {}
      
      /*! \brief Evaluate symbol at a value of x.
	
	This operator is only used if the result of evaluating the
	argument is an arithmetic type. If this is the case, the
	evaluation is passed to std::pow.
      */
      template<class R>
      auto operator()(const R& x) const -> typename std::enable_if<std::is_arithmetic<decltype(_arg(x))>::value, decltype(std::pow(_arg(x), Power))>::type {
	return std::pow(_arg(x), Power);
      }

      template<class R>
      auto operator()(const R& x) const -> typename std::enable_if<!std::is_arithmetic<decltype(_arg(x))>::value, decltype(std::pow(_arg(x), Power))>::type {
	return PowerOpEval<Power>::eval(_arg(x));
      }
    };


    /*! \relates PowerOp
      \name PowerOp helper functions.
    */
    /*! \brief Helper function for creating PowerOp types. */
    template<size_t N, class Arg>
    PowerOp<Arg, N> pow(const Arg& f)
    { return PowerOp<Arg, N>(f); }

    /*! \} */

    /*! \relates PowerOp
      \name PowerOp input/output operators.
    */
    /*! \brief Writes a human-readable representation of the BinaryOp to the output stream. */
    template<class Arg, size_t Power>
    inline std::ostream& operator<<(std::ostream& os, const PowerOp<Arg, Power>& p) {
      os << "(" << p._arg << ")^" << Power;
      return os;
    }
    /*! \} */

    /*! \relates PowerOp
      \name PowerOp algebra operations
    */

    /*! \brief Left-handed multiplication operator for PowerOp types. */
    template<class Arg, size_t Power> 
    auto expand(const PowerOp<Arg, Power>& f) -> decltype(PowerOpEval<Power>::eval(f._arg))
    { return PowerOpEval<Power>::eval(f._arg); }

    /*! \brief Left-handed multiplication operator for PowerOp types. */
    template<class Arg, size_t Power, class RHS>
    auto operator*(const PowerOp<Arg, Power>& l, const RHS& r) -> decltype(multiply(l, r))
    { return multiply(l, r); }
    
    /*! \brief Right-handed multiplication operator for PowerOp types. */
    template<class Arg, size_t Power, class RHS>
    auto operator*(const RHS& l, const PowerOp<Arg, Power>& r) -> decltype(multiply(l, r))
    { return multiply(l, r); }
    
    /*! \brief Multiplication operator for two PowerOp types. */
    template<class Arg1, size_t Power1, class Arg2, size_t Power2>
    auto operator*(const PowerOp<Arg1, Power1>& l, const PowerOp<Arg2, Power2>& r) -> decltype(multiply(l, r))
    { return multiply(l, r); }

    /*! \brief Left-handed addition operator for PowerOp types. */
    template<class Arg, size_t Power, class RHS>
    auto operator+(const PowerOp<Arg, Power>& l, const RHS& r) -> decltype(add(l, r))
    { return add(l, r); }
    
    /*! \brief Right-handed addition operator for PowerOp types. */
    template<class Arg, size_t Power, class RHS>
    auto operator+(const RHS& l, const PowerOp<Arg, Power>& r) -> decltype(add(l, r))
    { return add(l, r); }
    
    /*! \brief Addition operator for two PowerOp types. */
    template<class Arg1, size_t Power1, class Arg2, size_t Power2>
    auto operator+(const PowerOp<Arg1, Power1>& l, const PowerOp<Arg2, Power2>& r) -> decltype(add(l, r))
    { return add(l, r); }

    /*! \brief Left-handed subtraction operator for PowerOp types. */
    template<class Arg, size_t Power, class RHS>
    auto operator-(const PowerOp<Arg, Power>& l, const RHS& r) -> decltype(subtract(l, r))
    { return subtract(l, r); }
    
    /*! \brief Right-handed subtraction operator for PowerOp types. */
    template<class Arg, size_t Power, class RHS>
    auto operator-(const RHS& l, const PowerOp<Arg, Power>& r) -> decltype(subtract(l, r))
    { return subtract(l, r); }
    
    /*! \brief Subtraction operator for two PowerOp types. */
    template<class Arg1, size_t Power1, class Arg2, size_t Power2>
    auto operator-(const PowerOp<Arg1, Power1>& l, const PowerOp<Arg2, Power2>& r) -> decltype(subtract(l, r))
    { return subtract(l, r); }

    /*! \brief Derivatives of PowerOp operations.
     */
    template<class Arg, size_t Power>
    auto derivative(const PowerOp<Arg, Power>& f) -> decltype(derivative(f._arg) * PowerOp<Arg, Power-1>(f._arg))
    { return Power * derivative(f._arg) * PowerOp<Arg, Power-1>(f._arg); }

    template<class Arg>
    auto derivative(const PowerOp<Arg, 1>& f) -> decltype(derivative(f._arg))
    { return derivative(f._arg); }

    template<class Arg>
    auto derivative(const PowerOp<Arg, 2>& f) -> decltype(derivative(f._arg) * f._arg)
    { return 2 * derivative(f._arg) * f._arg; }

    template<class Arg>
    double derivative(const PowerOp<Arg, 0>& f)
    { return 0; }

    /*! \brief The maximum and minimum values of the PowerOp over a specifed range.
      \param f The PowerOp operation to evaluate.
      \param x_min The minimum x bound.
      \param x_max The maximum x bound.
    */
    template<class Arg, size_t Power, class Real>
    inline auto minmax(const PowerOp<Arg, Power>& f, const Real x_min, const Real x_max) -> std::pair<decltype(PowerOpEval<Power>::eval(minmax(f._arg, x_min, x_max).first)),
												      decltype(PowerOpEval<Power>::eval(minmax(f._arg, x_min, x_max).second))>
    {
      typedef std::pair<decltype(PowerOpEval<Power>::eval(minmax(f._arg, x_min, x_max).first)), decltype(PowerOpEval<Power>::eval(minmax(f._arg, x_min, x_max).second))> RetType;
      auto val = minmax(f._arg, x_min, x_max);
      
      auto min_pow = PowerOpEval<Power>::eval(val.first);
      auto max_pow = PowerOpEval<Power>::eval(val.second);
      
      //For odd powers, sign is preserved, so the arguments min^Power
      //is always less than the arguments max^Power
      if (Power % 2)
	return RetType(min_pow, max_pow);
      else {
	auto min = std::min(min_pow, max_pow);
	auto max = std::max(min_pow, max_pow);
	//If min-max range spans zero, we must include it.
	if ((val.first < 0) && (val.second > 0))
	  min = std::min(min, 0.0);

	return RetType(min, max);
      }
    }
    /*! \} */
  }
}
    
