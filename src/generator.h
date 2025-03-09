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

constexpr int EIGHT_BYTES{ 8 };

class Generator {
public:
    Generator(const Generator &) = delete;
    Generator &operator=(const Generator &) = delete;

    Generator() = default;

    void genTerm(const NodeTerm *const term) {
        std::visit(Visitor{

            [this](const NodeTermIntLiteral *const intLiteralTerm) {
                instruction("mov", "rax", intLiteralTerm->intLiteral.value.value());
                push("rax");
            },

            [this](const NodeTermIdentifier *const identifierTerm) {
                if (!mVars.contains(identifierTerm->identifier.value.value())) {
                    std::cerr << "Undeclared variable: " << identifierTerm->identifier.value.value() << std::endl;
                    exit(1);
                }
                const Var &var{ mVars[identifierTerm->identifier.value.value()] };
                std::stringstream offset{};
                offset << "QWORD [rsp + " << EIGHT_BYTES*(mStackLoc-var.stackLoc-1) << "]";
                push(offset.str());
            },

            [this](const NodeTermParen *const parenTerm) {
                genExpr(parenTerm->expr);
            }

        }, term->term);
    }

    void genBinExpr(const NodeBinExpr *const binExpr) {
        std::visit(Visitor{

            [this](const NodeBinExprAdd *const binExprAdd){
                genExpr(binExprAdd->lhs);
                genExpr(binExprAdd->rhs);
                pop("rbx");
                pop("rax");
                instruction("add", "rax", "rbx");
                push("rax");
            },

            [this](const NodeBinExprSub *const binExprSub){
                genExpr(binExprSub->lhs);
                genExpr(binExprSub->rhs);
                pop("rbx");
                pop("rax");
                instruction("sub", "rax", "rbx");
                push("rax");
            },

            [this](const NodeBinExprMul *const binExprMul){
                genExpr(binExprMul->lhs);
                genExpr(binExprMul->rhs);
                pop("rbx");
                pop("rax");
                instruction("mul", "rbx");
                push("rax");
            },

            [this](const NodeBinExprDiv *const binExprDiv){
                genExpr(binExprDiv->lhs);
                genExpr(binExprDiv->rhs);
                pop("rbx");
                pop("rax");
                instruction("div", "rbx");
                push("rax");
            },

        }, binExpr->expr);
    }

    void genExpr(const NodeExpr *const expr) {
        std::visit(Visitor{

            [this](const NodeTerm *const term) {
                genTerm(term);
            },

            [this](const NodeBinExpr *const binExpr) {
                genBinExpr(binExpr);
            },

        }, expr->expr);
    }

    void genScope(const NodeScope *const scope) {
        beginScope();
        for (const NodeStmt *const stmt : scope->stmts) {
            genStmt(stmt);
        }
        endScope();
    }

    void genBranchIf(const NodeBranchIf *const ifBranch, const std::string &endLabel) {
        genConditionAndScope(ifBranch->expr, ifBranch->scope, endLabel);
    }

    void genBranchElif(const NodeBranchElif *const elifBranch, const std::string &endLabel) {
        genConditionAndScope(elifBranch->expr, elifBranch->scope, endLabel);
    }

    void genBranchElse(const NodeBranchElse *const elseBranch) {
        genScope(elseBranch->scope);
    }

    void genStmt(const NodeStmt *const stmt) {
        std::visit(Visitor{

            [this](const NodeStmtExit *const exitStmt) {
                genExpr(exitStmt->expr);
                instruction("mov", "rax", 60);
                pop("rdi");
                syscall();
            },

            [this](const NodeStmtLet *const letStmt) {
                if (mVars.contains(letStmt->identifier.value.value())) {
                    std::cerr << "Identifier already used: " << letStmt->identifier.value.value() << std::endl;
                    exit(1);
                }
                mVars[letStmt->identifier.value.value()] = { .stackLoc = mStackLoc };
                mVarOrder.push(letStmt->identifier.value.value());
                genExpr(letStmt->expr);
            },

            [this](const NodeStmtIf *const ifStmt) {
                const std::string endLabel{ createLabel() };
                genBranchIf(ifStmt->ifBranch, endLabel);
                for (const NodeBranchElif *const elifBranch : ifStmt->elifBranches) {
                    genBranchElif(elifBranch, endLabel);
                }
                if (ifStmt->elseBranch) {
                    genBranchElse(ifStmt->elseBranch);
                }
                insertLabel(endLabel);
            },

            [this](const NodeScope *const scope) {
                genScope(scope);
            },

        }, stmt->stmt);
    }

    [[nodiscard]] std::string genProg(const NodeProg *const prog) {
        mOutput << "global _start\n";
        mOutput << "_start:\n";

        for (const NodeStmt *const stmt : prog->stmts) {
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
        size_t stackLoc{};
        // TODO Type type;
    };

    std::string createLabel() {
        return "label" + std::to_string(mLabelCount++);
    }

    void insertLabel(const std::string &label) {
        mOutput << label << ":\n";
    }

    void genConditionAndScope(
        const NodeExpr *const condExpr,
        const NodeScope *const scope,
        const std::string &endLabel
    ) {
        // condition
        genExpr(condExpr);
        pop("rax");
        instruction("test", "rax", "rax");

        // false
        const std::string falseLabel{ createLabel() };
        instruction("jz", falseLabel);

        // true
        genScope(scope);

        // end
        instruction("jmp", endLabel);

        insertLabel(falseLabel);
    }

    void beginScope() {
        mScopes.push(mVars.size());
    }

    void endScope() {
        const size_t popCount{ mVars.size() - mScopes.top() };
        instruction("add", "rsp", EIGHT_BYTES * popCount);
        mStackLoc -= popCount;

        for (size_t i{ 0 }; i < popCount; ++i) {
            mVars.erase(mVarOrder.top());
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

    std::stringstream mOutput{};
    size_t mStackLoc{};
    size_t mLabelCount{};
    std::unordered_map<std::string, Var> mVars{};
    std::stack<std::string, std::vector<std::string>> mVarOrder{};
    std::stack<size_t, std::vector<size_t>> mScopes{};
};
