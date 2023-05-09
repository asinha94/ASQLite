#include <iostream>
#include <string>
#include <cstdint>

#include "parser.h"
#include "database.h"

/**

        {"HOURS",
            {{"EMP_ID",      CT_INT},
             {"TIME_START",  CT_INT},
             {"TIME_END",    CT_INT}}},

        {"EMPLOYEE_TYPE",
            {{"EMP_TYPE_ID", CT_INT},
             {"TYPE",        CT_STR}}},
    };
*/



int main()
{

    asql::insertEmployee({0, 0, "Anu", 140.f});
    asql::insertEmployee({0, 0, "Tak", 180.f});
    asql::insertEmployee({0, 0, "Sav", 120.f});
    asql::insertEmployee({0, 0, "Raj", 160.f});

    asql::repl();
    return 0;
}