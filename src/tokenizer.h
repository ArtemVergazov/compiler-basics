#pragma once

#include <cctype> // std::isalpha, std::alnum, std::isspace
#include <cstdlib> // size_t
#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include "text_reader.h"

enum class TokenType {
    EXIT, INT_LITERAL, SEMI, OPEN_PAREN, CLOSE_PAREN, IDENTIFIER, LET, EQ,
    PLUS, MINUS, STAR, FSLASH, OPEN_CURLY, CLOSE_CURLY, IF, ELIF, ELSE,
};

inline std::optional<int> binPrec(const TokenType type) {
    switch (type) {
    case TokenType::PLUS:
        return 0;
    case TokenType::MINUS:
        return 0;
    case TokenType::STAR:
        return 1;
    case TokenType::FSLASH:
        return 1;
    default:
        return {};
    }
}

struct Token {
    TokenType type{};
    std::optional<std::string> value{};
};

class Tokenizer : public TextReader<std::string> {
public:
    Tokenizer(std::string &&src) : TextReader{ std::move(src) } {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens{};
        std::string buf{};

        while (peek()) {

            if (std::isalpha(*peek())) { // letter symbol

                buf.push_back(consume());
                while (peek() && std::isalnum(*peek())) { // identifier or keyword
                    buf.push_back(consume());
                }

                if (buf == "exit") { // `exit` keyword
                    tokens.push_back({ .type = TokenType::EXIT });
                } else if (buf == "let") { // `let` keyword
                    tokens.push_back({ .type = TokenType::LET });
                } else if (buf == "if") { // `if` keyword
                    tokens.push_back({ .type = TokenType::IF });
                } else if (buf == "elif") { // `elif` keyword
                    tokens.push_back({ .type = TokenType::ELIF });
                } else if (buf == "else") { // `else` keyword
                    tokens.push_back({ .type = TokenType::ELSE });
                } else { // identifier
                    tokens.push_back({ .type = TokenType::IDENTIFIER, .value = buf });
                }
                buf.clear();

            } else if (std::isdigit(*peek())) { // digit

                buf.push_back(consume());
                while (peek() && std::isdigit(*peek())) { // int literal
                    buf.push_back(consume());
                }
                tokens.push_back({ .type = TokenType::INT_LITERAL, .value = buf });
                buf.clear();

            } else if (*peek() == '(') {
                consume();
                tokens.push_back({ .type = TokenType::OPEN_PAREN });
            } else if (*peek() == ')') {
                consume();
                tokens.push_back({ .type = TokenType::CLOSE_PAREN });
            } else if (*peek() == ';') {
                consume();
                tokens.push_back({ .type = TokenType::SEMI });
            } else if (*peek() == '=') {
                consume();
                tokens.push_back({ .type = TokenType::EQ });
            } else if (*peek() == '+') {
                consume();
                tokens.push_back({ .type = TokenType::PLUS });
            } else if (*peek() == '-') {
                consume();
                tokens.push_back({ .type = TokenType::MINUS });
            } else if (*peek() == '*') {
                consume();
                tokens.push_back({ .type = TokenType::STAR });
            } else if (*peek() == '/') {
                consume();
                tokens.push_back({ .type = TokenType::FSLASH });
            } else if (*peek() == '{') {
                consume();
                tokens.push_back({ .type = TokenType::OPEN_CURLY });
            } else if (*peek() == '}') {
                consume();
                tokens.push_back({ .type = TokenType::CLOSE_CURLY });
            } else if (*peek() == '#') { // comment
                for (consume(); peek() && *peek() != '\n'; consume()) {} // not new line
            } else if (std::isspace(*peek())) { // space symbol
                consume();
            } else {
                std::cerr << "Syntax error!\n";
                exit(1);
            }
        }
        mIndex = 0;

        return tokens;
    }
};
