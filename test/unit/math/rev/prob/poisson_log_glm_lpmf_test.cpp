#include <stan/math/rev.hpp>
#include <gtest/gtest.h>
#include <stan/math/prim/scal/fun/value_of.hpp>
#include <vector>
#include <cmath>

using Eigen::Dynamic;
using Eigen::Matrix;
using stan::math::var;
using std::vector;

//  We check that the values of the new regression match those of one built
//  from existing primitives.
TEST(ProbDistributionsPoissonLogGLM, glm_matches_poisson_log_doubles) {
  vector<int> y{15, 3, 5};
  Matrix<double, Dynamic, Dynamic> x(3, 2);
  x << -12, 46, -42, 24, 25, 27;
  Matrix<double, Dynamic, 1> beta(2, 1);
  beta << 0.3, 2;
  double alpha = 0.3;
  Matrix<double, Dynamic, 1> alphavec = alpha * Matrix<double, 3, 1>::Ones();
  Matrix<double, Dynamic, 1> theta(3, 1);
  theta = x * beta + alphavec;
  EXPECT_FLOAT_EQ((stan::math::poisson_log_lpmf(y, theta)),
                  (stan::math::poisson_log_glm_lpmf(y, x, alpha, beta)));
  EXPECT_FLOAT_EQ((stan::math::poisson_log_lpmf<true>(y, theta)),
                  (stan::math::poisson_log_glm_lpmf<true>(y, x, alpha, beta)));
}
//  We check that the values of the new regression match those of one built
//  from existing primitives.
TEST(ProbDistributionsPoissonLogGLM, glm_matches_poisson_log_doubles_rand) {
  for (size_t ii = 0; ii < 42; ii++) {
    vector<int> y(3);
    for (size_t i = 0; i < 3; i++) {
      y[i] = Matrix<unsigned int, 1, 1>::Random(1, 1)[0] % 200;
    }
    Matrix<double, Dynamic, Dynamic> x
        = Matrix<double, Dynamic, Dynamic>::Random(3, 2);
    Matrix<double, Dynamic, 1> beta
        = Matrix<double, Dynamic, Dynamic>::Random(2, 1);
    Matrix<double, 1, 1> alphamat = Matrix<double, 1, 1>::Random(1, 1);
    double alpha = alphamat[0];
    Matrix<double, Dynamic, 1> alphavec = alpha * Matrix<double, 3, 1>::Ones();
    Matrix<double, Dynamic, 1> theta(3, 1);
    theta = x * beta + alphavec;
    EXPECT_FLOAT_EQ((stan::math::poisson_log_lpmf(y, theta)),
                    (stan::math::poisson_log_glm_lpmf(y, x, alpha, beta)));
    EXPECT_FLOAT_EQ(
        (stan::math::poisson_log_lpmf<true>(y, theta)),
        (stan::math::poisson_log_glm_lpmf<true>(y, x, alpha, beta)));
  }
}
//  We check that the gradients of the new regression match those of one built
//  from existing primitives.
TEST(ProbDistributionsPoissonLogGLM, glm_matches_poisson_log_vars) {
  vector<int> y{14, 2, 5};
  Matrix<var, Dynamic, Dynamic> x(3, 2);
  x << -12, 46, -42, 24, 25, 27;
  Matrix<var, Dynamic, 1> beta(2, 1);
  beta << 0.3, 2;
  var alpha = 0.3;
  Matrix<var, Dynamic, 1> alphavec = alpha * Matrix<double, 3, 1>::Ones();
  Matrix<var, Dynamic, 1> theta(3, 1);
  theta = x * beta + alphavec;
  var lp = stan::math::poisson_log_lpmf(y, theta);
  lp.grad();

  double lp_val = lp.val();
  double alpha_adj = alpha.adj();
  Matrix<double, Dynamic, Dynamic> x_adj(3, 2);
  Matrix<double, Dynamic, 1> beta_adj(2, 1);
  for (size_t i = 0; i < 2; i++) {
    beta_adj[i] = beta[i].adj();
    for (size_t j = 0; j < 3; j++) {
      x_adj(j, i) = x(j, i).adj();
    }
  }

  stan::math::recover_memory();

  vector<int> y2{14, 2, 5};
  Matrix<var, Dynamic, Dynamic> x2(3, 2);
  x2 << -12, 46, -42, 24, 25, 27;
  Matrix<var, Dynamic, 1> beta2(2, 1);
  beta2 << 0.3, 2;
  var alpha2 = 0.3;
  var lp2 = stan::math::poisson_log_glm_lpmf(y2, x2, alpha2, beta2);
  lp2.grad();
  EXPECT_FLOAT_EQ(lp_val, lp2.val());
  EXPECT_FLOAT_EQ(alpha_adj, alpha2.adj());
  for (size_t i = 0; i < 2; i++) {
    EXPECT_FLOAT_EQ(beta_adj[i], beta2[i].adj());
    for (size_t j = 0; j < 3; j++) {
      EXPECT_FLOAT_EQ(x_adj(j, i), x2(j, i).adj());
    }
  }
}

