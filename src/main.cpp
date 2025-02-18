#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <utility>

#include "tokenization.h"

//===========================================================================
// Hard-coded
constexpr char ASM_PATH[] = "out.asm";
constexpr char OBJ_PATH[] = "out.o";
constexpr char OUTNAME[] = "out";

//===========================================================================
// Compiler

std::string tokens_to_asm(const std::vector<Token> &tokens) {
    std::stringstream output;
    output << "global _start\n";
    output << "_start:\n";

    for (int i = 0; i < tokens.size(); ++i) {
        if (tokens[i].type == TokenType::Exit) {
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

    Tokenizer tokenizer(std::move(contents));
    auto tokens = tokenizer.tokenize();

    std::string asm_code = tokens_to_asm(tokens);
    write_file(ASM_PATH, asm_code);
    assemble(ASM_PATH);
    link(OBJ_PATH, OUTNAME);

    return 0;
}
