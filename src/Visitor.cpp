#include "TensoriumClangAudit/Visitor.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
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

} // namespace tensorium_clang_audit
