#pragma once

namespace clang {
class ASTContext;
class SourceLocation;
} // namespace clang

namespace tensorium_clang_audit {

enum class TensoriumDiagnostic {
  CXXAllocationInLoop,
  CXXDeallocationInLoop,
  CAllocationInLoop,
  CDeallocationInLoop,
  ExpensiveMathInLoop,
  NestedAllocationInLoop,
  LoopInvariantExpensiveMathInLoop,
  StlGrowthInLoop,
  IoInLoop,
  VirtualCallInLoop,
};

void reportDiagnostic(clang::ASTContext &Context,
                      clang::SourceLocation Location,
                      TensoriumDiagnostic Diagnostic);

} // namespace tensorium_clang_audit
