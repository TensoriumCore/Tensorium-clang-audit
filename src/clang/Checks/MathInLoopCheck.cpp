#include "TensoriumClangAudit/Clang/Checks/MathInLoopCheck.hpp"

#include "clang/AST/Decl.h"

#include <string>

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

} // namespace tensorium_clang_audit
