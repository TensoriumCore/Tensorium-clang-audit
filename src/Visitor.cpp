#include "TensoriumClangAudit/Visitor.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Stmt.h"
#include "llvm/Support/raw_ostream.h"

namespace tensorium_clang_audit {

TensoriumClangAuditVisitor::TensoriumClangAuditVisitor(
    clang::ASTContext &Context)
    : Context(Context) {}

bool TensoriumClangAuditVisitor::VisitFunctionDecl(
    clang::FunctionDecl *Function) {
  (void)Context;

  if (!Function || !Function->isThisDeclarationADefinition()) {
    return true;
  }

  llvm::errs() << "TensoriumClangAudit: function "
               << Function->getQualifiedNameAsString() << "\n";
  return true;
}

bool TensoriumClangAuditVisitor::TraverseForStmt(clang::ForStmt *Statement) {
  ++LoopDepth;
  bool Result =
      clang::RecursiveASTVisitor<TensoriumClangAuditVisitor>::TraverseForStmt(
          Statement);
  --LoopDepth;
  return Result;
}

bool TensoriumClangAuditVisitor::TraverseWhileStmt(
    clang::WhileStmt *Statement) {
  ++LoopDepth;
  bool Result =
      clang::RecursiveASTVisitor<TensoriumClangAuditVisitor>::TraverseWhileStmt(
          Statement);
  --LoopDepth;
  return Result;
}

bool TensoriumClangAuditVisitor::TraverseDoStmt(clang::DoStmt *Statement) {
  ++LoopDepth;
  bool Result =
      clang::RecursiveASTVisitor<TensoriumClangAuditVisitor>::TraverseDoStmt(
          Statement);
  --LoopDepth;
  return Result;
}

bool TensoriumClangAuditVisitor::VisitCXXNewExpr(
    clang::CXXNewExpr *Expression) {
  if (!Expression || LoopDepth == 0) {
    return true;
  }

  clang::DiagnosticsEngine &Diagnostics = Context.getDiagnostics();

  const unsigned DiagnosticID = Diagnostics.getCustomDiagID(
      clang::DiagnosticsEngine::Warning, "dynamic allocation inside loop");

  Diagnostics.Report(Expression->getBeginLoc(), DiagnosticID);

  return true;
}

} // namespace tensorium_clang_audit
