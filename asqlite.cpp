#include <cstdio>
#include <vector>
#include <string>

const char * sql = "SELECT * from table;";


void parse(const char * str)
{
    while (char c = *str++) {

        // Parse out the spaces
        while (true) {
            if (!isspace(c)) break;
            c = *str++;
        }

        std::string s;

        if (isalpha(c) || ispunct(c)) {
            do
                s += c;
            while (isalnum(c = *str++) || ispunct(c));
            printf("String: %s\n", s.c_str());
        }

        if (isdigit(c)) {
            do
                s += c;
            while (isdigit(c = *str++));
            printf("Int: %s\n", s.c_str());
        }

        if (c == '\0')
            return;
    }
}


int main()
{
    parse(sql);
    return 0;
}