#include "TensoriumClangAudit/Visitor.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Stmt.h"
#include "clang/Basic/Diagnostic.h"
#include "llvm/Support/raw_ostream.h"

namespace {
bool isCAllocationFunction(const clang::FunctionDecl *Function) {
  if (!Function) {
    return false;
  }
  const std::string Name = Function->getQualifiedNameAsString();
  return Name == "malloc" || Name == "::malloc" || Name == "std::malloc" ||
         Name == "calloc" || Name == "::calloc" || Name == "std::calloc" ||
         Name == "realloc" || Name == "::realloc" || Name == "std::realloc" ||
         Name == "free" || Name == "::free" || Name == "std::free";
}
} // namespace

namespace tensorium_clang_audit {

TensoriumClangAuditVisitor::TensoriumClangAuditVisitor(
    clang::ASTContext &Context)
    : Context(Context) {}

bool TensoriumClangAuditVisitor::VisitFunctionDecl(
    clang::FunctionDecl *Function) {
  if (!Function || !Function->isThisDeclarationADefinition()) {
    return true;
  }

  llvm::errs() << "TensoriumClangAudit: function "
               << Function->getQualifiedNameAsString() << "\n";
  return true;
}

bool TensoriumClangAuditVisitor::TraverseForStmt(clang::ForStmt *Statement) {
  ++LoopDepth;

  const bool Result =
      clang::RecursiveASTVisitor<TensoriumClangAuditVisitor>::TraverseForStmt(
          Statement);

  --LoopDepth;
  return Result;
}

bool TensoriumClangAuditVisitor::TraverseWhileStmt(
    clang::WhileStmt *Statement) {
  ++LoopDepth;

  const bool Result =
      clang::RecursiveASTVisitor<TensoriumClangAuditVisitor>::TraverseWhileStmt(
          Statement);

  --LoopDepth;
  return Result;
}

bool TensoriumClangAuditVisitor::TraverseDoStmt(clang::DoStmt *Statement) {
  ++LoopDepth;

  const bool Result =
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

bool TensoriumClangAuditVisitor::VisitCallExpr(clang::CallExpr *Expression) {
  if (!Expression || LoopDepth == 0) {
    return true;
  }

  const clang::FunctionDecl *Callee = Expression->getDirectCallee();

  if (!isCAllocationFunction(Callee)) {
    return true;
  }

  clang::DiagnosticsEngine &Diagnostics = Context.getDiagnostics();

  const unsigned DiagnosticID =
      Diagnostics.getCustomDiagID(clang::DiagnosticsEngine::Warning,
                                  "C allocation/deallocation call inside loop");

  Diagnostics.Report(Expression->getBeginLoc(), DiagnosticID);

  return true;
}

} // namespace tensorium_clang_audit
