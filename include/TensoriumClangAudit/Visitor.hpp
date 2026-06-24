#pragma once

#include "clang/AST/RecursiveASTVisitor.h"

namespace clang {
class ASTContext;
class FunctionDecl;
} // namespace clang

namespace tensorium_clang_audit {

class TensoriumClangAuditVisitor final
    : public clang::RecursiveASTVisitor<TensoriumClangAuditVisitor> {
public:
  explicit TensoriumClangAuditVisitor(clang::ASTContext &Context);

  bool VisitFunctionDecl(clang::FunctionDecl *Function);

private:
  clang::ASTContext &Context;
};

} // namespace tensorium_clang_audit
