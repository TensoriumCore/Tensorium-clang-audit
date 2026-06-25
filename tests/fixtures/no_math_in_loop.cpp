#include <cmath>

double expensive_math_outside_loop(double x) {
  return std::pow(x, 2.0) + std::sqrt(x);
}

void calls_math_helper_from_loop(double *output, int n, double x) {
  for (int i = 0; i < n; ++i) {
    output[i] = expensive_math_outside_loop(x);
  }
}
