#include "TensoriumClangAudit/Clang/Checks/VirtualCallInLoopCheck.hpp"

#include "clang/AST/Attr.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/ExprCXX.h"

namespace tensorium_clang_audit {

bool isVirtualDispatchCall(const clang::CXXMemberCallExpr *Expression) {
  if (!Expression) {
    return false;
  }

  const clang::CXXMethodDecl *Method = Expression->getMethodDecl();
  if (!Method || !Method->isVirtual()) {
    return false;
  }

  if (Method->hasAttr<clang::FinalAttr>()) {
    return false;
  }

  const clang::CXXRecordDecl *Parent = Method->getParent();
  if (Parent && Parent->hasAttr<clang::FinalAttr>()) {
    return false;
  }

  return true;
}

} // namespace tensorium_clang_audit
