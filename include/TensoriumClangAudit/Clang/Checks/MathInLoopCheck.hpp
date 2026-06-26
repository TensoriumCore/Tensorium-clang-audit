#pragma once

namespace clang {
class FunctionDecl;
} // namespace clang

namespace tensorium_clang_audit {

bool isExpensiveMathFunction(const clang::FunctionDecl *Function);

} // namespace tensorium_clang_audit