TEST(ProbDistributionsPoissonLogGLM, broadcast_x) {
  vector<int> y{1, 0, 5};
  Matrix<double, 1, Dynamic> x(1, 2);
  x << -12, 46;
  Matrix<var, 1, Dynamic> x1 = x;
  Matrix<var, Dynamic, Dynamic> x_mat = x.replicate(3, 1);
  Matrix<double, Dynamic, 1> beta(2, 1);
  beta << 0.3, 2;
  Matrix<var, Dynamic, 1> beta1 = beta;
  Matrix<var, Dynamic, 1> beta2 = beta;
  var alpha1 = 0.3;
  var alpha2 = 0.3;

  var lp1 = stan::math::poisson_log_glm_lpmf(y, x1, alpha1, beta1);
  var lp2 = stan::math::poisson_log_glm_lpmf(y, x_mat, alpha2, beta2);

  EXPECT_DOUBLE_EQ(lp1.val(), lp2.val());

  (lp1 + lp2).grad();

  for (int i = 0; i < 2; i++) {
    EXPECT_DOUBLE_EQ(x1[i].adj(), x_mat.col(i).adj().sum());
    EXPECT_DOUBLE_EQ(beta1[i].adj(), beta2[i].adj());
  }
  EXPECT_DOUBLE_EQ(alpha1.adj(), alpha2.adj());
}

TEST(ProbDistributionsPoissonLogGLM, broadcast_y) {
  int y = 13;
  Matrix<int, Dynamic, 1> y_vec = Matrix<int, Dynamic, 1>::Constant(3, 1, y);
  Matrix<double, Dynamic, Dynamic> x(3, 2);
  x << -12, 46, -42, 24, 25, 27;
  Matrix<var, Dynamic, Dynamic> x1 = x;
  Matrix<var, Dynamic, Dynamic> x2 = x;
  Matrix<double, Dynamic, 1> beta(2, 1);
  beta << 0.3, 2;
  Matrix<var, Dynamic, 1> beta1 = beta;
  Matrix<var, Dynamic, 1> beta2 = beta;
  var alpha1 = 0.3;
  var alpha2 = 0.3;

  var lp1 = stan::math::poisson_log_glm_lpmf(y, x1, alpha1, beta1);
  var lp2 = stan::math::poisson_log_glm_lpmf(y_vec, x2, alpha2, beta2);

  EXPECT_DOUBLE_EQ(lp1.val(), lp2.val());

  (lp1 + lp2).grad();

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      EXPECT_DOUBLE_EQ(x1(j, i).adj(), x2(j, i).adj());
    }
    EXPECT_DOUBLE_EQ(beta1[i].adj(), beta2[i].adj());
  }
  EXPECT_DOUBLE_EQ(alpha1.adj(), alpha2.adj());
}

