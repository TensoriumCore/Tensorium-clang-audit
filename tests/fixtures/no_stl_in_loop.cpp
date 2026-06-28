#include <vector>

void stl_growth_outside_loop(std::vector<int> &values) {
  values.push_back(1);
  values.emplace_back(2);
}

void presized_vector_loop(std::vector<int> &values, int n) {
  values.resize(n);

  for (int i = 0; i < n; ++i) {
    values[i] = i;
  }
}
