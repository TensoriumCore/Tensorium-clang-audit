#pragma once

namespace clang {
class FunctionDecl;
class CallExpr;
class Stmt;
} // namespace clang

namespace tensorium_clang_audit {

bool isExpensiveMathFunction(const clang::FunctionDecl *Function);

bool isLoopInvariantMathCall(const clang::CallExpr *Expression,
                             const clang::Stmt *CurrentLoop);
} // namespace tensorium_clang_audit
