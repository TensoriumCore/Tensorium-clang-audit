#include "TensoriumClangAudit/Plugin.hpp"

#include "TensoriumClangAudit/Options.hpp"
#include "TensoriumClangAudit/Visitor.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Frontend/CompilerInstance.h"

#include <string>

namespace tensorium_clang_audit {

TensoriumClangAuditConsumer::TensoriumClangAuditConsumer(
    clang::ASTContext &Context, TensoriumClangAuditOptions Options)
    : Context(Context), Options(Options) {}

void TensoriumClangAuditConsumer::HandleTranslationUnit(
    clang::ASTContext &Context) {
  TensoriumClangAuditVisitor Visitor(Context, Options);
  Visitor.TraverseDecl(Context.getTranslationUnitDecl());
}

std::unique_ptr<clang::ASTConsumer>
TensoriumClangAuditAction::CreateASTConsumer(clang::CompilerInstance &Compiler,
                                             llvm::StringRef InFile) {
  (void)InFile;
  return std::make_unique<TensoriumClangAuditConsumer>(Compiler.getASTContext(),
                                                       Options);
}

bool TensoriumClangAuditAction::ParseArgs(
    const clang::CompilerInstance &Compiler,
    const std::vector<std::string> &Args) {
  for (const std::string &Arg : Args) {
    llvm::StringRef NormalizedArg(Arg);
    NormalizedArg.consume_front("-");

    if (NormalizedArg == "quiet") {
      Options.Quiet = true;
      continue;
    }

    if (NormalizedArg == "checks=all") {
      Options.CheckAllocInLoop = true;
      Options.CheckMathInLoop = true;
      continue;
    }

    if (NormalizedArg == "checks=alloc-in-loop") {
      Options.CheckAllocInLoop = true;
      Options.CheckMathInLoop = false;
      continue;
    }

    if (NormalizedArg == "checks=math-in-loop") {
      Options.CheckAllocInLoop = false;
      Options.CheckMathInLoop = true;
      continue;
    }

    if (NormalizedArg == "checks=none") {
      Options.CheckAllocInLoop = false;
      Options.CheckMathInLoop = false;
      continue;
    }

    clang::DiagnosticsEngine &Diagnostics = Compiler.getDiagnostics();
    const unsigned DiagnosticID = Diagnostics.getCustomDiagID(
        clang::DiagnosticsEngine::Error,
        "unknown tensorium-clang-audit plugin argument '%0'");
    Diagnostics.Report(DiagnosticID) << Arg;
    return false;
  }

  return true;
}

} // namespace tensorium_clang_audit

static clang::FrontendPluginRegistry::Add<
    tensorium_clang_audit::TensoriumClangAuditAction>
    X("tensorium-clang-audit", "minimal Tensorium Clang audit frontend plugin");
