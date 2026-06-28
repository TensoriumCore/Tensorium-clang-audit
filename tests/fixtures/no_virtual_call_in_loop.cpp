struct ConcreteWorker {
  int step(int value) { return value + 1; }
};

struct FinalWorker final {
  virtual int step(int value) { return value + 1; }
};

int non_virtual_call_in_loop(ConcreteWorker &worker, int n) {
  int total = 0;
  for (int i = 0; i < n; ++i) {
    total += worker.step(i);
  }
  return total;
}

int final_virtual_call_in_loop(FinalWorker &worker, int n) {
  int total = 0;
  for (int i = 0; i < n; ++i) {
    total += worker.step(i);
  }
  return total;
}
