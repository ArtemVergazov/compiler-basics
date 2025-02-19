#pragma once

#include <sstream>
#include <utility>
#include "parser.h"

class Generator {
public:
    explicit Generator(NodeExit root) : mRoot(std::move(root)) {}

    [[nodiscard]] std::string generate() const {
        std::stringstream output;
        output << "global _start\n";
        output << "_start:\n";
        output << "    mov rax, 60\n";
        output << "    mov rdi, " << mRoot.expr.intLiteral.value.value() << "\n";
        output << "    syscall\n";

        return output.str();
    }

private:
    const NodeExit mRoot;
};
