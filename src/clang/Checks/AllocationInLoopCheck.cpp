#include "TensoriumClangAudit/Clang/Checks/AllocationInLoopCheck.hpp"

#include "clang/AST/Decl.h"

#include <string>

namespace tensorium_clang_audit {

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

} // namespace tensorium_clang_audit
