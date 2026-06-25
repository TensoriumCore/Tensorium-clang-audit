#include <cmath>

void expensive_math_in_loop(double *output, int n, double x) {
  for (int i = 0; i < n; ++i) {
    output[i] = std::pow(x, 2.0) + std::sqrt(x) + std::sin(x) + std::cos(x);
  }
}
