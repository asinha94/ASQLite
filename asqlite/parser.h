#pragma once

#include <string>
#include <vector>
#include <memory>

namespace asql {
    extern int repl();

    class Expr {
    public:
        Expr(const std::string& alias): alias{alias} {}
        virtual ~Expr() = default;
        virtual float eval() const { return 0; };
        virtual std::string GetAlias() const { return alias; }
        virtual std::vector<const Expr*> GetVariables() const {return {}; }; 
        std::string alias;
    };


    class FunctionExpr: public Expr {
    public:
    FunctionExpr(const std::string &name): Expr{""}, name{name} {}
    std::string GetAlias() const;
    float eval() const = 0;
    std::string name;
    std::vector<Expr> args;

    };


    class VariableExpr: public Expr {
    public:
        VariableExpr(const std::string &name): Expr{name}, name{name} {}
        std::string name;
        std::string qualifier;
        std::vector<const Expr*> GetVariables() const { return {this}; }
    };


    class StringExpr: public Expr {
    public:
        StringExpr(const std::string &str): Expr{"'" + str + "'"}, str{str} {}
        std::string str;
    };


    class FloatExpr: public Expr {
    public:
        FloatExpr(float number, const std::string &numstr): Expr{numstr}, number{number} {}
        float eval() const override { return number;}

        float number;
    };


    class IntExpr: public Expr {
    public:
        IntExpr(int number, const std::string &numstr): Expr{numstr}, number{number} {}
        int number;
        float eval() const override { return number;}
    };


    class BinaryExpr: public Expr {
    public:
        BinaryExpr(int op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs):
            Expr{""},
            op{op},
            lhs{std::move(lhs)},
            rhs{std::move(rhs)} {}
        
        float eval() const;
        std::string GetAlias() const;
        std::vector<const Expr*> GetVariables() const;
        
        int op;
        std::unique_ptr<Expr> lhs;
        std::unique_ptr<Expr> rhs;
    };
}