TEST(ProbDistributionsPoissonLogGLM,
     glm_matches_poisson_log_vars_zero_instances) {
  vector<int> y{};
  Matrix<var, Dynamic, Dynamic> x(0, 2);
  Matrix<var, Dynamic, 1> beta(2, 1);
  beta << 0.3, 2;
  var alpha = 0.3;
  Matrix<var, Dynamic, 1> theta(0, 1);
  var lp = stan::math::poisson_log_lpmf(y, theta);
  lp.grad();

  double lp_val = lp.val();
  double alpha_adj = alpha.adj();
  Matrix<double, Dynamic, 1> beta_adj(2, 1);
  for (size_t i = 0; i < 2; i++) {
    beta_adj[i] = beta[i].adj();
  }

  stan::math::recover_memory();

  vector<int> y2{};
  Matrix<var, Dynamic, Dynamic> x2(0, 2);
  Matrix<var, Dynamic, 1> beta2(2, 1);
  beta2 << 0.3, 2;
  var alpha2 = 0.3;
  var lp2 = stan::math::poisson_log_glm_lpmf(y2, x2, alpha2, beta2);
  lp2.grad();
  EXPECT_FLOAT_EQ(lp_val, lp2.val());
  EXPECT_FLOAT_EQ(alpha_adj, alpha2.adj());
  for (size_t i = 0; i < 2; i++) {
    EXPECT_FLOAT_EQ(beta_adj[i], beta2[i].adj());
  }
}

TEST(ProbDistributionsPoissonLogGLM,
     glm_matches_poisson_log_vars_zero_attributes) {
  vector<int> y{14, 2, 5};
  Matrix<var, Dynamic, Dynamic> x(3, 0);
  Matrix<var, Dynamic, 1> beta(0, 1);
  var alpha = 0.3;
  Matrix<var, Dynamic, 1> alphavec = alpha * Matrix<double, 3, 1>::Ones();
  Matrix<var, Dynamic, 1> theta(3, 1);
  theta = x * beta + alphavec;
  var lp = stan::math::poisson_log_lpmf(y, theta);
  lp.grad();

  double lp_val = lp.val();
  double alpha_adj = alpha.adj();

  stan::math::recover_memory();

  vector<int> y2{14, 2, 5};
  Matrix<var, Dynamic, Dynamic> x2(3, 0);
  Matrix<var, Dynamic, 1> beta2(0, 1);
  var alpha2 = 0.3;
  var lp2 = stan::math::poisson_log_glm_lpmf(y2, x2, alpha2, beta2);
  lp2.grad();
  EXPECT_FLOAT_EQ(lp_val, lp2.val());
  EXPECT_FLOAT_EQ(alpha_adj, alpha2.adj());
}

//  We check that the gradients of the new regression match those of one built
//  from existing primitives.
TEST(ProbDistributionsPoissonLogGLM, glm_matches_poisson_log_vars_rand) {
  for (size_t ii = 0; ii < 200; ii++) {
    vector<int> y(3);
    for (size_t i = 0; i < 3; i++) {
      y[i] = Matrix<unsigned int, 1, 1>::Random(1, 1)[0] % 200;
    }
    Matrix<double, Dynamic, Dynamic> xreal
        = Matrix<double, Dynamic, Dynamic>::Random(3, 2);
    Matrix<double, Dynamic, 1> betareal
        = Matrix<double, Dynamic, Dynamic>::Random(2, 1);
    Matrix<double, 1, 1> alphareal = Matrix<double, 1, 1>::Random(1, 1);
    Matrix<var, Dynamic, 1> beta = betareal;
    Matrix<var, Dynamic, 1> theta(3, 1);
    Matrix<var, Dynamic, Dynamic> x = xreal;
    var alpha = alphareal[0];
    Matrix<var, Dynamic, 1> alphavec = Matrix<double, 3, 1>::Ones() * alpha;
    theta = (x * beta) + alphavec;
    var lp = stan::math::poisson_log_lpmf(y, theta);
    lp.grad();

    double lp_val = lp.val();
    double alpha_adj = alpha.adj();
    Matrix<double, Dynamic, Dynamic> x_adj(3, 2);
    Matrix<double, Dynamic, 1> beta_adj(2, 1);
    for (size_t i = 0; i < 2; i++) {
      beta_adj[i] = beta[i].adj();
      for (size_t j = 0; j < 3; j++) {
        x_adj(j, i) = x(j, i).adj();
      }
    }

    stan::math::recover_memory();

    Matrix<var, Dynamic, 1> beta2 = betareal;
    Matrix<var, Dynamic, Dynamic> x2 = xreal;
    var alpha2 = alphareal[0];
    var lp2 = stan::math::poisson_log_glm_lpmf(y, x2, alpha2, beta2);
    lp2.grad();
    EXPECT_FLOAT_EQ(lp_val, lp2.val());
    EXPECT_FLOAT_EQ(alpha_adj, alpha2.adj());
    for (size_t i = 0; i < 2; i++) {
      EXPECT_FLOAT_EQ(beta_adj[i], beta2[i].adj());
      for (size_t j = 0; j < 3; j++) {
        EXPECT_FLOAT_EQ(x_adj(j, i), x2(j, i).adj());
      }
    }
  }
}

