#pragma once

#include <cstdlib> // size_t
#include <sstream>
#include <unordered_map>
#include <utility> // std::move
#include <variant> // std::visit
#include "parser.h"

constexpr int EIGHT_BIT = 8;

class Generator {
public:
    explicit Generator(NodeProg prog) : mProg(std::move(prog)) {}

    void genTerm(const NodeTerm *term) {
        std::visit(Visitor{

            [this](const NodeTermIntLiteral *intLiteralTerm) {
                mov("rax", intLiteralTerm->intLiteral.value.value());
                push("rax");
            },

            [this](const NodeTermIdentifier *identifierTerm) {
                if (!mVars.contains(identifierTerm->identifier.value.value())) {
                    std::cerr << "Undeclared variable: " << identifierTerm->identifier.value.value() << std::endl;
                    exit(1);
                }
                const Generator::Var &var = mVars[identifierTerm->identifier.value.value()];
                std::stringstream offset;
                offset << "QWORD [rsp + " << EIGHT_BIT*(mStackLoc-var.stackLoc-1) << "]\n";
                push(offset.str());
            },

        }, term->term);
    }

    void genExpr(const NodeExpr *expr) {
        std::visit(Visitor{

            [this](const NodeTerm *term) {
                genTerm(term);
            },

            [this](const NodeBinExpr *binExpr) {
                genExpr(binExpr->expr->lhs);
                genExpr(binExpr->expr->rhs);
                pop("rax");
                pop("rbx");
                add("rax", "rbx");
                push("rax");
            },

        }, expr->expr);
    }

    void genStmt(const NodeStmt *stmt) {
        std::visit(Visitor{

            [this](const NodeStmtExit *exitStmt) {
                genExpr(exitStmt->expr);
                mov("rax", 60);
                pop("rdi");
                syscall();
            },

            [this](const NodeStmtLet *letStmt) {
                if (mVars.contains(letStmt->identifier.value.value())) {
                    std::cerr << "Identifier already used: " << letStmt->identifier.value.value() << std::endl;
                    exit(1);
                }
                mVars[letStmt->identifier.value.value()] = { .stackLoc = mStackLoc };
                genExpr(letStmt->expr);
            },

        }, stmt->stmt);
    }

    [[nodiscard]] std::string genProg() {
        mOutput << "global _start\n";
        mOutput << "_start:\n";

        for (const NodeStmt *stmt : mProg.stmts) {
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
        size_t stackLoc;
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

    void add(const std::string &reg1, const std::string &reg2) {
        mOutput << "    add " << reg1 << ", " << reg2 << "\n";
    }

    void syscall() {
        mOutput << "    syscall\n";
    }

    const NodeProg mProg;
    std::stringstream mOutput{};
    size_t mStackLoc = 0;
    std::unordered_map<std::string, Var> mVars;
};
