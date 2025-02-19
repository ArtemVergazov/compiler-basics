#pragma once

#include <cstddef>
#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include "text_reader.h"

enum class TokenType { Exit, IntLiteral, Semi };

struct Token {
    TokenType type;
    std::optional<std::string> value;
};

class Tokenizer : public TextReader<std::string> {
public:
    Tokenizer(std::string &&src) : TextReader(std::move(src)) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        std::string buf;

        while (peek().has_value()) {
            if (std::isalpha(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && std::isalnum(peek().value())) {
                    buf.push_back(consume());
                }
                if (buf == "exit") {
                    tokens.push_back({ .type = TokenType::Exit });
                    buf.clear();
                } else {
                    std::cerr << "Syntax error!\n";
                    exit(1);
                }
            } else if (std::isdigit(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && std::isdigit(peek().value())) {
                    buf.push_back(consume());
                }
                tokens.push_back({ .type = TokenType::IntLiteral, .value = buf });
                buf.clear();
            } else if (peek().value() == ';') {
                consume();
                tokens.push_back({ .type = TokenType::Semi });
            } else if (std::isspace(peek().value())) {
                consume();
                continue;
            } else {
                std::cerr << "Syntax error!\n";
                exit(1);
            }
        }
        mIndex = 0;

        return tokens;
    }
};
