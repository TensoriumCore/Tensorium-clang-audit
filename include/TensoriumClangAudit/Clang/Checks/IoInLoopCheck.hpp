#pragma once

namespace clang {
class CallExpr;
class CXXOperatorCallExpr;
} // namespace clang

namespace tensorium_clang_audit {

bool isCIoCall(const clang::CallExpr *Expression);
bool isCxxStreamOutputCall(const clang::CXXOperatorCallExpr *Expression);

} // namespace tensorium_clang_audit
