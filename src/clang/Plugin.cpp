#include "TensoriumClangAudit/Clang/Plugin.hpp"

#include "TensoriumClangAudit/Clang/Options.hpp"
#include "TensoriumClangAudit/Clang/Visitor.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/ADT/SmallVector.h"

#include <string>

namespace {
llvm::StringRef normalizePluginArg(llvm::StringRef Arg) {
  while (Arg.consume_front("-")) {
  }
  return Arg;
}

bool applyLoopAnalyseOption(
    llvm::StringRef Value,
    tensorium_clang_audit::LoopAnalysisOptions &Options) {
  llvm::SmallVector<llvm::StringRef, 4> Values;
  Value.split(Values, ",");

  if (Values.empty()) {
    return false;
  }

  tensorium_clang_audit::LoopAnalysisOptions ParsedOptions;
  ParsedOptions.Alloc = false;
  ParsedOptions.Maths = false;
  ParsedOptions.Stl = false;
  ParsedOptions.Io = false;
  ParsedOptions.VirtualCalls = false;
  bool SawKnownValue = false;
  for (llvm::StringRef Entry : Values) {
    Entry = Entry.trim();

    if (Entry == "all") {
      ParsedOptions.Alloc = true;
      ParsedOptions.Maths = true;
      ParsedOptions.Stl = true;
      ParsedOptions.Io = true;
      ParsedOptions.VirtualCalls = true;
      SawKnownValue = true;
      continue;
    }

    if (Entry == "none") {
      ParsedOptions.Alloc = false;
      ParsedOptions.Maths = false;
      ParsedOptions.Stl = false;
      ParsedOptions.Io = false;
      ParsedOptions.VirtualCalls = false;
      SawKnownValue = true;
      continue;
    }

    if (Entry == "alloc" || Entry == "allocation" || Entry == "alloc-in-loop") {
      ParsedOptions.Alloc = true;
      SawKnownValue = true;
      continue;
    }

    if (Entry == "maths" || Entry == "math" || Entry == "math-in-loop") {
      ParsedOptions.Maths = true;
      SawKnownValue = true;
      continue;
    }

    if (Entry == "stl" || Entry == "stl-growth") {
      ParsedOptions.Stl = true;
      SawKnownValue = true;
      continue;
    }

    if (Entry == "io" || Entry == "i/o") {
      ParsedOptions.Io = true;
      SawKnownValue = true;
      continue;
    }

    if (Entry == "virtual" || Entry == "virtual-call" ||
        Entry == "virtual-calls" || Entry == "dispatch") {
      ParsedOptions.VirtualCalls = true;
      SawKnownValue = true;
      continue;
    }

    return false;
  }

  if (SawKnownValue) {
    Options = ParsedOptions;
  }
  return SawKnownValue;
}
} // namespace

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
    const llvm::StringRef NormalizedArg = normalizePluginArg(Arg);

    if (NormalizedArg == "quiet") {
      Options.Quiet = true;
      continue;
    }

    llvm::StringRef LoopAnalyseArg = NormalizedArg;
    if (LoopAnalyseArg.consume_front("loop-analyse=")) {
      if (applyLoopAnalyseOption(LoopAnalyseArg, Options.LoopAnalysis)) {
        continue;
      }
    }

    llvm::StringRef ChecksArg = NormalizedArg;
    if (ChecksArg.consume_front("checks=")) {
      if (applyLoopAnalyseOption(ChecksArg, Options.LoopAnalysis)) {
        continue;
      }
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
