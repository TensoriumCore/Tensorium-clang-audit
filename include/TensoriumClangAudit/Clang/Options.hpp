#pragma once

namespace tensorium_clang_audit {

struct LoopAnalysisOptions {
  bool Alloc = true;
  bool Maths = true;
};

struct TensoriumClangAuditOptions {
  bool Quiet = false;
  LoopAnalysisOptions LoopAnalysis;
};

} // namespace tensorium_clang_audit
