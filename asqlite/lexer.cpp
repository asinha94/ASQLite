#include <cstdio>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <algorithm>


#include "lexer.h"

namespace asql
{
    // Key for last tokenized item
    static Tok CurrToken;

     // The tokenized items
    std::string LexerString  = "";
    float       LexerFloat   = 0.0f;
    int         LexerInteger = 0;

    std::unordered_map<char, int> LexerBinOpPrecedent = {
        {'+', 10},
        {'-', 10},
        {'/', 20},
        {'*', 20}
    };

    /* SQL Keywords */
    std::unordered_map<std::string, int> LexerKeywords = {
        {"AS",     T_KEY_AS},
        {"BY",     T_KEY_BY},
        {"CREATE", T_QRY_CREATE},
        {"DELETE", T_QRY_DELETE},
        {"FROM",   T_KEY_FROM},
        {"GROUP",  T_KEY_GROUP},
        {"INSERT", T_QRY_INSERT},
        {"INTO",   T_KEY_INTO},
        {"JOIN",   T_KEY_JOIN},
        {"LIMIT",  T_KEY_LIMIT},
        {"ORDER",  T_KEY_ORDER},
        {"ON",     T_KEY_ON},
        {"SELECT", T_QRY_SELECT},
        {"TABLE",  T_KEY_TABLE},
        {"UPDATE", T_QRY_UPDATE},
        {"VALUES", T_KEY_VALUES},
        {"WHERE",  T_KEY_WHERE}
    };
    
    Tok GetNextToken() {
        return CurrToken = GetToken();
    }

    void ClearTokenLineBuffer()
    {
        while (CurrToken != asql::T_ENTER && CurrToken != asql::T_NULL)
            asql::GetNextToken();
    }

    Tok GetCurrentToken()
    {
        return CurrToken;
    }

    /*
    * The main tokenising function. i.e its job is to scan characters and figure out if it has run into
    * a string, an number (int/float), some sort of variable name, function name etc...
    * The parser is the one that figures out if its all valid 
    */
    Tok GetToken() {
        // Initialize with space so we dont return on the first loop
        static int LastChar = ' ';
        // Have to catch the newlines before they get eaten
        if (LastChar == '\n' || LastChar == '\r') {
            LastChar = ' ';
            return T_ENTER;
        }
        
        if (LastChar == ';') {
            LastChar = ' ';
            return T_NULL;
        }

        // strip out the initial whitespace.
        while (isspace(LastChar))
            LastChar = getchar();

        // Parse alphanumberic tokens
        if (isalpha(LastChar)) {
            // Fill in the string
            // TODO: Optimize, use string_view
            char FirstChar = LastChar;
            LexerString = ::toupper(FirstChar);
            while (isalnum((LastChar = getchar())))
                LexerString += ::toupper(LastChar);

            auto keyword = LexerKeywords.find(LexerString);
            if ( keyword != LexerKeywords.end() )
                return static_cast<Tok>(keyword->second);

            return T_RAW_VAR;
        }

        // Parse string literals
        if (LastChar == '"' || LastChar == '\'') {
            int terminating_char = LastChar;

        }

        // Parse numbers
        if (isdigit(LastChar)) {
            std::string NumStr;
        
            do {
                NumStr += LastChar;
                LastChar = getchar();
            } while (isdigit(LastChar));

            if (isspace(LastChar) || LastChar == '+' || LastChar == '-' || LastChar == '/' || LastChar == '*' || LastChar == ';' || LastChar == ')' || LastChar == ',') {
                LexerInteger = atoi(NumStr.c_str());
                return T_RAW_INT;
            }

            // Only valid option from this point is a floating point (with 1 decimal point)
            if (LastChar != '.') {
                printf("Parse error near character '%c'\n", LastChar);
                return T_EOF;
            }

            LexerFloat = strtof(NumStr.c_str(), 0);
            return T_RAW_FLOAT;
        }

        // Remove comments
        if (LastChar == '#') {
            // Comment until end of line.
            do
                LastChar = getchar();
            while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

            if (LastChar != EOF) return GetToken();

            // Check for end of file. Don't eat the EOF.
            if (LastChar == EOF) return T_EOF;

        }

        // Otherwise, just return the character as its ascii value.
        int PrevChar = LastChar;
        LastChar = getchar();
        return (Tok) PrevChar;
    }

    
}

