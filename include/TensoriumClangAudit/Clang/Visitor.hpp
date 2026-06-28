#pragma once

#include "clang/AST/ExprCXX.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include <vector>

namespace clang {
class ASTContext;
class CallExpr;
class CXXDeleteExpr;
class CXXNewExpr;
class CXXMemberCallExpr;
class CXXOperatorCallExpr;
class DoStmt;
class ForStmt;
class WhileStmt;
} // namespace clang

namespace tensorium_clang_audit {

struct TensoriumClangAuditOptions;

class TensoriumClangAuditVisitor final
    : public clang::RecursiveASTVisitor<TensoriumClangAuditVisitor> {
public:
  TensoriumClangAuditVisitor(clang::ASTContext &Context,
                             const TensoriumClangAuditOptions &Options);

  bool VisitCXXNewExpr(clang::CXXNewExpr *Expression);
  bool VisitCXXDeleteExpr(clang::CXXDeleteExpr *Expression);
  bool VisitCallExpr(clang::CallExpr *Expression);
  bool VisitCXXMemberCallExpr(clang::CXXMemberCallExpr *Expression);
  bool VisitCXXOperatorCallExpr(clang::CXXOperatorCallExpr *Expression);

  bool TraverseForStmt(clang::ForStmt *Statement);
  bool TraverseWhileStmt(clang::WhileStmt *Statement);
  bool TraverseDoStmt(clang::DoStmt *Statement);

private:
  clang::ASTContext &Context;
  const TensoriumClangAuditOptions &Options;
  unsigned LoopDepth = 0;
  std::vector<const clang::Stmt *> LoopStack;
};

} // namespace tensorium_clang_audit
