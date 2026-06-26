#include "TensoriumClangAudit/Clang/Support/DiagnosticReporter.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceLocation.h"

#include <cstddef>

namespace {
constexpr char TCA001[] = "TCA001: C++ allocation expression inside loop";
constexpr char TCA002[] = "TCA002: C++ deallocation expression inside loop";
constexpr char TCA003[] = "TCA003: C allocation call inside loop";
constexpr char TCA004[] = "TCA004: C deallocation call inside loop";
constexpr char TCA005[] = "TCA005: expensive math function call inside loop";
constexpr char TCA006[] =
    "TCA006: allocation or deallocation inside nested loop";

constexpr char TCA001Note[] =
    "consider reusing storage outside the loop or using a stack/value object";
constexpr char TCA002Note[] =
    "consider moving ownership cleanup outside the loop or using RAII storage";
constexpr char TCA003Note[] =
    "consider allocating once before the loop and reusing the buffer";
constexpr char TCA004Note[] = "consider freeing loop-owned storage after the "
                              "loop when ownership allows it";
constexpr char TCA005Note[] =
    "consider hoisting loop-invariant math or using a cheaper recurrence";
constexpr char TCA006Note[] = "consider moving allocation outside the outer "
                              "loop or reusing a workspace buffer";

template <std::size_t WarningSize, std::size_t NoteSize>
void reportWarningWithNote(clang::ASTContext &Context,
                           clang::SourceLocation Location,
                           const char (&Warning)[WarningSize],
                           const char (&Note)[NoteSize]) {
  clang::DiagnosticsEngine &Diagnostics = Context.getDiagnostics();

  const unsigned WarningID =
      Diagnostics.getCustomDiagID(clang::DiagnosticsEngine::Warning, Warning);
  Diagnostics.Report(Location, WarningID);

  const unsigned NoteID =
      Diagnostics.getCustomDiagID(clang::DiagnosticsEngine::Note, Note);
  Diagnostics.Report(Location, NoteID);
}
} // namespace

namespace tensorium_clang_audit {

void reportDiagnostic(clang::ASTContext &Context,
                      clang::SourceLocation Location,
                      TensoriumDiagnostic Diagnostic) {
  switch (Diagnostic) {
  case TensoriumDiagnostic::CXXAllocationInLoop:
    reportWarningWithNote(Context, Location, TCA001, TCA001Note);
    break;
  case TensoriumDiagnostic::CXXDeallocationInLoop:
    reportWarningWithNote(Context, Location, TCA002, TCA002Note);
    break;
  case TensoriumDiagnostic::CAllocationInLoop:
    reportWarningWithNote(Context, Location, TCA003, TCA003Note);
    break;
  case TensoriumDiagnostic::CDeallocationInLoop:
    reportWarningWithNote(Context, Location, TCA004, TCA004Note);
    break;
  case TensoriumDiagnostic::ExpensiveMathInLoop:
    reportWarningWithNote(Context, Location, TCA005, TCA005Note);
    break;
  case TensoriumDiagnostic::NestedAllocationInLoop:
    reportWarningWithNote(Context, Location, TCA006, TCA006Note);
    break;
  }
}

} // namespace tensorium_clang_audit
