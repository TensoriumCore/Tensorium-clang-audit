#include <cstdio>
#include <iostream>

void c_io_in_loop(int n) {
  for (int i = 0; i < n; ++i) {
    std::printf("%d\n", i);
    std::fprintf(stderr, "%d\n", i);
  }
}

void cxx_io_in_loop(int n) {
  for (int i = 0; i < n; ++i) {
    std::cout << i << '\n';
    std::cerr << i << '\n';
  }
}

void suppressed_io_in_loop(int n) {
  for (int i = 0; i < n; ++i) {
    // tensorium-clang-audit: disable-next-line TCA009
    std::printf("%d\n", i);
  }
}
