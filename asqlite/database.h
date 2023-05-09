#pragma once

#include <unordered_map>
#include <memory>
#include <vector>


namespace asql {

    struct DBTable {
        size_t size;
        void * data;
        ~DBTable() {delete data;}
    };

    using DBTablePtr = std::unique_ptr<DBTable>;


/* Fake DB. All of this will be removed */

struct EmployeeTbl {
    uint64_t emp_id;
    uint64_t emp_type_id;
    std::string name;
    float weight;
};
std::unordered_map<std::string, std::vector<DBTablePtr>> TableData;

void insertEmployee(EmployeeTbl data);


}
