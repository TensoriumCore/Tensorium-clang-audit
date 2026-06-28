#include "TensoriumClangAudit/Clang/Support/Suppression.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/ADT/StringRef.h"

#include <cstddef>

namespace {
llvm::StringRef
diagnosticCode(tensorium_clang_audit::TensoriumDiagnostic Diagnostic) {
  using tensorium_clang_audit::TensoriumDiagnostic;

  switch (Diagnostic) {
  case TensoriumDiagnostic::CXXAllocationInLoop:
    return "TCA001";
  case TensoriumDiagnostic::CXXDeallocationInLoop:
    return "TCA002";
  case TensoriumDiagnostic::CAllocationInLoop:
    return "TCA003";
  case TensoriumDiagnostic::CDeallocationInLoop:
    return "TCA004";
  case TensoriumDiagnostic::ExpensiveMathInLoop:
    return "TCA005";
  case TensoriumDiagnostic::NestedAllocationInLoop:
    return "TCA006";
  case TensoriumDiagnostic::LoopInvariantExpensiveMathInLoop:
    return "TCA007";
  case TensoriumDiagnostic::StlGrowthInLoop:
    return "TCA008";
  case TensoriumDiagnostic::IoInLoop:
    return "TCA009";
  case TensoriumDiagnostic::VirtualCallInLoop:
    return "TCA010";
  }
  return "";
}

llvm::StringRef getLine(llvm::StringRef Buffer, unsigned LineNumber) {
  if (LineNumber == 0) {
    return {};
  }

  unsigned CurrentLine = 1;
  std::size_t LineStart = 0;
  for (std::size_t Index = 0; Index <= Buffer.size(); ++Index) {
    if (Index != Buffer.size() && Buffer[Index] != '\n') {
      continue;
    }

    if (CurrentLine == LineNumber) {
      return Buffer.slice(LineStart, Index);
    }

    LineStart = Index + 1;
    ++CurrentLine;
  }

  return {};
}

bool lineSuppressesDiagnostic(llvm::StringRef Line, llvm::StringRef Code) {
  constexpr llvm::StringLiteral Prefix =
      "tensorium-clang-audit: disable-next-line";

  if (!Line.contains(Prefix)) {
    return false;
  }

  return Line.contains(Code) || Line.contains("all");
}
} // namespace

namespace tensorium_clang_audit {

bool isSuppressed(clang::ASTContext &Context, clang::SourceLocation Location,
                  TensoriumDiagnostic Diagnostic) {
  if (Location.isInvalid()) {
    return false;
  }

  const clang::SourceManager &SourceManager = Context.getSourceManager();
  const clang::SourceLocation ExpansionLocation =
      SourceManager.getExpansionLoc(Location);
  const std::pair<clang::FileID, unsigned> DecomposedLocation =
      SourceManager.getDecomposedLoc(ExpansionLocation);

  const clang::FileID File = DecomposedLocation.first;
  const unsigned LineNumber = SourceManager.getLineNumber(
      File, DecomposedLocation.second, /*Invalid=*/nullptr);
  if (LineNumber <= 1) {
    return false;
  }

  bool Invalid = false;
  const llvm::StringRef Buffer = SourceManager.getBufferData(File, &Invalid);
  if (Invalid) {
    return false;
  }

  const llvm::StringRef PreviousLine = getLine(Buffer, LineNumber - 1);
  return lineSuppressesDiagnostic(PreviousLine, diagnosticCode(Diagnostic));
}

} // namespace tensorium_clang_audit
