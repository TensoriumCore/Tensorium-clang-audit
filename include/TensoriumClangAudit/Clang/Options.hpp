#pragma once

namespace tensorium_clang_audit {

struct TensoriumClangAuditOptions {
  bool Quiet = false;
  bool CheckAllocInLoop = true;
  bool CheckMathInLoop = true;
};

} // namespace tensorium_clang_audit
