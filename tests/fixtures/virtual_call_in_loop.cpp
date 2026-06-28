struct Worker {
  virtual ~Worker() = default;
  virtual int step(int value) = 0;
};

int virtual_call_in_loop(Worker &worker, int n) {
  int total = 0;
  for (int i = 0; i < n; ++i) {
    total += worker.step(i);
  }
  return total;
}

void suppressed_virtual_call_in_loop(Worker &worker, int n) {
  for (int i = 0; i < n; ++i) {
    // tensorium-clang-audit: disable-next-line TCA010
    worker.step(i);
  }
}
