#pragma once

#include <cstdlib> // size_t
#include <iostream>
#include <optional>
#include <utility> // std::move
#include <variant>
#include <vector>

#include "arena_allocator.h"
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

// TODO use `using` instead of `struct`
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

// TODO use `using` instead of `struct`
struct NodeBinExpr {
    std::variant<
        const NodeBinExprAdd *,
        const NodeBinExprSub *,
        const NodeBinExprMul *,
        const NodeBinExprDiv *
    > expr{};
};

// TODO use `using` instead of `struct`
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

// TODO use `using` instead of `struct`
struct NodeStmt {
    std::variant<
        const NodeStmtExit *,
        const NodeStmtLet *,
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

    void parseError(const std::string &str) {
        std::cerr << str << std::endl;
        exit(1);
    }

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
                parseError("Expected expression");
            } else {
                tryConsume(TokenType::CLOSE_PAREN, "Unmatched '('");
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
            if (
                const std::optional<Token> curToken{ peek() };
                !curToken ||
                !(prec = binPrec(curToken->type)) ||
                prec < minPrec
            ) {
                break;
            }

            const TokenType binType { consume().type };

            NodeBinExpr *const binExpr{ mAllocator.emplace<NodeBinExpr>() };
            const NodeExpr *const rhsExpr{ parseExpr(*prec + 1) };

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
                std::cerr << "Expected a binary operator\n";
                exit(1);
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
        tryConsume(TokenType::CLOSE_CURLY, "Expected '}'");

        return scope;
    }

    const NodeStmt *parseStmt() {
        if (
            tryConsume(TokenType::EXIT) &&
            tryConsume(TokenType::OPEN_PAREN)
        ) {
            const NodeExpr *const expr{ parseExpr() };
            if (!expr) {
                std::cerr << "Invalid expression\n";
                exit(1);
            }
            
            tryConsume(TokenType::CLOSE_PAREN, "Expected ')'");
            tryConsume(TokenType::SEMI, "Expected ';'");
            
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
                std::cerr << "Invalid expression\n";
                exit(1);
            }
            tryConsume(TokenType::SEMI, "Expected ';'");

            const NodeStmtLet *const letStmt{ mAllocator.emplace<NodeStmtLet>(*identifier, expr) };
            const NodeStmt *const stmt{ mAllocator.emplace<NodeStmt>(letStmt) };

            return stmt;
        }

        if (tryConsume(TokenType::IF)) { // if branch
            NodeStmtIf *const ifStmt{ mAllocator.emplace<NodeStmtIf>() };

            tryConsume(TokenType::OPEN_PAREN, "Expected '('");

            const NodeExpr *const expr{ parseExpr() };
            if (!expr) {
                std::cerr << "Invalid expression\n";
                exit(1);
            }

            tryConsume(TokenType::CLOSE_PAREN, "Expected ')'");

            const NodeScope *const scope{ parseScope() };
            if (!scope) {
                std::cerr << "Invalid scope\n";
                exit(1);
            }

            const NodeBranchIf *const ifBranch{ mAllocator.emplace<NodeBranchIf>(expr, scope) };
            ifStmt->ifBranch = ifBranch;

            while (tryConsume(TokenType::ELIF)) { // elif branches
                tryConsume(TokenType::OPEN_PAREN, "Expected '('");

                const NodeExpr *const expr{ parseExpr() };
                if (!expr) {
                    std::cerr << "Invalid expression\n";
                    exit(1);
                }
    
                tryConsume(TokenType::CLOSE_PAREN, "Expected ')'");
    
                const NodeScope *const scope{ parseScope() };
                if (!scope) {
                    std::cerr << "Invalid scope\n";
                    exit(1);
                }

                const NodeBranchElif *const elifBranch{ mAllocator.emplace<NodeBranchElif>(expr, scope) };
                ifStmt->elifBranches.push_back(elifBranch);
            }

            if (tryConsume(TokenType::ELSE)) { // else branch
                const NodeScope *const scope{ parseScope() };
                if (!scope) {
                    std::cerr << "Invalid scope\n";
                    exit(1);
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
                std::cerr << "Invalid scope\n";
                exit(1);
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
                std::cerr << "Invalid statement\n";
                exit(1);
            }
            prog->stmts.push_back(stmt);
        }
        mIndex = 0;

        return prog;
    }

private:
    std::optional<Token> tryConsume(TokenType type) {
        if (peek() && peek()->type == type) {
            return consume();
        }

        return {};
    }

    Token tryConsume(TokenType type, const std::string &errMsg) {
        if (peek() && peek()->type == type) {
            return consume();
        }
        std::cerr << errMsg << std::endl;
        exit(1);
    }

    ArenaAllocator mAllocator{FOUR_MEGABYTES};
};
