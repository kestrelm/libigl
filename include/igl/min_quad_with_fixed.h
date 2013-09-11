#ifndef IGL_MIN_QUAD_WITH_FIXED_H
#define IGL_MIN_QUAD_WITH_FIXED_H
#include "igl_inline.h"

#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>
//#include <Eigen/SparseExtra>
// Bug in unsupported/Eigen/SparseExtra needs iostream first
#include <iostream>
#include <unsupported/Eigen/SparseExtra>

namespace igl
{
  template <typename T>
  struct min_quad_with_fixed_data;
  // MIN_QUAD_WITH_FIXED Minimize quadratic energy Z'*A*Z + Z'*B + C with
  // constraints that Z(known) = Y, optionally also subject to the constraints
  // Aeq*Z = Beq
  //
  // Templates:
  //   T  should be a eigen matrix primitive type like int or double
  // Inputs:
  //   A  n by n matrix of quadratic coefficients
  //   known list of indices to known rows in Z
  //   Y  list of fixed values corresponding to known rows in Z
  //   Aeq  m by n list of linear equality constraint coefficients
  //   pd flag specifying whether A(unknown,unknown) is positive definite
  // Outputs:
  //   data  factorization struct with all necessary information to solve
  //     using min_quad_with_fixed_solve
  // Returns true on success, false on error
  //
  // Benchmark: For a harmonic solve on a mesh with 325K facets, matlab 2.2
  // secs, igl/min_quad_with_fixed.h 7.1 secs
  //
  template <typename T, typename Derivedknown>
  IGL_INLINE bool min_quad_with_fixed_precompute(
    const Eigen::SparseMatrix<T>& A,
    const Eigen::PlainObjectBase<Derivedknown> & known,
    const Eigen::SparseMatrix<T>& Aeq,
    const bool pd,
    min_quad_with_fixed_data<T> & data
    );

  // Solves a system previously factored using min_quad_with_fixed_precompute
  // 
  // Template:
  //   T  type of sparse matrix (e.g. double) 
  //   DerivedY  type of Y (e.g. derived from VectorXd or MatrixXd)
  //   DerivedZ  type of Z (e.g. derived from VectorXd or MatrixXd)
  // Inputs:
  //   data  factorization struct with all necessary precomputation to solve
  //   B  n by 1 column of linear coefficients
  //   Beq  m by 1 list of linear equality constraint constant values
  // Outputs:
  //   Z  n by cols solution
  // Returns true on success, false on error
  template <
    typename T,
    typename DerivedB, 
    typename DerivedY,
    typename DerivedBeq, 
    typename DerivedZ>
  IGL_INLINE bool min_quad_with_fixed_solve(
    const min_quad_with_fixed_data<T> & data,
    const Eigen::PlainObjectBase<DerivedB> & B,
    const Eigen::PlainObjectBase<DerivedY> & Y,
    const Eigen::PlainObjectBase<DerivedBeq> & Beq,
    Eigen::PlainObjectBase<DerivedZ> & Z);
}

template <typename T>
struct igl::min_quad_with_fixed_data
{
  // Size of original system: number of unknowns + number of knowns
  int n;
  // Whether A(unknown,unknown) is positive definite
  bool Auu_pd;
  // Whether A(unknown,unknown) is symmetric
  bool Auu_sym;
  // Indices of known variables
  Eigen::VectorXi known;
  // Indices of unknown variables
  Eigen::VectorXi unknown;
  // Indices of lagrange variables
  Eigen::VectorXi lagrange;
  // Indices of unknown variable followed by Indices of lagrange variables
  Eigen::VectorXi unknown_lagrange;
  // Matrix multiplied against Y when constructing right hand side
  Eigen::SparseMatrix<T> preY;
  enum SolverType
  {
    LLT = 0,
    LDLT = 1,
    LU = 2,
    NUM_SOLVER_TYPES = 3
  } solver_type;
  // Solvers
  Eigen::SimplicialLLT <Eigen::SparseMatrix<T > > llt;
  Eigen::SimplicialLDLT<Eigen::SparseMatrix<T > > ldlt;
  Eigen::SparseLU<Eigen::SparseMatrix<T, Eigen::ColMajor>, Eigen::COLAMDOrdering<int> >   lu;
  // Debug
  Eigen::SparseMatrix<T> NA;
  Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> NB;
};

#ifdef IGL_HEADER_ONLY
#  include "min_quad_with_fixed.cpp"
#endif

#endif
