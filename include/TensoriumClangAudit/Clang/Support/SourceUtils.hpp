#pragma once

namespace clang {
class ASTContext;
class SourceLocation;
} // namespace clang

namespace tensorium_clang_audit {

bool isInMainFile(const clang::ASTContext &Context,
                  clang::SourceLocation Location);

} // namespace tensorium_clang_audit
