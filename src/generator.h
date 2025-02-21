#pragma once

#include <sstream>
#include <unordered_map>
#include <utility>
#include "parser.h"

class Generator {
public:
    explicit Generator(NodeProg prog) : mProg(std::move(prog)) {}

    void genExpr(const NodeExpr &expr) {
        std::visit(Visitor{

            [this](const NodeExprIntLiteral &intLiteralExpr) {
                mov("rax", intLiteralExpr.intLiteral.value.value());
                push("rax");
            },

            [this](const NodeExprIdentifier &identifierExpr) {
                if (!mVars.contains(identifierExpr.identifier.value.value())) {
                    std::cerr << "Undeclared variable: " << identifierExpr.identifier.value.value() << std::endl;
                    exit(1);
                }
                const auto &var = mVars[identifierExpr.identifier.value.value()];
                std::stringstream offset;
                offset << "QWORD [rsp + " << 8*(mStackLoc-var.stackLoc-1) << "]\n";
                push(offset.str());
            },

        }, expr);
    }

    void genStmt(const NodeStmt &stmt) {
        std::visit(Visitor{
            [this](const NodeStmtExit &exitStmt) {
                genExpr(exitStmt.expr);
                mov("rax", 60);
                pop("rdi");
                syscall();
            },
            [this](const NodeStmtLet &letStmt) {
                if (mVars.contains(letStmt.identifier.value.value())) {
                    std::cerr << "Identifier already used: " << letStmt.identifier.value.value() << std::endl;
                    exit(1);
                }
                mVars[letStmt.identifier.value.value()] = { .stackLoc = mStackLoc };
                genExpr(letStmt.expr);
            },
        }, stmt);
    }

    std::string genProg() {
        mOutput << "global _start\n";
        mOutput << "_start:\n";

        for (const NodeStmt &stmt : mProg.stmts) {
            genStmt(stmt);
        }

        // Exit with zero if no `exit` statement
        mov("rax", 60);
        mov("rdi", 0);
        syscall();

        return mOutput.str();
    }

private:
    // Helper type for the visitor
    template<typename... Ts>
    struct Visitor : Ts... {
        using Ts::operator()...;
    };

    struct Var {
        std::size_t stackLoc;
        // TODO Type type;
    };

    template <typename ValueT>
    void mov(const std::string &reg, const ValueT &val) {
        mOutput << "    mov " << reg << ", " << val << "\n";
    }

    void push(const std::string &reg) {
        mOutput << "    push " << reg << "\n";
        ++mStackLoc;
    }

    void pop(const std::string &reg) {
        mOutput << "    pop " << reg << "\n";
        --mStackLoc;
    }

    void syscall() {
        mOutput << "    syscall\n";
    }

    const NodeProg mProg;
    std::stringstream mOutput{};
    std::size_t mStackLoc = 0;
    std::unordered_map<std::string, Var> mVars;
};
