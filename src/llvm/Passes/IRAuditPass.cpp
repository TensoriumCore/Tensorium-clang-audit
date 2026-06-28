#include "TensoriumClangAudit/LLVM/Passes/IRAuditPass.hpp"

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

namespace tensorium_clang_audit::llvm_ir {

llvm::PreservedAnalyses
IRAuditPass::run(llvm::Module &Module,
                 llvm::ModuleAnalysisManager &AnalysisManager) {
  (void)AnalysisManager;

  llvm::errs() << "[tensorium-llvm-ir-audit] visiting module: "
               << Module.getName() << "\n";

  for (const llvm::Function &Function : Module) {
    if (Function.isDeclaration()) {
      continue;
    }

    llvm::errs() << "[tensorium-llvm-ir-audit] function: "
                 << Function.getName() << "\n";
  }

  return llvm::PreservedAnalyses::all();
}

} // namespace tensorium_clang_audit::llvm_ir
