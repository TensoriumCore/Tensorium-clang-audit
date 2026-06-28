#include "TensoriumClangAudit/Clang/Checks/IoInLoopCheck.hpp"

#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/OperationKinds.h"
#include "clang/AST/RecursiveASTVisitor.h"

#include <string>

namespace {
class StdStreamReferenceFinder
    : public clang::RecursiveASTVisitor<StdStreamReferenceFinder> {
public:
  bool VisitDeclRefExpr(clang::DeclRefExpr *Expression) {
    const clang::ValueDecl *Decl = Expression ? Expression->getDecl() : nullptr;
    if (!Decl) {
      return true;
    }

    const std::string Name = Decl->getQualifiedNameAsString();
    if (Name == "std::cout" || Name == "std::cerr" || Name == "std::clog" ||
        Name == "cout" || Name == "cerr" || Name == "clog") {
      Found = true;
    }

    return true;
  }

  bool found() const { return Found; }

private:
  bool Found = false;
};

bool isCIoFunction(const clang::FunctionDecl *Function) {
  if (!Function) {
    return false;
  }

  const std::string Name = Function->getQualifiedNameAsString();
  return Name == "printf" || Name == "::printf" || Name == "std::printf" ||
         Name == "fprintf" || Name == "::fprintf" || Name == "std::fprintf";
}
} // namespace

namespace tensorium_clang_audit {

bool isCIoCall(const clang::CallExpr *Expression) {
  return Expression && isCIoFunction(Expression->getDirectCallee());
}

bool isCxxStreamOutputCall(const clang::CXXOperatorCallExpr *Expression) {
  if (!Expression || Expression->getOperator() != clang::OO_LessLess) {
    return false;
  }

  for (const clang::Expr *Argument : Expression->arguments()) {
    if (!Argument) {
      continue;
    }

    StdStreamReferenceFinder Finder;
    Finder.TraverseStmt(const_cast<clang::Expr *>(Argument));
    if (Finder.found()) {
      return true;
    }
  }

  return false;
}

} // namespace tensorium_clang_audit
