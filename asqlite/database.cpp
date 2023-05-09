#include <string>
#include "database.h"


namespace asql {
    void insertEmployee(EmployeeTbl data)
    {
        auto& table = TableData["employee"];
        EmployeeTbl * ptr = new EmployeeTbl{data};
        auto p = std::make_unique<DBTable>(sizeof(EmployeeTbl), ptr);
        table.push_back(p);
    }
}