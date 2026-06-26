#include "TensoriumClangAudit/Clang/Checks/MathInLoopCheck.hpp"

#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/Support/Casting.h"

#include <string>

namespace {
class LoopLocalCollector
    : public clang::RecursiveASTVisitor<LoopLocalCollector> {
public:
  explicit LoopLocalCollector(llvm::DenseSet<const clang::VarDecl *> &Variables)
      : Variables(Variables) {}

  bool VisitVarDecl(clang::VarDecl *Variable) {
    if (Variable && Variable->hasLocalStorage()) {
      Variables.insert(Variable->getCanonicalDecl());
    }
    return true;
  }

private:
  llvm::DenseSet<const clang::VarDecl *> &Variables;
};

class LoopLocalReferenceFinder
    : public clang::RecursiveASTVisitor<LoopLocalReferenceFinder> {
public:
  explicit LoopLocalReferenceFinder(
      const llvm::DenseSet<const clang::VarDecl *> &LoopLocalVariables)
      : LoopLocalVariables(LoopLocalVariables) {}

  bool VisitDeclRefExpr(clang::DeclRefExpr *Expression) {
    const auto *Variable =
        llvm::dyn_cast_or_null<clang::VarDecl>(Expression->getDecl());

    if (!Variable) {
      return true;
    }

    if (LoopLocalVariables.contains(Variable->getCanonicalDecl())) {
      Found = true;
    }

    return true;
  }

  bool found() const { return Found; }

private:
  const llvm::DenseSet<const clang::VarDecl *> &LoopLocalVariables;
  bool Found = false;
};
} // namespace

namespace tensorium_clang_audit {

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

bool isLoopInvariantMathCall(const clang::CallExpr *Expression,
                             const clang::Stmt *CurrentLoop) {
  if (!Expression || !CurrentLoop) {
    return false;
  }

  llvm::DenseSet<const clang::VarDecl *> LoopLocalVariables;
  LoopLocalCollector Collector(LoopLocalVariables);
  Collector.TraverseStmt(const_cast<clang::Stmt *>(CurrentLoop));

  for (const clang::Expr *Argument : Expression->arguments()) {
    if (!Argument) {
      continue;
    }

    LoopLocalReferenceFinder Finder(LoopLocalVariables);
    Finder.TraverseStmt(const_cast<clang::Expr *>(Argument));

    if (Finder.found()) {
      return false;
    }
  }

  return true;
}
} // namespace tensorium_clang_audit
