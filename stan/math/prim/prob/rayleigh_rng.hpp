#ifndef STAN_MATH_PRIM_PROB_RAYLEIGH_RNG_HPP
#define STAN_MATH_PRIM_PROB_RAYLEIGH_RNG_HPP

#include <stan/math/prim/meta.hpp>
#include <stan/math/prim/err.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <cmath>

namespace stan {
namespace math {

/** \ingroup prob_dists
 * Return a Rayleigh random variate with scale parameter sigma
 * using the specified random number generator.
 *
 * sigma can be a scalar or a one-dimensional container.
 *
 * @tparam T_scale Type of scale parameter
 * @tparam RNG class of random number generator
 * @param sigma (Sequence of) positive scale parameter(s)
 * @param rng random number generator
 * @return (Sequence of) Rayleigh random variate(s)
 * @throw std::domain_error if sigma is nonpositive
 */
template <typename T_scale, class RNG>
inline typename VectorBuilder<true, double, T_scale>::type rayleigh_rng(
    const T_scale& sigma, RNG& rng) {
  using boost::random::uniform_real_distribution;
  using boost::variate_generator;

  static const char* function = "rayleigh_rng";

  check_positive_finite(function, "Scale parameter", sigma);

  scalar_seq_view<T_scale> sigma_vec(sigma);
  size_t N = size(sigma);
  VectorBuilder<true, double, T_scale> output(N);

  variate_generator<RNG&, uniform_real_distribution<> > uniform_rng(
      rng, uniform_real_distribution<>(0.0, 1.0));
  for (size_t n = 0; n < N; ++n) {
    output[n] = sigma_vec[n] * std::sqrt(-2.0 * std::log(uniform_rng()));
  }

  return output.data();
}

}  // namespace math
}  // namespace stan
#endif
