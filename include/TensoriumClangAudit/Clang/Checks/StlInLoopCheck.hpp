#pragma once

namespace clang {
class CXXMemberCallExpr;
} // namespace clang

namespace tensorium_clang_audit {

bool isStlGrowthCall(const clang::CXXMemberCallExpr *Expression);

} // namespace tensorium_clang_audit
