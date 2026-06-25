#include <cstdlib>

void foo() {}

int add(int a, int b) { return a + b; }

double sum_first_n(int n) {
  double total = 0.0;

  for (int i = 0; i < n; ++i) {
    total += static_cast<double>(i);
  }

  return total;
}

void c_allocation_in_loop(int n) {
  for (int i = 0; i < n; i++) {
    int *ptr = static_cast<int *>(std::malloc(sizeof(n)));

    if (ptr) {
      *ptr = i;
      std::free(ptr);
    }
  }
}

void nested_allocation_in_loop(int n) {
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      int *value = new int(i + j);
      delete value;
    }
  }
}

void allocation_in_loop(int n) {
  for (int i = 0; i < n; ++i) {
    int *ptr = new int(i);
    delete ptr;

    int *values = new int[n];
    delete[] values;
  }
}

int main() {
  foo();
  allocation_in_loop(4);
  c_allocation_in_loop(4);
  nested_allocation_in_loop(4);
  return add(1, 2) + static_cast<int>(sum_first_n(10));
}
