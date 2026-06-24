#pragma once

#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/ADT/StringRef.h"

#include <memory>
#include <string>
#include <vector>

namespace clang {
class ASTContext;
class CompilerInstance;
} // namespace clang

namespace tensorium_clang_audit {

class TensoriumClangAuditConsumer final : public clang::ASTConsumer {
public:
  explicit TensoriumClangAuditConsumer(clang::ASTContext &Context);

  void HandleTranslationUnit(clang::ASTContext &Context) override;

private:
  clang::ASTContext &Context;
};

class TensoriumClangAuditAction final : public clang::PluginASTAction {
protected:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler,
                    llvm::StringRef InFile) override;

  bool ParseArgs(const clang::CompilerInstance &Compiler,
                 const std::vector<std::string> &Args) override;
};

} // namespace tensorium_clang_audit
