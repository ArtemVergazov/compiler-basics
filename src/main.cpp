#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <utility> // std::move
#include <vector>

#include "error.h"
#include "generator.h"
#include "parser.h"
#include "tokenizer.h"

//===========================================================================
// Hardcoded
constexpr char ASM_PATH[]{ "out.asm" };
constexpr char OBJ_PATH[]{ "out.o" };
constexpr char OUTNAME[]{ "out" };

//===========================================================================
// Compiler
void callAssembler(const char *const filename) {
    const std::string command{ "nasm -felf64 " + std::string(filename) };
    if (std::system(command.c_str()) != 0) {
        error("Assembler error");
    }
}

void callLinker(const char *const filename, const char *const outname) {
    const std::string command{ "ld -o " + std::string(outname) + " " + std::string(filename) };
    if (std::system(command.c_str()) != 0) {
        error("Linker error");
    }
}

//===========================================================================
// Work with files
std::string readFile(const char *const filename) {
    const std::ifstream input{ filename };
    std::stringstream contentsStream{};
    contentsStream << input.rdbuf();
    return contentsStream.str();
}

void writeFile(const char *const filename, const std::string &contents) {
    std::ofstream file{ filename };
    file << contents;
}

//===========================================================================
int main(int argc, char **argv) {
    if (argc != 2) {
        error("Usage: compile <input.code>");
    }

    std::string contents{ readFile(argv[1]) };

    Tokenizer tokenizer{ std::move(contents) };
    std::vector<Token> tokens{ tokenizer.tokenize() };

    Parser parser{ std::move(tokens) };
    const NodeProg *const prog{ parser.parseProg() }; // parser deallocates the memory

    Generator generator{};
    const std::string asmCode{ generator.genProg(prog) };

    writeFile(ASM_PATH, asmCode);
    callAssembler(ASM_PATH);
    callLinker(OBJ_PATH, OUTNAME);

    return 0;
}
