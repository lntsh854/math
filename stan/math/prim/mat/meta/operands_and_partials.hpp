#ifndef STAN_MATH_PRIM_MAT_META_OPERANDS_AND_PARTIALS_HPP
#define STAN_MATH_PRIM_MAT_META_OPERANDS_AND_PARTIALS_HPP

#include <stan/math/prim/mat/meta/broadcast_array.hpp>
#include <stan/math/prim/scal/meta/operands_and_partials.hpp>
#include <stan/math/prim/scal/meta/return_type.hpp>

#include <Eigen/Dense>
#include <vector>

namespace stan {
namespace math {
namespace internal {

/* This class will be used for both multivariate (nested container)
   operands_and_partials edges as well as for the univariate case.
 */
template <typename T_op, typename T_part, int R, int C>
class ops_partials_edge<T_part, Eigen::Matrix<T_op, R, C>> {
 public:
  typedef empty_broadcast_array<T_part, Eigen::Matrix<T_op, R, C>> partials_t;
  partials_t partials_;
  empty_broadcast_array<partials_t, Eigen::Matrix<T_op, R, C>> partials_vec_;
  ops_partials_edge() {}
  explicit ops_partials_edge(const Eigen::Matrix<T_op, R, C> ops) {}

 private:
  template <typename, typename, typename, typename, typename, typename>
  friend class stan::math::operands_and_partials;

  void dump_partials(double* /* partials */) const {}  // reverse mode
  void dump_operands(void* /* operands */) const {}    // reverse mode
  double dx() const { return 0; }                      // used for fvars
  int size() const { return 0; }
};

template <typename T_op, typename T_part, int R, int C>
class ops_partials_edge<T_part, std::vector<Eigen::Matrix<T_op, R, C>>> {
 public:
  typedef empty_broadcast_array<T_part, Eigen::Matrix<T_op, R, C>> partials_t;
  empty_broadcast_array<partials_t, Eigen::Matrix<T_op, R, C>> partials_vec_;
  ops_partials_edge() {}
  explicit ops_partials_edge(
      const std::vector<Eigen::Matrix<T_op, R, C>> ops) {}

 private:
  template <typename, typename, typename, typename, typename, typename>
  friend class stan::math::operands_and_partials;

  void dump_partials(double* /* partials */) const {}  // reverse mode
  void dump_operands(void* /* operands */) const {}    // reverse mode
  double dx() const { return 0; }                      // used for fvars
  int size() const { return 0; }
};

template <typename T_op, typename T_part>
class ops_partials_edge<T_part, std::vector<std::vector<T_op> > > {
 public:
  typedef empty_broadcast_array<T_part, std::vector<std::vector<T_op> > > partials_t;
  partials_t partials_;
  empty_broadcast_array<partials_t, std::vector<std::vector<T_op> > > partials_vec_;
  ops_partials_edge() {}
  explicit ops_partials_edge(const std::vector<std::vector<T_op> > ops) {}

 private:
  template <typename, typename, typename, typename, typename, typename>
  friend class stan::math::operands_and_partials;

  void dump_partials(double* /* partials */) const {}  // reverse mode
  void dump_operands(void* /* operands */) const {}    // reverse mode
  double dx() const { return 0; }                      // used for fvars
  int size() const { return 0; }
};
}  // namespace internal
}  // namespace math
}  // namespace stan
#endif