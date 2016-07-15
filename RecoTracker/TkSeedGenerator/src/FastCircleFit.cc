#include "RecoTracker/TkSeedGenerator/interface/FastCircleFit.h"
#include "DataFormats/Math/interface/AlgebraicROOTObjects.h"

#include <Eigen/Dense>

#ifdef MK_DEBUG
#include <iostream>
#define PRINT std::cout
#else
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#define PRINT LogTrace("FastCircleFit")
#endif

#include <numeric>

namespace {
  template<typename T>
  T sqr(T t) { return t*t; }
}

FastCircleFit::FastCircleFit(const GlobalPoint *points, const GlobalError *errors, size_t size):
  x0_(0), y0_(0), rho_(0),
  chi2_(0)
{
  const auto N = size;
  declareDynArray(float, N, x);
  declareDynArray(float, N, y);
  declareDynArray(float, N, z);
  declareDynArray(float, N, weight); // 1/sigma^2

  // transform
  for(size_t i=0; i<N; ++i) {
    const auto& point = points[i];
    const auto p = point.basicVector();
    x[i] = p.x();
    y[i] = p.y();
    z[i] = sqr(p.x()) + sqr(p.y());

    // TODO: The following accounts only the hit uncertainty in phi.
    // Albeit it "works nicely", should we account also the
    // uncertainty in r in FPix (and the correlation between phi and r)?
    const float phiErr2 = errors[i].phierr(point);
    const float w = 1.f/(point.perp2()*phiErr2);
    weight[i] = w;

    PRINT << " point " << p.x()
          << " " << p.y()
          << " transformed " << x[i]
          << " " << y[i]
          << " " << z[i]
          << " weight " << w
          << std::endl;
  }
  const float invTotWeight = 1.f/std::accumulate(weight.begin(), weight.end(), 0.f);
  PRINT << " invTotWeight " << invTotWeight;

  Eigen::Vector3f mean = Eigen::Vector3f::Zero();
  for(size_t i=0; i<N; ++i) {
    const float w = weight[i];
    mean[0] += w*x[i];
    mean[1] += w*y[i];
    mean[2] += w*z[i];
  }
  mean *= invTotWeight;
  PRINT << " mean " << mean[0] << " " << mean[1] << " " << mean[2] << std::endl;

  Eigen::Matrix3f A = Eigen::Matrix3f::Zero();
  for(size_t i=0; i<N; ++i) {
    const auto diff_x = x[i] - mean[0];
    const auto diff_y = y[i] - mean[1];
    const auto diff_z = z[i] - mean[2];
    const auto w = weight[i];

    // exploit the fact that only lower triangular part of the matrix
    // is used in the eigenvalue calculation
    A(0,0) += w * diff_x * diff_x;
    A(1,0) += w * diff_y * diff_x;
    A(1,1) += w * diff_y * diff_y;
    A(2,0) += w * diff_z * diff_x;
    A(2,1) += w * diff_z * diff_y;
    A(2,2) += w * diff_z * diff_z;
    PRINT << " i " << i << " A " << A(0,0) << " " << A(0,1) << " " << A(0,2) << std::endl
          << "     " << A(1,0) << " " << A(1,1) << " " << A(1,2) << std::endl
          << "     " << A(2,0) << " " << A(2,1) << " " << A(2,2) << std::endl;
  }
  A *= 1./static_cast<float>(N);

  PRINT << " A " << A(0,0) << " " << A(0,1) << " " << A(0,2) << std::endl
        << "   " << A(1,0) << " " << A(1,1) << " " << A(1,2) << std::endl
        << "   " << A(2,0) << " " << A(2,1) << " " << A(2,2) << std::endl;

  // find eigenvalues and vectors
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> eigen(A);
  const auto& eigenValues = eigen.eigenvalues();
  const auto& eigenVectors = eigen.eigenvectors();

  // in Eigen the first eigenvalue is the smallest
  PRINT << " eigenvalues " << eigenValues[0]
        << " " << eigenValues[1]
        << " " << eigenValues[2]
        << std::endl;

  PRINT << " eigen " << eigenVectors(0,0) << " " << eigenVectors(0,1) << " " << eigenVectors(0,2) << std::endl
        << "       " << eigenVectors(1,0) << " " << eigenVectors(1,1) << " " << eigenVectors(1,2) << std::endl
        << "       " << eigenVectors(2,0) << " " << eigenVectors(2,1) << " " << eigenVectors(2,2) << std::endl;

  // eivenvector corresponding smallest eigenvalue
  auto n = eigenVectors.col(0);
  PRINT << " n (unit) " << n[0] << " " << n[1] << " " << n[2] << std::endl;

  const float c = -1.f * (n[0]*mean[0] + n[1]*mean[1] + n[2]*mean[2]);

  PRINT << " c " << c << std::endl;

  // calculate fit parameters
  const auto tmp = 0.5f * 1.f/n[2];
  x0_ = -n[0]*tmp;
  y0_ = -n[1]*tmp;
  const float rho2 = (1 - sqr(n[2]) - 4*c*n[2]) * sqr(tmp);
  rho_ = std::sqrt(rho2);

  // calculate square sum
  float S = 0;
  for(size_t i=0; i<N; ++i) {
    const float incr = sqr(c + n[0]*x[i] + n[1]*y[i] + n[2]*z[i]) * weight[i];
#if defined(MK_DEBUG) || defined(EDM_ML_DEBUG)
    const float distance = std::sqrt(sqr(x[i]-x0_) + sqr(y[i]-y0_)) - rho_;
    PRINT << " i " << i
          << " chi2 incr " << incr
          << " d(hit, circle) " << distance
          << " sigma " << 1.f/std::sqrt(weight[i])
          << std::endl;
#endif
    S += incr;
  }
  chi2_ = S;
}
