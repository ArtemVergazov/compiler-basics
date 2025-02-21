#pragma once

#include <cstddef>
#include <iostream>
#include <optional>
#include <variant>
#include <vector>

#include "text_reader.h"
#include "tokenizer.h"

struct NodeExprIntLiteral {
    Token intLiteral;
};

struct NodeExprIdentifier {
    Token identifier;
};

using NodeExpr = std::variant<NodeExprIntLiteral, NodeExprIdentifier>;

struct NodeStmtExit {
    NodeExpr expr;
};

struct NodeStmtLet {
    Token identifier;
    NodeExpr expr;
};

using NodeStmt = std::variant<NodeStmtExit, NodeStmtLet>;

struct NodeProg {
    std::vector<NodeStmt> stmts;
};

class Parser : public TextReader<std::vector<Token>> {
public:
    explicit Parser(std::vector<Token> &&tokens) : TextReader(std::move(tokens)) {}

    std::optional<NodeExpr> parseExpr() {
        if (peek().has_value() && peek().value().type == TokenType::INT_LITERAL) {

            return NodeExprIntLiteral{ .intLiteral = consume() };

        } else if (peek().has_value() && peek().value().type == TokenType::IDENTIFIER) {

            return NodeExprIdentifier{ .identifier = consume() };

        }

        return {};
    }

    std::optional<NodeStmt> parseStmt() {
        if (
            peek().has_value() && peek().value().type == TokenType::EXIT &&
            peek(1).has_value() && peek(1).value().type == TokenType::OPEN_PAREN
        ) {
            consume();
            consume();

            auto expr = parseExpr();
            if (!expr) {
                std::cerr << "Invalid expression\n";
                exit(1);
            }
            NodeStmtExit exitStmt{ .expr = expr.value() };

            if (!peek().has_value() || peek().value().type != TokenType::CLOSE_PAREN) {
                std::cerr << "Expected ')'\n";
                exit(1);
            }
            consume();

            if (!peek().has_value() || peek().value().type != TokenType::SEMI) {
                std::cerr << "Expected ';'\n";
                exit(1);
            }
            consume();

            return exitStmt;

        } else if (
            peek().has_value() && peek().value().type == TokenType::LET &&
            peek(1).has_value() && peek(1).value().type == TokenType::IDENTIFIER &&
            peek(2).has_value() && peek(2).value().type == TokenType::EQ
        ) {
            consume();
            NodeStmtLet letStmt{ .identifier = consume() };
            consume();

            auto expr = parseExpr();
            if (!expr) {
                std::cerr << "Invalid expression\n";
                exit(1);
            }
            letStmt.expr = expr.value();

            if (!peek().has_value() || peek().value().type != TokenType::SEMI) {
                std::cerr << "Expected ';'\n";
                exit(1);
            }
            consume();

            return letStmt;
        }

        return {};
    }

    NodeProg parseProg() {
        NodeProg prog;

        while (peek().has_value()) {
            auto stmt = parseStmt();
            if (!stmt) {
                std::cerr << "Invalid statement\n";
                exit(1);
            }
            prog.stmts.push_back(stmt.value());
        }
        mIndex = 0;

        return prog;
    }
};
