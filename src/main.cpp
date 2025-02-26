#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <utility> // std::move
#include <vector>

#include "generator.h"
#include "parser.h"
#include "tokenizer.h"

//===========================================================================
// Hardcoded
constexpr char ASM_PATH[] = "out.asm";
constexpr char OBJ_PATH[] = "out.o";
constexpr char OUTNAME[] = "out";

//===========================================================================
// Compiler
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
    std::vector<Token> tokens = tokenizer.tokenize();

    Parser parser(std::move(tokens));
    NodeProg prog = parser.parseProg();

    // if (!prog) {
    //     std::cerr << "No exit statement found\n";
    //     exit(1);
    // }

    Generator generator(std::move(prog));
    std::string asm_code = generator.genProg();

    write_file(ASM_PATH, asm_code);
    assemble(ASM_PATH);
    link(OBJ_PATH, OUTNAME);

    return 0;
}
