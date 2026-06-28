#include "TensoriumClangAudit/Clang/Checks/StlInLoopCheck.hpp"

#include "clang/AST/DeclCXX.h"
#include "clang/AST/ExprCXX.h"

#include <string>

namespace {
bool isStdVectorLikeClass(const clang::CXXRecordDecl *Record) {
  if (!Record) {
    return false;
  }

  const std::string Name = Record->getQualifiedNameAsString();
  return Name == "std::vector" || Name.rfind("std::vector<", 0) == 0 ||
         Name.find("::vector") != std::string::npos;
}
} // namespace

namespace tensorium_clang_audit {

bool isStlGrowthCall(const clang::CXXMemberCallExpr *Expression) {
  if (!Expression) {
    return false;
  }

  const clang::CXXMethodDecl *Method = Expression->getMethodDecl();
  if (!Method) {
    return false;
  }

  const std::string MethodName = Method->getNameAsString();
  if (MethodName != "push_back" && MethodName != "emplace_back") {
    return false;
  }

  return isStdVectorLikeClass(Method->getParent());
}

} // namespace tensorium_clang_audit