//  We check that the gradients of the new regression match those of one built
//  from existing primitives, in case beta is a scalar.
TEST(ProbDistributionsPoissonLogGLM,
     glm_matches_poisson_log_vars_rand_scal_beta) {
  for (size_t ii = 0; ii < 42; ii++) {
    vector<int> y(3);
    for (size_t i = 0; i < 3; i++) {
      y[i] = Matrix<unsigned int, 1, 1>::Random(1, 1)[0] % 200;
    }
    Matrix<double, Dynamic, Dynamic> xreal
        = Matrix<double, Dynamic, Dynamic>::Random(3, 1);
    double betareal = Matrix<double, Dynamic, Dynamic>::Random(1, 1)(0, 0);
    Matrix<double, 1, 1> alphareal = Matrix<double, 1, 1>::Random(1, 1);
    var beta = betareal;
    Matrix<var, Dynamic, 1> theta(3, 1);
    Matrix<var, Dynamic, Dynamic> x = xreal;
    var alpha = alphareal[0];
    Matrix<var, Dynamic, 1> alphavec = Matrix<double, 3, 1>::Ones() * alpha;
    theta = (x * beta) + alphavec;
    var lp = stan::math::poisson_log_lpmf(y, theta);
    lp.grad();

    double lp_val = lp.val();
    double alpha_adj = alpha.adj();
    Matrix<double, Dynamic, Dynamic> x_adj(3, 1);
    double beta_adj = beta.adj();
    for (size_t j = 0; j < 3; j++) {
      x_adj(j, 0) = x(j, 0).adj();
    }

    stan::math::recover_memory();

    var beta2 = betareal;
    Matrix<var, Dynamic, Dynamic> x2 = xreal;
    var alpha2 = alphareal[0];
    var lp2 = stan::math::poisson_log_glm_lpmf(y, x2, alpha2, beta2);
    lp2.grad();
    EXPECT_FLOAT_EQ(lp_val, lp2.val());
    EXPECT_FLOAT_EQ(beta_adj, beta2.adj());
    EXPECT_FLOAT_EQ(alpha_adj, alpha2.adj());
    for (size_t j = 0; j < 3; j++) {
      EXPECT_FLOAT_EQ(x_adj(j, 0), x2(j, 0).adj());
    }
  }
}

