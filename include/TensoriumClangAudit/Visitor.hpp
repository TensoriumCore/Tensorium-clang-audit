#pragma once

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/ExprCXX.h"

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
  bool TraverseForStmt(clang::ForStmt *Statement);
  bool TraverseWhileStmt(clang::WhileStmt *Statement);
  bool TraverseDoStmt(clang::DoStmt *Statement);

  bool VisitCXXNewExpr(clang::CXXNewExpr *Expression);

private:
  clang::ASTContext &Context;
  unsigned LoopDepth = 0;
};

} // namespace tensorium_clang_audit
