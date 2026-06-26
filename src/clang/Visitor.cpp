#include "TensoriumClangAudit/Clang/Visitor.hpp"

#include "TensoriumClangAudit/Clang/Checks/AllocationInLoopCheck.hpp"
#include "TensoriumClangAudit/Clang/Checks/MathInLoopCheck.hpp"
#include "TensoriumClangAudit/Clang/Options.hpp"
#include "TensoriumClangAudit/Clang/Support/DiagnosticReporter.hpp"
#include "TensoriumClangAudit/Clang/Support/SourceUtils.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Stmt.h"

namespace tensorium_clang_audit {

TensoriumClangAuditVisitor::TensoriumClangAuditVisitor(
    clang::ASTContext &Context, const TensoriumClangAuditOptions &Options)
    : Context(Context), Options(Options) {}

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
  if (!Options.CheckAllocInLoop) {
    return true;
  }
  if (!isInMainFile(Context, Expression->getBeginLoc())) {
    return true;
  }

  reportDiagnostic(Context, Expression->getBeginLoc(),
                   LoopDepth > 1 ? TensoriumDiagnostic::NestedAllocationInLoop
                                 : TensoriumDiagnostic::CXXAllocationInLoop);

  return true;
}

bool TensoriumClangAuditVisitor::VisitCXXDeleteExpr(
    clang::CXXDeleteExpr *Expression) {
  if (!Expression || LoopDepth == 0) {
    return true;
  }
  if (!Options.CheckAllocInLoop) {
    return true;
  }
  if (!isInMainFile(Context, Expression->getBeginLoc())) {
    return true;
  }

  reportDiagnostic(Context, Expression->getBeginLoc(),
                   LoopDepth > 1 ? TensoriumDiagnostic::NestedAllocationInLoop
                                 : TensoriumDiagnostic::CXXDeallocationInLoop);

  return true;
}

bool TensoriumClangAuditVisitor::VisitCallExpr(clang::CallExpr *Expression) {
  if (!Expression || LoopDepth == 0) {
    return true;
  }

  const clang::FunctionDecl *Callee = Expression->getDirectCallee();

  const bool IsAllocation =
      Options.CheckAllocInLoop && isCAllocationFunction(Callee);
  const bool IsDeallocation =
      Options.CheckAllocInLoop && isCDeallocationFunction(Callee);
  const bool IsExpensiveMath =
      Options.CheckMathInLoop && isExpensiveMathFunction(Callee);
  if (!IsAllocation && !IsDeallocation && !IsExpensiveMath) {
    return true;
  }
  if (!isInMainFile(Context, Expression->getBeginLoc())) {
    return true;
  }

  if ((IsAllocation || IsDeallocation) && LoopDepth > 1) {
    reportDiagnostic(Context, Expression->getBeginLoc(),
                     TensoriumDiagnostic::NestedAllocationInLoop);
  } else if (IsAllocation) {
    reportDiagnostic(Context, Expression->getBeginLoc(),
                     TensoriumDiagnostic::CAllocationInLoop);
  } else if (IsDeallocation) {
    reportDiagnostic(Context, Expression->getBeginLoc(),
                     TensoriumDiagnostic::CDeallocationInLoop);
  } else {
    reportDiagnostic(Context, Expression->getBeginLoc(),
                     TensoriumDiagnostic::ExpensiveMathInLoop);
  }

  return true;
}

} // namespace tensorium_clang_audit
