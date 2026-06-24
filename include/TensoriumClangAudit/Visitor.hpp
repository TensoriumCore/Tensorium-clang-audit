#pragma once

#include "clang/AST/ExprCXX.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"

namespace clang {
class ASTContext;
class CallExpr;
class FunctionDecl;
class CXXNewExpr;
class DoStmt;
class ForStmt;
class WhleStmt;
} // namespace clang

namespace tensorium_clang_audit {

class TensoriumClangAuditVisitor final
    : public clang::RecursiveASTVisitor<TensoriumClangAuditVisitor> {
public:
  explicit TensoriumClangAuditVisitor(clang::ASTContext &Context);

  bool VisitFunctionDecl(clang::FunctionDecl *Function);
  bool VisitCXXNewExpr(clang::CXXNewExpr *Expression);
  bool VisitCallExpr(clang::CallExpr *Expression);
  
  bool TraverseForStmt(clang::ForStmt *Statement);
  bool TraverseWhileStmt(clang::WhileStmt *Statement);
  bool TraverseDoStmt(clang::DoStmt *Statement);


private:
  clang::ASTContext &Context;
  unsigned LoopDepth = 0;
};

} // namespace tensorium_clang_audit
