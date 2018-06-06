#ifndef STAN_MATH_GPU_BASIC_MATRIX_GPU_HPP
#define STAN_MATH_GPU_BASIC_MATRIX_GPU_HPP
#ifdef STAN_OPENCL
#include <stan/math/gpu/matrix_gpu.hpp>
#include <stan/math/gpu/err/check_matrix_gpu.hpp>
#include <stan/math/prim/mat/fun/Eigen.hpp>
#include <stan/math/prim/scal/err/check_size_match.hpp>
#include <CL/cl.hpp>

/** @file stan/math/gpu/basic_matrix_gpu.hpp
*    @brief basic_matrix_gpu - basic matrix operations:
*    copy, copy lower/upper triangular, copy triangular transposed,
*    copy submatrix, init matrix with zeros, create an identity matrix,
*    add, subtract, transpose
*/
namespace stan {
  namespace math {
    enum triangularity {LOWER = 0, UPPER = 1, NONE = 2 };
    enum copy_transposed_triangular {LOWER_TO_UPPER_TRIANGULAR = 0,
      UPPER_TO_LOWER_TRIANGULAR = 1};
    /**
     * Stores the tranpose of the second matrix
     * to the first matrix. Both matrices 
     * are on the  GPU.
     * 
     * @param src the input matrix
     * 
     * @return transposed input matrix
     * 
     */
    inline matrix_gpu transpose(matrix_gpu & src) {
      matrix_gpu dst(src.cols(), src.rows());
      if ( dst.size() == 0 ) return dst;
      cl::Kernel kernel = opencl_context.get_kernel("transpose");
      cl::CommandQueue cmdQueue = opencl_context.queue();
      try {
        kernel.setArg(0, dst.buffer());
        kernel.setArg(1, src.buffer());
        kernel.setArg(2, src.rows());
        kernel.setArg(3, src.cols());
        cmdQueue.enqueueNDRangeKernel(
          kernel,
          cl::NullRange,
          cl::NDRange(src.rows(), src.cols()),
          cl::NullRange,
          NULL,
          NULL);
      } catch (const cl::Error& e) {
        check_opencl_error("transpose", e);
      }
      return dst;
    }

    /**
     * Stores zeros in the matrix on the GPU. 
     * It support writing the zeros to lower
     * triangular, upper triangular or the 
     * whole matrix.
     * 
     * @param A matrix
     * @param part optional parameter
     * that describes where to assign zeros: 
     * LOWER - lower triangular
     * UPPER - upper triangular
     * if the parameter is not specified, 
     * zeros are assigned to the whole matrix.
     * 
     */
    inline void zeros(matrix_gpu & A, triangularity part = NONE) {
      if ( A.size() == 0 ) return;
      cl::Kernel kernel = opencl_context.get_kernel("zeros");
      cl::CommandQueue cmdQueue = opencl_context.queue();
      try {
        kernel.setArg(0, A.buffer());
        kernel.setArg(1, A.rows());
        kernel.setArg(2, A.cols());
        kernel.setArg(3, part);
        cmdQueue.enqueueNDRangeKernel(
          kernel,
          cl::NullRange,
          cl::NDRange(A.rows(), A.cols()),
          cl::NullRange,
          NULL,
          NULL);
      } catch (const cl::Error& e) {
        check_opencl_error("zeros", e);
      }
    }

    /**
     * Returns the identity matrix stored on the GPU
     * 
     * @param rows_cols the number of rows and columns
     * 
     * @return the identity matrix
     * 
     */
    inline matrix_gpu identity(int rows_cols) {
      matrix_gpu A(rows_cols, rows_cols);
      if (rows_cols == 0) {
        return A;
      }
      cl::Kernel kernel = opencl_context.get_kernel("identity");
      cl::CommandQueue cmdQueue = opencl_context.queue();

      try {
        kernel.setArg(0, A.buffer());
        kernel.setArg(1, A.rows());
        kernel.setArg(2, A.cols());
        cmdQueue.enqueueNDRangeKernel(
          kernel,
          cl::NullRange,
          cl::NDRange(A.rows(), A.cols()),
          cl::NullRange,
          NULL,
          NULL);
      } catch (const cl::Error& e) {
        check_opencl_error("identity", e);
      }
      return A;
    }

