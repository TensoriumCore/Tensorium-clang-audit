#pragma once

#include "llvm/IR/PassManager.h"

namespace llvm {
class Module;
} // namespace llvm

namespace tensorium_clang_audit::llvm_ir {

class IRAuditPass final : public llvm::PassInfoMixin<IRAuditPass> {
public:
  llvm::PreservedAnalyses run(llvm::Module &Module,
                              llvm::ModuleAnalysisManager &AnalysisManager);
};

} // namespace tensorium_clang_audit::llvm_ir
