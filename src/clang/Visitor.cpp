#include "TensoriumClangAudit/Clang/Visitor.hpp"

#include "TensoriumClangAudit/Clang/Checks/AllocationInLoopCheck.hpp"
#include "TensoriumClangAudit/Clang/Checks/IoInLoopCheck.hpp"
#include "TensoriumClangAudit/Clang/Checks/MathInLoopCheck.hpp"
#include "TensoriumClangAudit/Clang/Checks/StlInLoopCheck.hpp"
#include "TensoriumClangAudit/Clang/Checks/VirtualCallInLoopCheck.hpp"
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
  LoopStack.push_back(Statement);

  const bool Result =
      clang::RecursiveASTVisitor<TensoriumClangAuditVisitor>::TraverseForStmt(
          Statement);

  LoopStack.pop_back();
  --LoopDepth;
  return Result;
}

bool TensoriumClangAuditVisitor::TraverseWhileStmt(
    clang::WhileStmt *Statement) {
  ++LoopDepth;
  LoopStack.push_back(Statement);

  const bool Result =
      clang::RecursiveASTVisitor<TensoriumClangAuditVisitor>::TraverseWhileStmt(
          Statement);

  LoopStack.pop_back();
  --LoopDepth;
  return Result;
}

bool TensoriumClangAuditVisitor::TraverseDoStmt(clang::DoStmt *Statement) {
  ++LoopDepth;
  LoopStack.push_back(Statement);

  const bool Result =
      clang::RecursiveASTVisitor<TensoriumClangAuditVisitor>::TraverseDoStmt(
          Statement);

  LoopStack.pop_back();
  --LoopDepth;
  return Result;
}

bool TensoriumClangAuditVisitor::VisitCXXNewExpr(
    clang::CXXNewExpr *Expression) {
  if (!Expression || LoopDepth == 0) {
    return true;
  }
  if (!Options.LoopAnalysis.Alloc) {
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
  if (!Options.LoopAnalysis.Alloc) {
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
      Options.LoopAnalysis.Alloc && isCAllocationFunction(Callee);
  const bool IsDeallocation =
      Options.LoopAnalysis.Alloc && isCDeallocationFunction(Callee);
  const bool IsExpensiveMath =
      Options.LoopAnalysis.Maths && isExpensiveMathFunction(Callee);
  const bool IsCIo = Options.LoopAnalysis.Io && isCIoCall(Expression);
  if (!IsAllocation && !IsDeallocation && !IsExpensiveMath && !IsCIo) {
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
  } else if (IsCIo) {
    reportDiagnostic(Context, Expression->getBeginLoc(),
                     TensoriumDiagnostic::IoInLoop);
  } else {
    const clang::Stmt *CurrentLoop =
        LoopStack.empty() ? nullptr : LoopStack.back();
    const bool IsLoopInvariant =
        isLoopInvariantMathCall(Expression, CurrentLoop);

    reportDiagnostic(Context, Expression->getBeginLoc(),
                     IsLoopInvariant
                         ? TensoriumDiagnostic::LoopInvariantExpensiveMathInLoop
                         : TensoriumDiagnostic::ExpensiveMathInLoop);
  }

  return true;
}

bool TensoriumClangAuditVisitor::VisitCXXMemberCallExpr(
    clang::CXXMemberCallExpr *Expression) {
  if (!Expression || LoopDepth == 0) {
    return true;
  }
  if (!isInMainFile(Context, Expression->getBeginLoc())) {
    return true;
  }

  if (Options.LoopAnalysis.Stl && isStlGrowthCall(Expression)) {
    reportDiagnostic(Context, Expression->getBeginLoc(),
                     TensoriumDiagnostic::StlGrowthInLoop);
    return true;
  }

  if (Options.LoopAnalysis.VirtualCalls && isVirtualDispatchCall(Expression)) {
    reportDiagnostic(Context, Expression->getBeginLoc(),
                     TensoriumDiagnostic::VirtualCallInLoop);
  }

  return true;
}

bool TensoriumClangAuditVisitor::VisitCXXOperatorCallExpr(
    clang::CXXOperatorCallExpr *Expression) {
  if (!Expression || LoopDepth == 0) {
    return true;
  }
  if (!Options.LoopAnalysis.Io) {
    return true;
  }
  if (!isInMainFile(Context, Expression->getBeginLoc())) {
    return true;
  }
  if (!isCxxStreamOutputCall(Expression)) {
    return true;
  }

  reportDiagnostic(Context, Expression->getBeginLoc(),
                   TensoriumDiagnostic::IoInLoop);
  return true;
}

} // namespace tensorium_clang_audit