//  We check that the gradients of the new regression match those of one built
//  from existing primitives, for the GLM with varying intercept.
TEST(ProbDistributionsPoissonLogGLM, glm_matches_poisson_varying_intercept) {
  for (size_t ii = 0; ii < 42; ii++) {
    vector<int> y(3);
    for (size_t i = 0; i < 3; i++) {
      y[i] = Matrix<unsigned int, 1, 1>::Random(1, 1)[0] % 200;
    }
    Matrix<double, Dynamic, Dynamic> xreal
        = Matrix<double, Dynamic, Dynamic>::Random(3, 2);
    Matrix<double, Dynamic, 1> betareal
        = Matrix<double, Dynamic, Dynamic>::Random(2, 1);
    Matrix<double, Dynamic, 1> alphareal
        = Matrix<double, Dynamic, Dynamic>::Random(3, 1);

    Matrix<var, Dynamic, 1> beta = betareal;
    Matrix<var, Dynamic, 1> theta(3, 1);
    Matrix<var, Dynamic, Dynamic> x = xreal;
    Matrix<var, Dynamic, 1> alpha = alphareal;
    theta = (x * beta) + alpha;
    var lp = stan::math::poisson_log_lpmf(y, theta);

    lp.grad();

    double lp_val = lp.val();
    Matrix<double, Dynamic, 1> alpha_adj(3, 1);
    Matrix<double, Dynamic, Dynamic> x_adj(3, 2);
    Matrix<double, Dynamic, 1> beta_adj(2, 1);
    for (size_t i = 0; i < 2; i++) {
      beta_adj[i] = beta[i].adj();
      for (size_t j = 0; j < 3; j++) {
        x_adj(j, i) = x(j, i).adj();
      }
    }
    for (size_t j = 0; j < 3; j++) {
      alpha_adj[j] = alpha[j].adj();
    }

    stan::math::recover_memory();

    Matrix<var, Dynamic, 1> beta2 = betareal;
    Matrix<var, Dynamic, Dynamic> x2 = xreal;
    Matrix<var, Dynamic, 1> alpha2 = alphareal;

    var lp2 = stan::math::poisson_log_glm_lpmf(y, x2, alpha2, beta2);
    lp2.grad();

    EXPECT_FLOAT_EQ(lp_val, lp2.val());
    for (size_t i = 0; i < 2; i++) {
      EXPECT_FLOAT_EQ(beta_adj[i], beta2[i].adj());
      for (size_t j = 0; j < 3; j++) {
        EXPECT_FLOAT_EQ(x_adj(j, i), x2(j, i).adj());
      }
    }
    for (size_t j = 0; j < 3; j++) {
      EXPECT_FLOAT_EQ(alpha_adj[j], alpha2[j].adj());
    }
  }
}

//  We check that we can instantiate all different interface types.
TEST(ProbDistributionsPoissonLogGLM, glm_matches_poisson_log_interface_types) {
  double value = 0;
  double value2 = 0;

  int i = 1;
  std::vector<int> vi = {{1, 0}};
  double d = 1.0;
  std::vector<double> vd = {{1.0, 2.0}};
  Eigen::VectorXd ev(2);
  Eigen::RowVectorXd rv(2);
  Eigen::MatrixXd m1(1, 1);
  m1 << 1.0;
  Eigen::MatrixXd m(2, 2);
  ev << 1.0, 2.0;
  rv << 1.0, 2.0;
  m << 1.0, 2.0, 3.0, 4.0;

  value += stan::math::poisson_log_glm_lpmf(i, m1, d, d);
  value += stan::math::poisson_log_glm_lpmf(vi, m, vd, vd);
  value += stan::math::poisson_log_glm_lpmf(vi, m, ev, ev);
  value += stan::math::poisson_log_glm_lpmf(vi, m, rv, rv);

  var v = 1.0;
  std::vector<var> vv = {{1.0, 2.0}};
  Eigen::Matrix<var, -1, 1> evv(2);
  Eigen::Matrix<var, 1, -1> rvv(2);
  Eigen::Matrix<var, -1, -1> m1v(1, 1);
  m1v << 1.0;
  Eigen::Matrix<var, -1, -1> mv(2, 2);
  evv << 1.0, 2.0;
  rvv << 1.0, 2.0;
  mv << 1.0, 2.0, 3.0, 4.0;

  value2 += stan::math::poisson_log_glm_lpmf(i, m1v, v, v).val();
  value2 += stan::math::poisson_log_glm_lpmf(vi, mv, vv, vv).val();
  value2 += stan::math::poisson_log_glm_lpmf(vi, mv, evv, evv).val();
  value2 += stan::math::poisson_log_glm_lpmf(vi, mv, rvv, rvv).val();

  EXPECT_FLOAT_EQ(value, value2);
}

