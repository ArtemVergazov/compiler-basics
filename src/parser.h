#pragma once

#include <cstddef>
#include <iostream>
#include <optional>
#include <vector>

#include "text_reader.h"
#include "tokenizer.h"

struct NodeExpr {
    Token intLiteral;
};

struct NodeExit {
    NodeExpr expr;
};

class Parser : public TextReader<std::vector<Token>> {
public:
    explicit Parser(std::vector<Token> &&tokens) : TextReader(std::move(tokens)) {}

    std::optional<NodeExpr> parseExpr() {
        if (peek().has_value() && peek().value().type == TokenType::IntLiteral) {
            return NodeExpr{ .intLiteral = consume() };
        }
        return {};
    }

    std::optional<NodeExit> parse() {
        std::optional<NodeExit> nodeExit;

        while (peek().has_value()) {
            if (peek().value().type == TokenType::Exit) {
                consume();

                auto nodeExpr = parseExpr();
                if (!nodeExpr) {
                    std::cerr << "Invalid expression\n";
                    exit(1);
                }
                nodeExit = { .expr = nodeExpr.value() };

                if (!peek().has_value() || peek().value().type != TokenType::Semi) {
                    std::cerr << "Invalid expression\n";
                    exit(1);
                }
                consume();
            }
        }
        mIndex = 0;

        return nodeExit;
    }
};
