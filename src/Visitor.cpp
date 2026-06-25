#include "TensoriumClangAudit/Visitor.hpp"

#include "TensoriumClangAudit/Options.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Stmt.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceManager.h"

#include <cstddef>

namespace {
constexpr char TCA001[] = "TCA001: C++ allocation expression inside loop";
constexpr char TCA002[] = "TCA002: C++ deallocation expression inside loop";
constexpr char TCA003[] = "TCA003: C allocation call inside loop";
constexpr char TCA004[] = "TCA004: C deallocation call inside loop";
constexpr char TCA005[] = "TCA005: expensive math function call inside loop";

constexpr char TCA001Note[] =
    "consider reusing storage outside the loop or using a stack/value object";
constexpr char TCA002Note[] =
    "consider moving ownership cleanup outside the loop or using RAII storage";
constexpr char TCA003Note[] =
    "consider allocating once before the loop and reusing the buffer";
constexpr char TCA004Note[] = "consider freeing loop-owned storage after the "
                              "loop when ownership allows it";
constexpr char TCA005Note[] =
    "consider hoisting loop-invariant math or using a cheaper recurrence";

bool isCAllocationFunction(const clang::FunctionDecl *Function) {
  if (!Function) {
    return false;
  }
  const std::string Name = Function->getQualifiedNameAsString();
  return Name == "malloc" || Name == "::malloc" || Name == "std::malloc" ||
         Name == "calloc" || Name == "::calloc" || Name == "std::calloc" ||
         Name == "realloc" || Name == "::realloc" || Name == "std::realloc";
}

bool isExpensiveMathFunction(const clang::FunctionDecl *Function) {
  if (!Function) {
    return false;
  }
  const std::string Name = Function->getQualifiedNameAsString();
  return Name == "pow" || Name == "::pow" || Name == "std::pow" ||
         Name == "sqrt" || Name == "::sqrt" || Name == "std::sqrt" ||
         Name == "sin" || Name == "::sin" || Name == "std::sin" ||
         Name == "cos" || Name == "::cos" || Name == "std::cos";
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

template <std::size_t WarningSize, std::size_t NoteSize>
void reportWarningWithNote(clang::ASTContext &Context,
                           clang::SourceLocation Location,
                           const char (&Warning)[WarningSize],
                           const char (&Note)[NoteSize]) {
  clang::DiagnosticsEngine &Diagnostics = Context.getDiagnostics();

  const unsigned WarningID =
      Diagnostics.getCustomDiagID(clang::DiagnosticsEngine::Warning, Warning);
  Diagnostics.Report(Location, WarningID);

  const unsigned NoteID =
      Diagnostics.getCustomDiagID(clang::DiagnosticsEngine::Note, Note);
  Diagnostics.Report(Location, NoteID);
}
} // namespace

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
  reportWarningWithNote(Context, Expression->getBeginLoc(), TCA001, TCA001Note);

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
  reportWarningWithNote(Context, Expression->getBeginLoc(), TCA002, TCA002Note);

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
  if (IsAllocation) {
    reportWarningWithNote(Context, Expression->getBeginLoc(), TCA003,
                          TCA003Note);
  } else if (IsDeallocation) {
    reportWarningWithNote(Context, Expression->getBeginLoc(), TCA004,
                          TCA004Note);
  } else {
    reportWarningWithNote(Context, Expression->getBeginLoc(), TCA005,
                          TCA005Note);
  }

  return true;
}

} // namespace tensorium_clang_audit
