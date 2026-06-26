#include <cmath>

void expensive_math_in_loop(double *output, int n, double x) {
  for (int i = 0; i < n; ++i) {
    output[i] = std::pow(x, 2.0) + std::sqrt(x) + std::sin(x) + std::cos(x);
  }
}

void loop_invariant_math_in_loop(double *output, int n, double alpha) {
  for (int i = 0; i < n; ++i) {
    output[i] = std::sqrt(alpha);
  }
}

void loop_variant_math_in_loop(double *output, const double *input, int n) {
  for (int i = 0; i < n; ++i) {
    output[i] = std::sqrt(input[i]);
  }
}
