#include "TensoriumClangAudit/Visitor.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Stmt.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceManager.h"

namespace {
bool isCAllocationFunction(const clang::FunctionDecl *Function) {
  if (!Function) {
    return false;
  }
  const std::string Name = Function->getQualifiedNameAsString();
  return Name == "malloc" || Name == "::malloc" || Name == "std::malloc" ||
         Name == "calloc" || Name == "::calloc" || Name == "std::calloc" ||
         Name == "realloc" || Name == "::realloc" || Name == "std::realloc";
}

bool isCDeallocationFunction(const clang::FunctionDecl *Function) {
  if (!Function) {
    return false;
  }
  const std::string Name = Function->getQualifiedNameAsString();
  return Name == "free" || Name == "::free" || Name == "std::free";
}

bool isInMainFile(const clang::ASTContext &Context,
                  clang::SourceLocation Location) {
  if (Location.isInvalid()) {
    return false;
  }

  const clang::SourceManager &SourceManager = Context.getSourceManager();
  const clang::SourceLocation ExpansionLocation =
      SourceManager.getExpansionLoc(Location);

  return SourceManager.getFileID(ExpansionLocation) ==
         SourceManager.getMainFileID();
}
} // namespace

namespace tensorium_clang_audit {

TensoriumClangAuditVisitor::TensoriumClangAuditVisitor(
    clang::ASTContext &Context)
    : Context(Context) {}

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
  if (!isInMainFile(Context, Expression->getBeginLoc())) {
    return true;
  }
  clang::DiagnosticsEngine &Diagnostics = Context.getDiagnostics();

  const unsigned DiagnosticID = Diagnostics.getCustomDiagID(
      clang::DiagnosticsEngine::Warning, "dynamic allocation inside loop");

  Diagnostics.Report(Expression->getBeginLoc(), DiagnosticID);

  return true;
}

bool TensoriumClangAuditVisitor::VisitCXXDeleteExpr(
    clang::CXXDeleteExpr *Expression) {
  if (!Expression || LoopDepth == 0) {
    return true;
  }
  if (!isInMainFile(Context, Expression->getBeginLoc())) {
    return true;
  }
  clang::DiagnosticsEngine &Diagnostics = Context.getDiagnostics();

  const unsigned DiagnosticID = Diagnostics.getCustomDiagID(
      clang::DiagnosticsEngine::Warning, "dynamic deallocation inside loop");

  Diagnostics.Report(Expression->getBeginLoc(), DiagnosticID);

  return true;
}

bool TensoriumClangAuditVisitor::VisitCallExpr(clang::CallExpr *Expression) {
  if (!Expression || LoopDepth == 0) {
    return true;
  }

  const clang::FunctionDecl *Callee = Expression->getDirectCallee();

  const bool IsAllocation = isCAllocationFunction(Callee);
  const bool IsDeallocation = isCDeallocationFunction(Callee);
  if (!IsAllocation && !IsDeallocation) {
    return true;
  }
  if (!isInMainFile(Context, Expression->getBeginLoc())) {
    return true;
  }
  clang::DiagnosticsEngine &Diagnostics = Context.getDiagnostics();

  const unsigned DiagnosticID =
      IsAllocation
          ? Diagnostics.getCustomDiagID(clang::DiagnosticsEngine::Warning,
                                        "C allocation call inside loop")
          : Diagnostics.getCustomDiagID(clang::DiagnosticsEngine::Warning,
                                        "C deallocation call inside loop");

  Diagnostics.Report(Expression->getBeginLoc(), DiagnosticID);

  return true;
}

} // namespace tensorium_clang_audit
