#include "TensoriumClangAudit/Clang/Support/SourceUtils.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"

namespace tensorium_clang_audit {

bool isInMainFile(const clang::ASTContext &Context,
                  clang::SourceLocation Location) {
  if (Location.isInvalid()) {
    return false;
  }

  const clang::SourceManager &SourceManager = Context.getSourceManager();
  const clang::SourceLocation ExpansionLocation =
      SourceManager.getExpansionLoc(Location);

  return SourceManager.getFileID(ExpansionLocation) ==
         SourceManager.getMainFileID();
}

} // namespace tensorium_clang_audit
