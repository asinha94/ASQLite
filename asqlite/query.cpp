#include <algorithm>
#include <unordered_set>
#include "query.h"


namespace asql {

    std::unordered_map<std::string, std::unordered_map<std::string, ColumnType>> database_tables {
        {"EMPLOYEES",
            {{"EMP_ID",      CT_INT},
             {"EMP_TYPE_ID", CT_INT},
             {"NAME",        CT_STR},
             {"WEIGHT_KG",   CT_FLOAT}}},

        {"HOURS",
            {{"EMP_ID",      CT_INT},
             {"TIME_START",  CT_INT},
             {"TIME_END",    CT_INT}}},

        {"EMPLOYEE_TYPE",
            {{"EMP_TYPE_ID", CT_INT},
             {"TYPE",        CT_STR}}},
    };

    void SelectQuery::Validate()
    {
        /* First check if the tables exist. */
        std::unordered_map<std::string, std::string> table_aliases;

        for (const auto &table : tables) {
            if (auto f = database_tables.find(table.name); f == database_tables.end()) {
                printf("Unknown table %s\n", table.name.c_str());
                return;
            }

            /* Create an alias helper table at the same time */
            if (auto f = table_aliases.find(table.alias); f != table_aliases.end()) {
                printf("Duplicate table alias '%s' found\n", table.alias.c_str());
                return;
            }

            table_aliases.emplace(table.alias, table.name);
        }

        /* Check that all the columns listed can be found in the FROM tables */
        std::unordered_map<std::string, std::unordered_set<std::string>> table_references;
        for (const auto &column: columns) {

            const auto& is_binary_expression = dynamic_cast<const BinaryExpr*>(column.get());

            for (const auto var: column->GetVariables()) {
                // Check the qualified tables first i.e select a.x from a
                auto var_expr = dynamic_cast<const VariableExpr*>(var);
                if (var_expr->qualifier.size()) {
                    auto f = table_aliases.find(var_expr->qualifier);
                    if (f == table_aliases.end()) {
                        printf("Unknown qualifier '%s'\n", var_expr->qualifier.c_str());
                        return;
                    }

                    // Table has to be present, dont bother checking for end()
                    auto dt = database_tables.find(f->second);
                    auto cols = dt->second;
                    
                    auto col = cols.find(var_expr->name);
                    if (col == cols.end()) {
                        printf("Unknown column '%s' in table '%s'\n", var_expr->name.c_str(), dt->first.c_str());
                        return;
                    }

                    // If part of a binary expression, the column type can't be a string

                    auto& t = table_references[dt->first];
                    t.emplace(var_expr->name);
                    
                } else { // unqualified column names i.e select x from a
                    
                    /* Check if multiple columns with same name exist */
                    bool found = false;
                    for (const auto& table: tables) {
                        const auto& dt = database_tables[table.name];
                        auto col = dt.find(var_expr->name);
                        if (col != dt.end()) {
                            if (found) {
                                printf("Ambiguous reference to column '%s'\n", var_expr->name.c_str());
                                return;
                            }

                            found = true;
                            auto& table_ref = table_references[table.name];
                            table_ref.emplace(var_expr->name);
                        }
                    }

                    // TODO: If (count != 1) to prevent branches?
                    if (!found) {
                        printf("Unknown column '%s' in SELECT clause\n", var_expr->name.c_str()); //
                        return;
                    }

                }

                /* Read rows from DB */
                for (const auto& table: table_references) {
                    printf("In table: %s\n", table.first.c_str());
                    for (const auto& col: table.second) {
                        printf("\tCOL: %s\n", col.c_str());
                    }
                }



            }
            
        }



    }
}