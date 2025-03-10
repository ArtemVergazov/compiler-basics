#pragma once

#include <cstdlib> // size_t
#include <optional>
#include <utility> // std::move
#include <variant>
#include <vector>

#include "arena_allocator.h"
#include "error.h"
#include "text_reader.h"
#include "tokenizer.h"

constexpr int FOUR_MEGABYTES{ 4 * 1024 * 1024 };

struct NodeExpr;
struct NodeStmt;

struct NodeTermIntLiteral {
    Token intLiteral{};
};

struct NodeTermIdentifier {
    Token identifier{};
};

struct NodeTermParen {
    const NodeExpr *expr{};
};

struct NodeTerm {
    std::variant<
        const NodeTermIntLiteral *,
        const NodeTermIdentifier *,
        const NodeTermParen *
    > term{};
};

struct NodeBinExprAdd {
    const NodeExpr *lhs{};
    const NodeExpr *rhs{};
};

struct NodeBinExprSub {
    const NodeExpr *lhs{};
    const NodeExpr *rhs{};
};

struct NodeBinExprMul {
    const NodeExpr *lhs{};
    const NodeExpr *rhs{};
};

struct NodeBinExprDiv {
    const NodeExpr *lhs{};
    const NodeExpr *rhs{};
};

struct NodeBinExpr {
    std::variant<
        const NodeBinExprAdd *,
        const NodeBinExprSub *,
        const NodeBinExprMul *,
        const NodeBinExprDiv *
    > expr{};
};

struct NodeExpr {
    std::variant<const NodeTerm *, const NodeBinExpr *> expr{};
};

struct NodeScope {
    std::vector<const NodeStmt *>stmts{};
};

struct NodeStmtExit {
    const NodeExpr *expr{};
};

struct NodeStmtLet {
    Token identifier{};
    const NodeExpr *expr{};
};

struct NodeStmtAssign {
    Token identifier{};
    const NodeExpr *expr{};
};

struct NodeBranchIf {
    const NodeExpr *expr{};
    const NodeScope *scope{};
};

struct NodeBranchElif {
    const NodeExpr *expr{};
    const NodeScope *scope{};
};

struct NodeBranchElse {
    const NodeScope *scope{};
};

struct NodeStmtIf {
    const NodeBranchIf *ifBranch{};
    std::vector<const NodeBranchElif *>elifBranches{};
    const NodeBranchElse *elseBranch{};
};

struct NodeStmt {
    std::variant<
        const NodeStmtExit *,
        const NodeStmtLet *,
        const NodeStmtAssign *,
        const NodeStmtIf *,
        const NodeScope *
    > stmt{};
};

struct NodeProg {
    std::vector<const NodeStmt *> stmts{};
};

class Parser : public TextReader<std::vector<Token>> {
public:
    Parser() = delete;
    Parser(const Parser &) = delete;
    Parser &operator=(const Parser &) = delete;

    explicit Parser(std::vector<Token> &&tokens) : TextReader{ std::move(tokens) } {}

    const NodeTerm *parseTerm() {
        if (const std::optional<Token> intLiteral{ tryConsume(TokenType::INT_LITERAL) }) {
            const NodeTermIntLiteral *const intLiteralTerm{ mAllocator.emplace<NodeTermIntLiteral>(*intLiteral) };
            const NodeTerm *const term{ mAllocator.emplace<NodeTerm>(intLiteralTerm) };
            return term;
        }

        if (const std::optional<Token> identifier{ tryConsume(TokenType::IDENTIFIER) }) {
            const NodeTermIdentifier *const identifierTerm{ mAllocator.emplace<NodeTermIdentifier>(*identifier) };
            const NodeTerm *const term{ mAllocator.emplace<NodeTerm>(identifierTerm) };

            return term;
        }

        if (tryConsume(TokenType::OPEN_PAREN)) {
            const NodeExpr *const expr{ parseExpr() };
            if (!expr) {
                errorExpected("expression");
            } else {
                forceConsume(TokenType::CLOSE_PAREN);
            }

            const NodeTermParen *const parenTerm{ mAllocator.emplace<NodeTermParen>(expr) };
            const NodeTerm *const term{ mAllocator.emplace<NodeTerm>(parenTerm) };

            return term;
        }

        return nullptr;
    }

    const NodeExpr *parseExpr(int minPrec = 0) {
        const NodeTerm *const term{ parseTerm() };
        if (!term) {
            return nullptr;
        }

        const NodeExpr *lhsExpr{ mAllocator.emplace<NodeExpr>(term) };

        while (true) {
            std::optional<int> prec{};
            if (!peek() || !(prec = binPrec(peek()->type)) || prec < minPrec) {
                break;
            }

            const TokenType binType{ consume().type };

            NodeBinExpr *const binExpr{ mAllocator.emplace<NodeBinExpr>() };
            const NodeExpr *const rhsExpr{ parseExpr(*prec + 1) };
            if (!rhsExpr) {
                errorExpected("expression");
            }

            if (binType == TokenType::PLUS) {
                const NodeBinExprAdd *const addExpr{ mAllocator.emplace<NodeBinExprAdd>(lhsExpr, rhsExpr) };
                binExpr->expr = addExpr;
            } else if (binType == TokenType::STAR) {
                const NodeBinExprMul *const mulExpr{ mAllocator.emplace<NodeBinExprMul>(lhsExpr, rhsExpr) };
                binExpr->expr = mulExpr;
            } else if (binType == TokenType::MINUS) {
                const NodeBinExprSub *const subExpr{ mAllocator.emplace<NodeBinExprSub>(lhsExpr, rhsExpr) };
                binExpr->expr = subExpr;
            } else if (binType == TokenType::FSLASH) {
                const NodeBinExprDiv *const divExpr{ mAllocator.emplace<NodeBinExprDiv>(lhsExpr, rhsExpr) };
                binExpr->expr = divExpr;
            } else {
                errorExpected("a binary operator");
            }
            lhsExpr = mAllocator.emplace<NodeExpr>(binExpr);
        }

        return lhsExpr;
    }

