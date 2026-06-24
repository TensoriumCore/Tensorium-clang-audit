void foo() {}

int add(int a, int b) {
  return a + b;
}

double sum_first_n(int n) {
  double total = 0.0;

  for (int i = 0; i < n; ++i) {
    total += static_cast<double>(i);
  }

  return total;
}

int main() {
  foo();
  return add(1, 2) + static_cast<int>(sum_first_n(10));
}
