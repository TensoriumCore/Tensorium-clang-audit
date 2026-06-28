#include "TensoriumClangAudit/LLVM/Plugin.hpp"

#include "TensoriumClangAudit/LLVM/Passes/IRAuditPass.hpp"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

namespace {
constexpr llvm::StringLiteral PluginName = "tensorium-llvm-ir-audit";
constexpr llvm::StringLiteral PassName = "tensorium-ir-audit";
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, PluginName.data(), LLVM_VERSION_STRING,
          [](llvm::PassBuilder &PassBuilder) {
            PassBuilder.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::ModulePassManager &PassManager,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                  if (Name != PassName) {
                    return false;
                  }

                  PassManager.addPass(
                      tensorium_clang_audit::llvm_ir::IRAuditPass());
                  return true;
                });
          }};
}
