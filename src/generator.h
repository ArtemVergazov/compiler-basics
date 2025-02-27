#pragma once

#include <cstdlib> // size_t
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility> // std::move
#include <variant> // std::visit

#include "not_implemented_error.h"
#include "parser.h"

constexpr int EIGHT_BYTES = 8;

class Generator {
public:
    explicit Generator(NodeProg prog) : mProg(std::move(prog)) {}

    void genTerm(const NodeTerm *term) {
        std::visit(Visitor{

            [this](const NodeTermIntLiteral *intLiteralTerm) {
                instruction("mov", "rax", intLiteralTerm->intLiteral.value.value());
                push("rax");
            },

            [this](const NodeTermIdentifier *identifierTerm) {
                if (!mVars.contains(identifierTerm->identifier.value.value())) {
                    std::cerr << "Undeclared variable: " << identifierTerm->identifier.value.value() << std::endl;
                    exit(1);
                }
                const Var &var = mVars[identifierTerm->identifier.value.value()];
                std::stringstream offset;
                offset << "QWORD [rsp + " << EIGHT_BYTES*(mStackLoc-var.stackLoc-1) << "]";
                push(offset.str());
            },

            [this](const NodeTermParen *parenTerm) {
                genExpr(parenTerm->expr);
            }

        }, term->term);
    }

    void genBinExpr(const NodeBinExpr *binExpr) {
        std::visit(Visitor{

            [this](const NodeBinExprAdd *binExprAdd){
                genExpr(binExprAdd->lhs);
                genExpr(binExprAdd->rhs);
                pop("rbx");
                pop("rax");
                instruction("add", "rax", "rbx");
                push("rax");
            },

            [this](const NodeBinExprSub *binExprSub){
                genExpr(binExprSub->lhs);
                genExpr(binExprSub->rhs);
                pop("rbx");
                pop("rax");
                instruction("sub", "rax", "rbx");
                push("rax");
            },

            [this](const NodeBinExprMul *binExprMul){
                genExpr(binExprMul->lhs);
                genExpr(binExprMul->rhs);
                pop("rbx");
                pop("rax");
                instruction("mul", "rbx");
                push("rax");
            },

            [this](const NodeBinExprDiv *binExprDiv){
                genExpr(binExprDiv->lhs);
                genExpr(binExprDiv->rhs);
                pop("rbx");
                pop("rax");
                instruction("div", "rbx");
                push("rax");
            },

        }, binExpr->expr);
    }

    void genExpr(const NodeExpr *expr) {
        std::visit(Visitor{

            [this](const NodeTerm *term) {
                genTerm(term);
            },

            [this](const NodeBinExpr *binExpr) {
                genBinExpr(binExpr);
            },

        }, expr->expr);
    }

    void genScope(const NodeScope *scope) {
        beginScope();
        for (const NodeStmt *stmt : scope->stmts) {
            genStmt(stmt);
        }
        endScope();
    }

    void genStmt(const NodeStmt *stmt) {
        std::visit(Visitor{

            [this](const NodeStmtExit *exitStmt) {
                genExpr(exitStmt->expr);
                instruction("mov", "rax", 60);
                pop("rdi");
                syscall();
            },

            [this](const NodeStmtLet *letStmt) {
                if (mVars.contains(letStmt->identifier.value.value())) {
                    std::cerr << "Identifier already used: " << letStmt->identifier.value.value() << std::endl;
                    exit(1);
                }
                mVars[letStmt->identifier.value.value()] = { .stackLoc = mStackLoc };
                mVarOrder.push(letStmt->identifier.value.value());
                genExpr(letStmt->expr);
            },

            [this](const NodeStmtIf *ifStmt) {
                genExpr(ifStmt->expr);
                pop("rax");
                instruction("test", "rax", "rax");
                std::string label = createLabel();
                instruction("jz", label);
                genScope(ifStmt->scope);
                mOutput << label << ":\n";
            },

            [this](const NodeScope *scope) {
                genScope(scope);
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
        instruction("mov", "rax", 60);
        instruction("mov", "rdi", 0);
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

    std::string createLabel() {
        return "label" + std::to_string(mLabelCount++);
    }

    void beginScope() {
        mScopes.push(mVars.size());
    }

    void endScope() {
        size_t popCount = mVars.size() - mScopes.top();
        instruction("add", "rsp", EIGHT_BYTES * popCount);
        mStackLoc -= popCount;

        for (size_t i = 0; i < popCount; ++i) {
            std::string name = mVarOrder.top();
            mVars.erase(name);
            mVarOrder.pop();
        }

        mScopes.pop();
    }

    void push(const std::string &reg) {
        instruction("push", reg);
        ++mStackLoc;
    }

    void pop(const std::string &reg) {
        instruction("pop", reg);
        --mStackLoc;
    }

    void syscall() {
        instruction("syscall");
    }

    void instruction(const std::string &instruction) {
        mOutput << "    " << instruction << "\n";
    }

    template <typename T>
    void instruction(const std::string &instruction, const T &arg) {
        mOutput << "    " << instruction << " " << arg << "\n";
    }

    template <typename T1, typename T2>
    void instruction(const std::string &instruction, const T1 &arg1, const T2 &arg2) {
        mOutput << "    " << instruction << " " << arg1 << ", " << arg2 << "\n";
    }

    const NodeProg mProg;
    std::stringstream mOutput{};
    size_t mStackLoc{};
    size_t mLabelCount{};
    std::unordered_map<std::string, Var> mVars{};
    std::stack<std::string, std::vector<std::string>> mVarOrder{};
    std::stack<size_t, std::vector<size_t>> mScopes{};
};
