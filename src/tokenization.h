#pragma once
#include <string>
#include <optional>
#include <vector>
#include <iostream>

enum class TokenType { Exit, IntLiteral, Semi };

struct Token {
    TokenType type;
    std::optional<std::string> value;
};

class Tokenizer {
public:
    inline explicit Tokenizer(std::string &&src) : m_src(src) {}

    inline std::vector<Token> tokenize() {
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

        m_index = 0;
        return tokens;
    }

private:
    [[nodiscard]] std::optional<char> peek(int ahead = 1) const {
        if (m_index + ahead > m_src.length()) {
            return {};
        }

        return m_src[m_index];
    }

    char consume() { return m_src[m_index++]; }

    const std::string m_src;
    int m_index = 0;
};