    /**
     * Copies the lower or upper 
     * triangular of the source matrix to
     * the destination matrix. 
     * Both matrices are stored on the GPU.
     * 
     * @param src the source matrix
     * @param lower_upper enum to describe
     * which part of the matrix to copy: 
     * LOWER - copies the lower triangular
     * UPPER - copes the upper triangular
     * 
     * @return the matrix with the copied content
     * 
     */
    inline matrix_gpu copy_triangular(matrix_gpu & src,
     triangularity lower_upper) {
      if (src.size() == 0) {
        return src;
      }
      if (src.size() == 1) {
        return src;
      }
      matrix_gpu dst(src.rows(), src.cols());
      cl::Kernel kernel = opencl_context.get_kernel("copy_triangular");
      cl::CommandQueue cmdQueue = opencl_context.queue();
      try {
        kernel.setArg(0, dst.buffer());
        kernel.setArg(1, src.buffer());
        kernel.setArg(2, dst.rows());
        kernel.setArg(3, dst.cols());
        kernel.setArg(4, lower_upper);
        cmdQueue.enqueueNDRangeKernel(
          kernel,
          cl::NullRange,
          cl::NDRange(dst.rows(), dst.cols()),
          cl::NullRange,
          NULL,
          NULL);
      } catch (const cl::Error& e) {
        check_opencl_error("copy_triangular", e);
      }
      return dst;
    }
     /**
     * Copies a submatrix of the source matrix to 
     * the destination matrix. The submatrix to copy 
     * starts at (src_offset_rows, src_offset_cols)
     * and is of size size_rows x size_cols.
     * The submatrix is copied to the 
     * destination matrix starting at 
     * (dst_offset_rows, dst_offset_cols)
     * 
     * @param src the source matrix
     * @param dst the destination submatrix
     * @param src_offset_rows the offset row in src
     * @param src_offset_cols the offset column in src
     * @param dst_offset_rows the offset row in dst
     * @param dst_offset_cols the offset column in dst
     * @param size_rows the number of rows in the submatrix
     * @param size_cols the number of columns in the submatrix
     * 
     * @throw <code>std::invalid_argument</code> if 
     * 
     * 
     */
    inline void copy_submatrix(matrix_gpu & src,
     matrix_gpu & dst, int src_offset_rows, int src_offset_cols,
     int dst_offset_rows, int dst_offset_cols, int size_rows, int size_cols) {
      if (size_rows == 0 || size_cols == 0)
        return;
      if ( (src_offset_rows+size_rows) > src.rows() ||
           (src_offset_cols+size_cols) > src.cols() ) {
        domain_error("copy_submatrix", "submatrix in src" ,
          " is out of bounds", "");
      }
      if ((dst_offset_rows+size_rows) > dst.rows() ||
           (dst_offset_cols+size_cols) > dst.cols() ) {
        domain_error("copy_submatrix", "submatrix in src" ,
         " is out of bounds", "");
      }
      cl::Kernel kernel = opencl_context.get_kernel("copy_submatrix");
      cl::CommandQueue cmdQueue = opencl_context.queue();
      try {
        kernel.setArg(0, src.buffer());
        kernel.setArg(1, dst.buffer());

        kernel.setArg(2, src_offset_rows);
        kernel.setArg(3, src_offset_cols);
        kernel.setArg(4, dst_offset_rows);
        kernel.setArg(5, dst_offset_cols);

        kernel.setArg(6, size_rows);
        kernel.setArg(7, size_cols);

        kernel.setArg(8, src.rows());
        kernel.setArg(9, src.cols());
        kernel.setArg(10, dst.rows());
        kernel.setArg(11, dst.cols());

        cmdQueue.enqueueNDRangeKernel(
          kernel,
          cl::NullRange,
          cl::NDRange(size_rows, size_cols),
          cl::NullRange,
          NULL,
          NULL);
      } catch (const cl::Error& e) {
        check_opencl_error("copy_submatrix", e);
      }
    }

