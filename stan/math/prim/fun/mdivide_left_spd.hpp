#ifndef STAN_MATH_PRIM_MAT_FUN_MDIVIDE_LEFT_SPD_HPP
#define STAN_MATH_PRIM_MAT_FUN_MDIVIDE_LEFT_SPD_HPP

#include <stan/math/prim/err.hpp>
#include <stan/math/prim/fun/Eigen.hpp>
#include <stan/math/prim/fun/promote_common.hpp>

namespace stan {
namespace math {

/**
 * Returns the solution of the system Ax=b where A is symmetric positive
 * definite.
 *
 * @tparam T1 type of elements in the first matrix
 * @tparam T2 type of elements in the right-hand side matrix or vector
 * @tparam R1 number of rows in the first matrix, can be Eigen::Dynamic
 * @tparam C1 number of columns in the first matrix, can be Eigen::Dynamic
 * @tparam R2 number of rows in the right-hand side matrix, can be
 *         Eigen::Dynamic
 * @tparam C2 number of columns in the right-hand side matrix, can be
 *         Eigen::Dynamic
 *
 * @param A Matrix.
 * @param b Right hand side matrix or vector.
 * @return x = A^-1 b, solution of the linear system.
 * @throws std::domain_error if A is not square or the rows of b don't
 * match the size of A.
 */
template <typename T1, typename T2, int R1, int C1, int R2, int C2>
inline Eigen::Matrix<return_type_t<T1, T2>, R1, C2> mdivide_left_spd(
    const Eigen::Matrix<T1, R1, C1> &A, const Eigen::Matrix<T2, R2, C2> &b) {
  check_multiplicable("mdivide_left_spd", "A", A, "b", b);
  check_pos_definite("mdivide_left_spd", "A", A);
  return promote_common<Eigen::Matrix<T1, R1, C1>, Eigen::Matrix<T2, R1, C1> >(
             A)
      .llt()
      .solve(
          promote_common<Eigen::Matrix<T1, R2, C2>, Eigen::Matrix<T2, R2, C2> >(
              b));
}

}  // namespace math
}  // namespace stan

#endif
