#ifndef STAN_MATH_PRIM_MAT_FUN_ROUND_HPP
#define STAN_MATH_PRIM_MAT_FUN_ROUND_HPP

#include <stan/math/prim/mat/fun/Eigen.hpp>
#include <stan/math/prim/meta.hpp>
#include <stan/math/prim/scal/fun/round.hpp>
#include <stan/math/prim/vectorize/apply_scalar_unary.hpp>

namespace stan {
namespace math {

/**
 * Structure to wrap round() so it can be vectorized.
 *
 * @tparam T type of argument
 * @param x argument variable
 * @return Rounded value of x.
 */
struct round_fun {
  template <typename T>
  static inline T fun(const T& x) {
    return round(x);
  }
};

/**
 * Vectorized version of round.
 *
 * @tparam T type of container
 * @param x container
 * @return Rounded value of each value in x.
 */
template <typename T, typename = require_not_eigen_vt<std::is_arithmetic, T>>
inline auto round(const T& x) {
  return apply_scalar_unary<round_fun, T>::apply(x);
}

/**
 * Version of round() that accepts Eigen Matrix or matrix expressions.
 * @tparam Derived derived type of x
 * @param x Matrix or matrix expression
 * @return Rounded value of each value in x.
 */
template <typename Derived,
          typename = require_eigen_vt<std::is_arithmetic, Derived>>
inline auto round(const Eigen::MatrixBase<Derived>& x) {
  return x.derived().array().round().matrix();
}

}  // namespace math
}  // namespace stan

#endif