    /**
     * Copies the lower triangular 
     * to the upper triangular of the 
     * matrix on the GPU.
     * 
     * @param A matrix
     * @param lower_upper enum to describe
     * which copy operation to perform 
     * LOWER_TO_UPPER_TRIANGULAR - copies the lower 
     *   triangular to the upper triangular
     * UPPER_TO_LOWER_TRIANGULAR - copes the upper
     *  triangular to the lower triangular
     * 
     * @throw <code>std::invalid_argument</code> if the 
     * matrices do not have matching dimensions
     * 
     */
    inline void copy_triangular_transposed(matrix_gpu & A,
     copy_transposed_triangular lower_upper) {
      if (A.size() == 0) {
        return;
      }
      if (A.size() == 1) {
        return;
      }
      check_square("copy_triangular_transposed (GPU)", "A", A);
      cl::Kernel kernel =
        opencl_context.get_kernel("copy_triangular_transposed");
      cl::CommandQueue cmdQueue = opencl_context.queue();
      try {
        kernel.setArg(0, A.buffer());
        kernel.setArg(1, A.rows());
        kernel.setArg(2, A.cols());
        kernel.setArg(3, lower_upper);
        cmdQueue.enqueueNDRangeKernel(
          kernel,
          cl::NullRange,
          cl::NDRange(A.rows(), A.cols()),
          cl::NullRange,
          NULL,
          NULL);
      } catch (const cl::Error& e) {
        check_opencl_error("copy_triangular_transposed", e);
      }
    }
    /**
     * Matrix addition on the GPU
     * Adds the first two matrices and stores 
     * the result in the third matrix (C=A+B)
     * 
     * @param A first matrix
     * @param B second matrix
     * 
     * @return sum of A and B
     * 
     * @throw <code>std::invalid_argument</code> if the 
     * input matrices do not have matching dimensions
     * 
     */
    inline matrix_gpu add(matrix_gpu& A,
     matrix_gpu& B) {
      check_matching_dims("add (GPU)", "A", A, "B", B);
      matrix_gpu C(A.rows(), A.cols());
      if (C.size() == 0) {
        return C;
      }
      cl::Kernel kernel = opencl_context.get_kernel("add");
      cl::CommandQueue cmdQueue = opencl_context.queue();
      try {
        kernel.setArg(0, C.buffer());
        kernel.setArg(1, A.buffer());
        kernel.setArg(2, B.buffer());
        kernel.setArg(3, A.rows());
        kernel.setArg(4, A.cols());
        cmdQueue.enqueueNDRangeKernel(
          kernel,
          cl::NullRange,
          cl::NDRange(A.rows(), A.cols()),
          cl::NullRange,
          NULL,
          NULL);
      } catch (const cl::Error& e) {
        check_opencl_error("add", e);
      }
      return C;
    }

    /**
     * Matrix subtraction on the GPU
     * Subtracts the second matrix
     * from the first matrix and stores 
     * the result in the third matrix (C=A-B)
     * 
     * @param A first matrix
     * @param B second matrix
     * 
     * @return subtraction result matrix
     * 
     * @throw <code>std::invalid_argument</code> if the 
     * input matrices do not have matching dimensions
     * 
     */
    inline matrix_gpu subtract(matrix_gpu & A, matrix_gpu & B) {
      check_matching_dims("subtract (GPU)", "A", A, "B", B);
      matrix_gpu C(A.rows(), A.cols());
      if (A.size() == 0) {
        return C;
      }
      cl::Kernel kernel = opencl_context.get_kernel("subtract");
      cl::CommandQueue cmdQueue = opencl_context.queue();

      try {
        kernel.setArg(0, C.buffer());
        kernel.setArg(1, A.buffer());
        kernel.setArg(2, B.buffer());
        kernel.setArg(3, A.rows());
        kernel.setArg(4, A.cols());
        cmdQueue.enqueueNDRangeKernel(
          kernel,
          cl::NullRange,
          cl::NDRange(A.rows(), A.cols()),
          cl::NullRange,
          NULL,
          NULL);
      } catch (cl::Error& e) {
        check_opencl_error("subtract", e);
      }
      return C;
    }
  }
}

#endif
#endif