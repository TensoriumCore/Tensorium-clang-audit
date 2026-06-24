#include "TensoriumClangAudit/Plugin.hpp"

#include "TensoriumClangAudit/Visitor.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/Support/raw_ostream.h"

namespace tensorium_clang_audit {

TensoriumClangAuditConsumer::TensoriumClangAuditConsumer(
    clang::ASTContext &Context)
    : Context(Context) {}

void TensoriumClangAuditConsumer::HandleTranslationUnit(
    clang::ASTContext &Context) {
  llvm::errs() << "TensoriumClangAudit: visiting translation unit\n";

  TensoriumClangAuditVisitor Visitor(Context);
  Visitor.TraverseDecl(Context.getTranslationUnitDecl());
}

std::unique_ptr<clang::ASTConsumer>
TensoriumClangAuditAction::CreateASTConsumer(clang::CompilerInstance &Compiler,
                                             llvm::StringRef InFile) {
  (void)InFile;
  llvm::errs() << "TensoriumClangAudit: plugin loaded\n";
  return std::make_unique<TensoriumClangAuditConsumer>(
      Compiler.getASTContext());
}

bool TensoriumClangAuditAction::ParseArgs(
    const clang::CompilerInstance &Compiler,
    const std::vector<std::string> &Args) {
  (void)Compiler;
  (void)Args;
  return true;
}

} // namespace tensorium_clang_audit

static clang::FrontendPluginRegistry::Add<
    tensorium_clang_audit::TensoriumClangAuditAction>
    X("tensorium-clang-audit",
      "minimal Tensorium Clang audit frontend plugin");
