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

void nested_cpp_allocation_in_loop(int n) {
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      int *value = new int(i + j);
      delete value;
    }
  }
}

void nested_c_allocation_in_loop(int n) {
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      int *value = static_cast<int *>(std::malloc(sizeof(int)));
      std::free(value);
    }
  }
}
