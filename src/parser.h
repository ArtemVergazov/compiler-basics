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

struct NodeExpr;

struct NodeTermIntLiteral {
    Token intLiteral;
};

struct NodeTermIdentifier {
    Token identifier;
};

struct NodeTermParen {
    NodeExpr *expr;
};

struct NodeTerm {
    std::variant<NodeTermIntLiteral *, NodeTermIdentifier *, NodeTermParen *> term;
};

struct NodeBinExprAdd {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprSub {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprMul {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprDiv {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExpr {
    std::variant<
        NodeBinExprAdd *,
        NodeBinExprSub *,
        NodeBinExprMul *,
        NodeBinExprDiv *
    > expr;
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
        if (std::optional<Token> intLiteral = tryConsume(TokenType::INT_LITERAL)) {

            NodeTermIntLiteral *intLiteralTerm = mAllocator.alloc<NodeTermIntLiteral>();
            intLiteralTerm->intLiteral = *intLiteral;
            NodeTerm *term = mAllocator.alloc<NodeTerm>();
            term->term = intLiteralTerm;

            return term;

        } else if (std::optional<Token> indentifier = tryConsume(TokenType::IDENTIFIER)) {

            NodeTermIdentifier *identifierTerm = mAllocator.alloc<NodeTermIdentifier>();
            identifierTerm->identifier = *indentifier;
            NodeTerm *term = mAllocator.alloc<NodeTerm>();
            term->term = identifierTerm;

            return term;

        } else if (tryConsume(TokenType::OPEN_PAREN)) {

            NodeTermParen *parenTerm = mAllocator.alloc<NodeTermParen>();
            NodeExpr *expr = parseExpr();
            if (!expr) {
                std::cerr << "Expected expression\n";
                exit(1);
            }
            parenTerm->expr = expr;

            tryConsume(TokenType::CLOSE_PAREN, "Unmatched '('");

            NodeTerm *term = mAllocator.alloc<NodeTerm>();
            term->term = parenTerm;

            return term;
        }

        return nullptr;
    }

    NodeExpr *parseExpr(int minPrec = 0) {
        NodeTerm *term = parseTerm();
        if (!term) {
            return nullptr;
        }

        NodeExpr *lhsExpr = mAllocator.alloc<NodeExpr>();
        lhsExpr->expr = term;

        while (true) {
            std::optional<int> prec;
            if (
                std::optional<Token> curToken = peek();
                !curToken ||
                !(prec = binPrec(curToken->type)) ||
                prec < minPrec
            ) {
                break;
            }

            NodeBinExpr *binExpr = mAllocator.alloc<NodeBinExpr>();
            Token op = consume();

            NodeExpr *rhsExpr = parseExpr(*prec + 1);

            if (op.type == TokenType::ADD) {
                NodeBinExprAdd *addExpr = mAllocator.alloc<NodeBinExprAdd>();
                addExpr->lhs = lhsExpr;
                addExpr->rhs = rhsExpr;
                binExpr->expr = addExpr;
            } else if (op.type == TokenType::MUL) {
                NodeBinExprMul *mulExpr = mAllocator.alloc<NodeBinExprMul>();
                mulExpr->lhs = lhsExpr;
                mulExpr->rhs = rhsExpr;
                binExpr->expr = mulExpr;
            } else if (op.type == TokenType::SUB) {
                NodeBinExprSub *subExpr = mAllocator.alloc<NodeBinExprSub>();
                subExpr->lhs = lhsExpr;
                subExpr->rhs = rhsExpr;
                binExpr->expr = subExpr;
            } else if (op.type == TokenType::DIV) {
                NodeBinExprDiv *divExpr = mAllocator.alloc<NodeBinExprDiv>();
                divExpr->lhs = lhsExpr;
                divExpr->rhs = rhsExpr;
                binExpr->expr = divExpr;
            } else {
                std::cerr << "Expected a binary operator\n";
                exit(1);
            }
            lhsExpr = mAllocator.alloc<NodeExpr>();
            lhsExpr->expr = binExpr;
        }

        return lhsExpr;
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
            letStmt->identifier = *identifier;

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

        while (peek()) {
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

    ArenaAllocator mAllocator;
};
