#include <vector>

void stl_growth_in_loop(std::vector<int> &values, int n) {
  for (int i = 0; i < n; ++i) {
    values.push_back(i);
    values.emplace_back(i + 1);
  }
}

void suppressed_stl_growth_in_loop(std::vector<int> &values, int n) {
  for (int i = 0; i < n; ++i) {
    // tensorium-clang-audit: disable-next-line TCA008
    values.push_back(i);
  }
}
