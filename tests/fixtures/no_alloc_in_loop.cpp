#include <cstdlib>

int *allocate_outside_loop() { return new int(4); }

void free_outside_loop(int *value) { delete value; }

void *malloc_outside_loop() { return std::malloc(sizeof(int)); }

void free_c_outside_loop(void *value) { std::free(value); }

void calls_allocator_from_loop(int n) {
  for (int i = 0; i < n; ++i) {
    void *value = malloc_outside_loop();
    free_c_outside_loop(value);
  }
}

void numeric_loop_only(int n) {
  int total = 0;

  for (int i = 0; i < n; ++i) {
    total += i;
  }
}
