#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <optional>
#include <vector>
#include <cstdlib>

//===========================================================================
// Hard-coded
constexpr char ASM_PATH[] = "out.asm";
constexpr char OBJ_PATH[] = "out.o";
constexpr char OUTNAME[] = "out";

//===========================================================================
// Compiler

enum class TokenType {
    Return,
    IntLiteral,
    Semi
};

struct Token {
    TokenType type;
    std::optional<std::string> value;
};

std::vector<Token> tokenize(const std::string &str) {
    std::vector<Token> tokens;
    std::string buf;

    for (int i = 0; i < str.length(); ++i) {
        if (std::isalpha(str[i])) {
            buf.push_back(str[i++]);
            while (i < str.length() && std::isalnum(str[i])) {
                buf.push_back(str[i++]);
            }
            --i;
            if (buf == "return") {
                tokens.push_back({.type = TokenType::Return});
                buf.clear();
            } else {
                std::cerr << "Syntax error!\n";
                exit(1);
            }
        } else if (std::isdigit(str[i])) {
            buf.push_back(str[i++]);
            while (i < str.length() && std::isdigit(str[i])) {
                buf.push_back(str[i++]);
            }
            --i;
            tokens.push_back({.type = TokenType::IntLiteral, .value = buf});
            buf.clear();
        } else if (str[i] == ';') {
            tokens.push_back({.type = TokenType::Semi});
        } else if (std::isspace(str[i])) {
            continue;
        } else {
            std::cerr << "Syntax error!\n";
            exit(1);
        }
    }

    return tokens;
}

std::string tokens_to_asm(const std::vector<Token> &tokens) {
    std::stringstream output;
    output << "global _start\n";
    output << "_start:\n";

    for (int i = 0; i < tokens.size(); ++i) {
        if (tokens[i].type == TokenType::Return) {
            if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::IntLiteral) {
                if (i + 2 < tokens.size() && tokens[i + 2].type == TokenType::Semi) {
                    output << "    mov rax, 60\n";
                    output << "    mov rdi, " << tokens[i + 1].value.value() << "\n";
                    output << "    syscall\n";
                }
            }
        }
    }

    return output.str();
}

void assemble(const char *filename) {
    std::string command = "nasm -felf64 " + std::string(filename);
    std::system(command.c_str());
}

void link(const char *filename, const char *outname) {
    std::string command = "ld -o " + std::string(outname) + " " + std::string(filename);
    std::system(command.c_str());
}

//===========================================================================
// Work with files

std::string read_file(const char *filename) {
    std::ifstream input(filename);
    std::stringstream contents_stream;
    contents_stream << input.rdbuf();
    return contents_stream.str();
}

void write_file(const char *filename, const std::string &contents) {
    std::ofstream file(filename);
    file << contents;
}

//===========================================================================
int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: compile <input.code>\n";
        return 1;
    }

    std::string contents = read_file(argv[1]);
    auto tokens = tokenize(contents);
    std::string asm_code = tokens_to_asm(tokens);
    write_file(ASM_PATH, asm_code);
    assemble(ASM_PATH);
    link(OBJ_PATH, OUTNAME);

    return 0;
}
