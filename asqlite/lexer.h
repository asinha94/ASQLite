#pragma once

#include <string>

namespace asql {

    enum Tok {
        // SQL special chars
        T_COMMA       = ',',
        T_OPEN_PAREN  = '(',
        T_CLOSE_PAREN = ')',
        T_SEMI_COLON  = ';',
        T_EQUALS      = '=',

        // Only called at the end of the string or statement
        T_NULL        =  0,
        T_EOF         = -1,
        T_ENTER       = -2,

        // commands
        T_QRY_SELECT  = -3,
        T_QRY_DELETE  = -4,
        T_QRY_UPDATE  = -5,
        T_QRY_INSERT  = -6,
        T_QRY_CREATE  = -7,

        // Keywords        
        T_KEY_FROM    = -12,
        T_KEY_WHERE   = -13,
        T_KEY_LIMIT   = -14,
        T_KEY_ORDER   = -15,
        T_KEY_BY      = -16,
        T_KEY_GROUP   = -17,
        T_KEY_INTO    = -18,
        T_KEY_VALUES  = -19,
        T_KEY_JOIN    = -20,
        T_KEY_ON      = -21,
        T_KEY_AS      = -22,
        T_KEY_TABLE   = -23,

        // Raw values or variables
        T_RAW_FLOAT   = -30,
        T_RAW_INT     = -31,
        T_RAW_STR     = -32,
        T_RAW_VAR     = -33,
    };

    /* Precendence for binary operations */
    extern std::unordered_map<char, int> LexerBinOpPrecedent;
    /* SQL Keywords */
    extern std::unordered_map<std::string, int> LexerKeywords;

    /* The tokenized items */
    extern std::string LexerString;
    extern float       LexerFloat;
    extern int         LexerInteger;

    /* Token funtions */
    extern Tok GetToken();
    extern Tok GetNextToken();
    extern Tok GetCurrentToken();
    void       ClearTokenLineBuffer();

}