//  We check that the right errors are thrown.
TEST(ProbDistributionsPoissonLogGLM, glm_matches_poisson_log_error_checking) {
  int N = 3;
  int M = 2;
  int W = 4;

  Eigen::Matrix<int, -1, 1> y(N, 1);
  for (int n = 0; n < N; n++) {
    y[n] = Eigen::Matrix<unsigned int, -1, 1>::Random(1, 1)[0] % 200;
  }
  Eigen::Matrix<int, -1, 1> yw1(W, 1);
  for (int n = 0; n < W; n++) {
    yw1[n] = Eigen::Matrix<unsigned int, -1, 1>::Random(1, 1)[0] % 200;
  }
  Eigen::Matrix<int, -1, 1> yw2(N, 1);
  for (int n = 0; n < N; n++) {
    yw2[n] = -(Eigen::Matrix<unsigned int, -1, 1>::Random(1, 1)[0] % 200);
  }
  Eigen::Matrix<double, -1, -1> x = Eigen::Matrix<double, -1, -1>::Random(N, M);
  Eigen::Matrix<double, -1, -1> xw1
      = Eigen::Matrix<double, -1, -1>::Random(W, M);
  Eigen::Matrix<double, -1, -1> xw2
      = Eigen::Matrix<double, -1, -1>::Random(N, W);
  Eigen::Matrix<double, -1, -1> xw3
      = Eigen::Matrix<double, -1, -1>::Random(N, M) * NAN;
  Eigen::Matrix<double, -1, 1> alpha
      = Eigen::Matrix<double, -1, 1>::Random(N, 1);
  Eigen::Matrix<double, -1, 1> alphaw1
      = Eigen::Matrix<double, -1, 1>::Random(W, 1);
  Eigen::Matrix<double, -1, 1> alphaw2
      = Eigen::Matrix<double, -1, 1>::Random(N, 1) * NAN;
  Eigen::Matrix<double, -1, 1> beta
      = Eigen::Matrix<double, -1, 1>::Random(M, 1);
  Eigen::Matrix<double, -1, 1> betaw1
      = Eigen::Matrix<double, -1, 1>::Random(W, 1);
  Eigen::Matrix<double, -1, 1> betaw2
      = Eigen::Matrix<double, -1, 1>::Random(M, 1) * NAN;

  EXPECT_THROW(stan::math::poisson_log_glm_lpmf(yw1, x, alpha, beta),
               std::invalid_argument);
  EXPECT_THROW(stan::math::poisson_log_glm_lpmf(yw2, x, alpha, beta),
               std::domain_error);
  EXPECT_THROW(stan::math::poisson_log_glm_lpmf(y, xw1, alpha, beta),
               std::invalid_argument);
  EXPECT_THROW(stan::math::poisson_log_glm_lpmf(y, xw2, alpha, beta),
               std::invalid_argument);
  EXPECT_THROW(stan::math::poisson_log_glm_lpmf(y, xw3, alpha, beta),
               std::domain_error);
  EXPECT_THROW(stan::math::poisson_log_glm_lpmf(y, x, alphaw1, beta),
               std::invalid_argument);
  EXPECT_THROW(stan::math::poisson_log_glm_lpmf(y, x, alphaw2, beta),
               std::domain_error);
  EXPECT_THROW(stan::math::poisson_log_glm_lpmf(y, x, alpha, betaw1),
               std::invalid_argument);
  EXPECT_THROW(stan::math::poisson_log_glm_lpmf(y, x, alpha, betaw2),
               std::domain_error);
}

TEST(ProbDistributionsPoissonLogGLM, glm_matches_poisson_log_vars_propto) {
  vector<int> y{2, 0, 1, 2, 1, 0, 0, 1, 3, 0};
  Matrix<double, Dynamic, Dynamic> x(10, 3);
  x << -1.87936, 0.55093, -2.50689, 4.78584, 0.988523, -2.46141, 1.46229,
      2.21497, 1.72734, -0.916165, -0.563808, 0.165279, -0.752066, 1.43575,
      0.296557, -0.738422, 0.438686, 0.664492, 0.518203, -0.27485, 2, 0, 1, 2,
      1, 0, 0, 1, 3, 0;
  Matrix<var, Dynamic, 1> beta(3, 1);
  beta << 1.17711, 1.24432, -0.596639;
  var alpha = -1.04272;
  Matrix<var, Dynamic, 1> alphavec = alpha * Matrix<var, 10, 1>::Ones();
  Matrix<var, Dynamic, 1> theta(10, 1);
  theta = x * beta + alphavec;
  var lp = stan::math::poisson_log_lpmf<true>(y, theta);
  double lp_val = lp.val();
  var lp1 = stan::math::poisson_log_glm_lpmf<true>(y, x, alpha, beta);
  double lp1_val = lp1.val();
  EXPECT_FLOAT_EQ(lp_val, lp1_val);
}
