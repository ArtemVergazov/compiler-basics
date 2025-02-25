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

constexpr int FOUR_MEGABYTES = 4 * 1024 * 1024;

struct NodeTermIntLiteral {
    Token intLiteral;
};

struct NodeTermIdentifier {
    Token identifier;
};

struct NodeTerm {
    std::variant<NodeTermIntLiteral *, NodeTermIdentifier *> term;
};

struct NodeExpr;

struct NodeBinExprAdd {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

// struct NodeBinExprMul {
//     NodeExpr *lhs;
//     NodeExpr *rhs;
// };

struct NodeBinExpr {
    // std::variant<NodeBinExprAdd *, NodeBinExprMul *> expr;
    NodeBinExprAdd *expr;
};

struct NodeExpr {
    std::variant<NodeTerm *, NodeBinExpr *> expr;
};

struct NodeStmtExit {
    NodeExpr *expr;
};

struct NodeStmtLet {
    Token identifier;
    NodeExpr *expr;
};

struct NodeStmt {
    std::variant<NodeStmtExit *, NodeStmtLet *> stmt;
};

struct NodeProg {
    std::vector<NodeStmt *> stmts{};
};

class Parser : public TextReader<std::vector<Token>> {
public:
    explicit Parser(std::vector<Token> &&tokens) :
        TextReader(std::move(tokens)), mAllocator(FOUR_MEGABYTES) {}

    NodeTerm *parseTerm() {
        if (auto intLiteral = tryConsume(TokenType::INT_LITERAL)) {

            NodeTermIntLiteral *intLiteralTerm = mAllocator.alloc<NodeTermIntLiteral>();
            intLiteralTerm->intLiteral = intLiteral.value();
            NodeTerm *term = mAllocator.alloc<NodeTerm>();
            term->term = intLiteralTerm;

            return term;

        } else if (auto indentifier = tryConsume(TokenType::IDENTIFIER)) {

            NodeTermIdentifier *identifierTerm = mAllocator.alloc<NodeTermIdentifier>();
            identifierTerm->identifier = indentifier.value();
            NodeTerm *term = mAllocator.alloc<NodeTerm>();
            term->term = identifierTerm;

            return term;
        }

        return nullptr;
    }

    NodeExpr *parseExpr() {
        NodeTerm *term = parseTerm();
        if (!term) {
            return nullptr;
        }

        if (tryConsume(TokenType::PLUS)) {
            NodeBinExpr *binExpr = mAllocator.alloc<NodeBinExpr>();
            NodeBinExprAdd *addExpr = mAllocator.alloc<NodeBinExprAdd>();
            NodeExpr *lhs = mAllocator.alloc<NodeExpr>();
            lhs->expr = term;
            addExpr->lhs = lhs;

            if (NodeExpr *rhs = parseExpr()) {
                addExpr->rhs = rhs;
                binExpr->expr = addExpr;
                NodeExpr *expr = mAllocator.alloc<NodeExpr>();
                expr->expr = binExpr;

                return expr;

            } else {
                std::cerr << "Expected expression\n";
                exit(1);
            }
        }

        NodeExpr *expr = mAllocator.alloc<NodeExpr>();
        expr->expr = term;

        return expr;
    }

    NodeStmt *parseStmt() {
        if (
            tryConsume(TokenType::EXIT) &&
            tryConsume(TokenType::OPEN_PAREN)
        ) {
            NodeExpr *expr = parseExpr();
            if (!expr) {
                std::cerr << "Invalid expression\n";
                exit(1);
            }
            NodeStmtExit *exitStmt = mAllocator.alloc<NodeStmtExit>();
            exitStmt->expr = expr;

            tryConsume(TokenType::CLOSE_PAREN, "Expected ')'");
            tryConsume(TokenType::SEMI, "Expected ';'");

            NodeStmt *stmt = mAllocator.alloc<NodeStmt>();
            stmt->stmt = exitStmt;

            return stmt;

        } else if (
            std::optional<Token> identifier;
            tryConsume(TokenType::LET) &&
            (identifier = tryConsume(TokenType::IDENTIFIER)) &&
            tryConsume(TokenType::EQ)
        ) {
            NodeStmtLet *letStmt = mAllocator.alloc<NodeStmtLet>();
            letStmt->identifier = identifier.value();

            NodeExpr *expr = parseExpr();
            if (!expr) {
                std::cerr << "Invalid expression\n";
                exit(1);
            }
            letStmt->expr = expr;

            tryConsume(TokenType::SEMI, "Expected ';'");

            NodeStmt *stmt = mAllocator.alloc<NodeStmt>();
            stmt->stmt = letStmt;

            return stmt;
        }

        return nullptr;
    }

    NodeProg parseProg() {
        NodeProg prog;

        while (peek().has_value()) {
            NodeStmt *stmt = parseStmt();
            if (!stmt) {
                std::cerr << "Invalid statement\n";
                exit(1);
            }
            prog.stmts.push_back(stmt);
        }
        mIndex = 0;

        return prog;
    }

private:
    std::optional<Token> tryConsume(TokenType type) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }

        return {};
    }

    Token tryConsume(TokenType type, const std::string &errMsg) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        std::cerr << errMsg << std::endl;
        exit(1);
    }

    ArenaAllocator mAllocator;
};
