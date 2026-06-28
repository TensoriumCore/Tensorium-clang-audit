#include <cstdio>
#include <iostream>

void c_io_outside_loop() { std::printf("outside\n"); }

void cxx_io_outside_loop() { std::cout << "outside\n"; }

void loop_without_io(int n) {
  int total = 0;
  for (int i = 0; i < n; ++i) {
    total += i;
  }
}
