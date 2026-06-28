#pragma once

#include "TensoriumClangAudit/Clang/Support/DiagnosticReporter.hpp"

namespace clang {
class ASTContext;
class SourceLocation;
} // namespace clang

namespace tensorium_clang_audit {

bool isSuppressed(clang::ASTContext &Context, clang::SourceLocation Location,
                  TensoriumDiagnostic Diagnostic);

} // namespace tensorium_clang_audit
