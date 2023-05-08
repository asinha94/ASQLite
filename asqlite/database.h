#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <utility>

#include "parser.h"


namespace asql {

    enum EqualityOp {
        EO_LESS_THAN,
        EO_LESS_THAN_EQUAL,
        EO_EQUALS,
        EO_NOT_EQUAL,
        EO_GREATER_THAN,
        EO_GREATER_THAN_EQUALS,
    };

    enum ColumnType: int {
        CT_INT = 1,
        CT_FLOAT,
        CT_STR
    };

    class Table {
    public:
        Table(const std::string& name):
            name{name},
            alias{name} {}
        std::string name;
        std::string alias; 
    };


    class Filter {
    public:
        Filter(std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs, EqualityOp Op):
            lhs{std::move(lhs)},
            rhs{std::move(rhs)},
            Op{Op} {}
        std::unique_ptr<Expr> lhs;
        std::unique_ptr<Expr> rhs;
        EqualityOp Op;
    };


    class SelectQuery {
    public:
        void Validate();
        std::vector<std::unique_ptr<Expr>> columns;
        std::vector<Table> tables;
        std::vector<Filter> filters;
        int limit = -1;
    };

    using ColumnPair = std::pair<std::string, ColumnType>;
    extern std::unordered_map<std::string, std::unordered_map<std::string, ColumnType>> database_tables;

}