    const NodeScope *parseScope() {
        if (!tryConsume(TokenType::OPEN_CURLY)) {
            return nullptr;
        }

        NodeScope *const scope{ mAllocator.emplace<NodeScope>() };
        for (const NodeStmt *innerStmt{ parseStmt() }; ; innerStmt = parseStmt()) {
            if (innerStmt) {
                scope->stmts.push_back(innerStmt);
            } else {
                break;
            }
        }
        forceConsume(TokenType::CLOSE_CURLY);

        return scope;
    }

    const NodeStmt *parseStmt() {
        if (tryConsume(TokenType::EXIT) && tryConsume(TokenType::OPEN_PAREN)) {

            const NodeExpr *const expr{ parseExpr() };
            if (!expr) {
                errorExpected("expression");
            }

            forceConsume(TokenType::CLOSE_PAREN);
            forceConsume(TokenType::SEMI);
            
            const NodeStmtExit *const exitStmt{ mAllocator.emplace<NodeStmtExit>(expr) };
            const NodeStmt *const stmt{ mAllocator.emplace<NodeStmt>(exitStmt) };

            return stmt;
        }

        if (
            std::optional<Token> identifier{};
            tryConsume(TokenType::LET) &&
            (identifier = tryConsume(TokenType::IDENTIFIER)) &&
            tryConsume(TokenType::EQ)
        ) {
            const NodeExpr *const expr{ parseExpr() };
            if (!expr) {
                errorExpected("expression");
            }
            forceConsume(TokenType::SEMI);

            const NodeStmtLet *const letStmt{ mAllocator.emplace<NodeStmtLet>(*identifier, expr) };
            const NodeStmt *const stmt{ mAllocator.emplace<NodeStmt>(letStmt) };

            return stmt;
        }

        if (
            std::optional<Token> identifier{ tryConsume(TokenType::IDENTIFIER) };
            identifier && tryConsume(TokenType::EQ)
        ) {
            const NodeExpr *const expr{ parseExpr() };
            if (!expr) {
                errorExpected("expression");
            }
            forceConsume(TokenType::SEMI);
            const NodeStmtAssign *const assignStmt{ mAllocator.emplace<NodeStmtAssign>(*identifier, expr) };
            const NodeStmt *const stmt{ mAllocator.emplace<NodeStmt>(assignStmt) };

            return stmt;
        }

        if (tryConsume(TokenType::IF)) { // if branch
            NodeStmtIf *const ifStmt{ mAllocator.emplace<NodeStmtIf>() };

            forceConsume(TokenType::OPEN_PAREN);

            const NodeExpr *const expr{ parseExpr() };
            if (!expr) {
                errorExpected("expression");
            }

            forceConsume(TokenType::CLOSE_PAREN);

            const NodeScope *const scope{ parseScope() };
            if (!scope) {
                errorExpected("scope");
            }

            const NodeBranchIf *const ifBranch{ mAllocator.emplace<NodeBranchIf>(expr, scope) };
            ifStmt->ifBranch = ifBranch;

            while (tryConsume(TokenType::ELIF)) { // elif branches
                forceConsume(TokenType::OPEN_PAREN);

                const NodeExpr *const expr{ parseExpr() };
                if (!expr) {
                    errorExpected("expression");
                }
    
                forceConsume(TokenType::CLOSE_PAREN);
    
                const NodeScope *const scope{ parseScope() };
                if (!scope) {
                    errorExpected("scope");
                }

                const NodeBranchElif *const elifBranch{ mAllocator.emplace<NodeBranchElif>(expr, scope) };
                ifStmt->elifBranches.push_back(elifBranch);
            }

            if (tryConsume(TokenType::ELSE)) { // else branch
                const NodeScope *const scope{ parseScope() };
                if (!scope) {
                    errorExpected("scope");
                }
                const NodeBranchElse *const elseBranch{ mAllocator.emplace<NodeBranchElse>(scope) };
                ifStmt->elseBranch = elseBranch;
            }

            const NodeStmt *const stmt{ mAllocator.emplace<NodeStmt>(ifStmt) };

            return stmt;
        }

        if (peek() && peek()->type == TokenType::OPEN_CURLY) {
            const NodeScope *const scope{ parseScope() };
            if (!scope) {
                errorExpected("scope");
            }
            const NodeStmt *const stmt = mAllocator.emplace<NodeStmt>(scope);

            return stmt;
        }

        return nullptr;
    }

    const NodeProg *parseProg() {
        NodeProg *const prog{ mAllocator.emplace<NodeProg>() };

        while (peek()) {
            const NodeStmt *const stmt{ parseStmt() };
            if (!stmt) {
                errorExpected("statement");
            }
            prog->stmts.push_back(stmt);
        }
        mIndex = 0;

        return prog;
    }

private:
    void errorExpected(const std::string &msg) {
        const int line = peek() ? peek()->ln : peek(-1)->ln;
        error("[Parse Error] Expected " + msg + " at line " + std::to_string(line));
    }

    std::optional<Token> tryConsume(TokenType type) {
        if (peek() && peek()->type == type) {
            return consume();
        }

        return {};
    }

    Token forceConsume(TokenType type) {
        if (std::optional<Token> tok{ tryConsume(type) }) {
            return *tok;
        }
        errorExpected(to_string(type));

        return {}; // unreachable
    }

    ArenaAllocator mAllocator{ FOUR_MEGABYTES };
};
