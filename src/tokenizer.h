#pragma once

#include <cctype> // std::isalpha, std::alnum, std::isspace
#include <cstdlib> // size_t
#include <optional>
#include <string>
#include <vector>

#include "error.h"
#include "text_reader.h"

enum class TokenType {
    EXIT, INT_LITERAL, SEMI, OPEN_PAREN, CLOSE_PAREN, IDENTIFIER, LET, EQ,
    PLUS, MINUS, STAR, FSLASH, OPEN_CURLY, CLOSE_CURLY, IF, ELIF, ELSE,
};

inline std::optional<int> binPrec(const TokenType &type) {
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

std::string to_string(const TokenType &type) {
    switch (type) {
    case TokenType::EXIT:
        return "'exit'";
    case TokenType::INT_LITERAL:
        return "int literal";
    case TokenType::SEMI:
        return "';'";
    case TokenType::OPEN_PAREN:
        return "'('";
    case TokenType::CLOSE_PAREN:
        return "')'";
    case TokenType::IDENTIFIER:
        return "identifier";
    case TokenType::LET:
        return "'let'";
    case TokenType::EQ:
        return "'='";
    case TokenType::PLUS:
        return "'+'";
    case TokenType::MINUS:
        return "'-'";
    case TokenType::STAR:
        return "'*'";
    case TokenType::FSLASH:
        return "'/'";
    case TokenType::OPEN_CURLY:
        return "'{'";
    case TokenType::CLOSE_CURLY:
        return "'}'";
    case TokenType::IF:
        return "'if'";
    case TokenType::ELIF:
        return "'elif'";
    case TokenType::ELSE:
        return "'else'";
    }

    return {}; // unreachable
}

struct Token {
    TokenType type{};
    int ln{};
    std::optional<std::string> value{};
};

class Tokenizer : public TextReader<std::string> {
public:
    Tokenizer(std::string &&src) : TextReader{ std::move(src) } {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens{};
        std::string buf{};
        int lineCount{ 1 };
        while (peek()) {

            if (std::isalpha(*peek())) { // letter symbol

                buf.push_back(consume());
                while (peek() && std::isalnum(*peek())) { // identifier or keyword
                    buf.push_back(consume());
                }

                if (buf == "exit") { // `exit` keyword
                    tokens.push_back({ .type = TokenType::EXIT, .ln = lineCount });
                } else if (buf == "let") { // `let` keyword
                    tokens.push_back({ .type = TokenType::LET, .ln = lineCount });
                } else if (buf == "if") { // `if` keyword
                    tokens.push_back({ .type = TokenType::IF, .ln = lineCount });
                } else if (buf == "elif") { // `elif` keyword
                    tokens.push_back({ .type = TokenType::ELIF, .ln = lineCount });
                } else if (buf == "else") { // `else` keyword
                    tokens.push_back({ .type = TokenType::ELSE, .ln = lineCount });
                } else { // identifier
                    tokens.push_back({ .type = TokenType::IDENTIFIER, .ln = lineCount, .value = buf });
                }
                buf.clear();

            } else if (std::isdigit(*peek())) { // digit

                buf.push_back(consume());
                while (peek() && std::isdigit(*peek())) { // int literal
                    buf.push_back(consume());
                }
                tokens.push_back({ .type = TokenType::INT_LITERAL, .ln = lineCount, .value = buf });
                buf.clear();

            } else if (*peek() == '(') {
                consume();
                tokens.push_back({ .type = TokenType::OPEN_PAREN, .ln = lineCount });
            } else if (*peek() == ')') {
                consume();
                tokens.push_back({ .type = TokenType::CLOSE_PAREN, .ln = lineCount });
            } else if (*peek() == ';') {
                consume();
                tokens.push_back({ .type = TokenType::SEMI, .ln = lineCount });
            } else if (*peek() == '=') {
                consume();
                tokens.push_back({ .type = TokenType::EQ, .ln = lineCount });
            } else if (*peek() == '+') {
                consume();
                tokens.push_back({ .type = TokenType::PLUS, .ln = lineCount });
            } else if (*peek() == '-') {
                consume();
                tokens.push_back({ .type = TokenType::MINUS, .ln = lineCount });
            } else if (*peek() == '*') {
                consume();
                tokens.push_back({ .type = TokenType::STAR, .ln = lineCount });
            } else if (*peek() == '/') {
                consume();
                tokens.push_back({ .type = TokenType::FSLASH, .ln = lineCount });
            } else if (*peek() == '{') {
                consume();
                tokens.push_back({ .type = TokenType::OPEN_CURLY, .ln = lineCount });
            } else if (*peek() == '}') {
                consume();
                tokens.push_back({ .type = TokenType::CLOSE_CURLY, .ln = lineCount });
            } else if (*peek() == '#') { // comment
                for (consume(); peek() && *peek() != '\n'; consume()) {} // not new line
            } else if (*peek() == '\n') { // newline
                consume();
                ++lineCount;
            } else if (std::isspace(*peek())) { // space symbol
                consume();
            } else {
                error("Invalid token");
            }
        }
        mIndex = 0;

        return tokens;
    }
};
