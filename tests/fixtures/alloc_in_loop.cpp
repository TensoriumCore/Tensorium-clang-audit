#include <cstdlib>

void cpp_allocation_in_loop(int n) {
  for (int i = 0; i < n; ++i) {
    int *value = new int(i);
    delete value;

    int *values = new int[n];
    delete[] values;
  }
}

void c_allocation_in_loop(int n) {
  for (int i = 0; i < n; ++i) {
    int *value = static_cast<int *>(std::malloc(sizeof(int)));

    if (value) {
      *value = i;
      std::free(value);
    }
  }
}
