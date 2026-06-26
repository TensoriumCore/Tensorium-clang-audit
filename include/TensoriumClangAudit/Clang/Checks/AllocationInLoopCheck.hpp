#pragma once

namespace clang {
class FunctionDecl;
} // namespace clang

namespace tensorium_clang_audit {

bool isCAllocationFunction(const clang::FunctionDecl *Function);
bool isCDeallocationFunction(const clang::FunctionDecl *Function);

} // namespace tensorium_clang_audit
