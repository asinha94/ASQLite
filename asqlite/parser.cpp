#include <memory>
#include <string>

#include "parser.h"
#include "lexer.h"
#include "database.h"


namespace asql {

    /* Expression Defintions */

    std::string FunctionExpr::GetAlias() const {
        if (alias.size())
            return alias;

        std::string genName = name + "(";
        for (const auto &a : args)
            genName += a.GetAlias();
        return genName + ")";
    }


    std::string BinaryExpr::GetAlias() const {
        if (alias.size())
            return alias;

        std::string opStr {static_cast<char>(op)};
        return "(" + lhs->GetAlias() + opStr + rhs->GetAlias() + ")";
    }

    float BinaryExpr::eval() const {
        auto l = lhs->eval();
        auto r = rhs->eval();

        switch (op) {
        case '*': return l * r;
        case '/': return l / r;
        case '-': return l - r;
        case '+': return l + r;
        default: break;
        }
        return 0;
    }
    

    static int GetTokPrecedence() {
        if (auto f = LexerBinOpPrecedent.find(GetCurrentToken()); f != LexerBinOpPrecedent.end())
            return f->second;
        return -1;
    }

    /* Parsing Functions */
    static std::unique_ptr<Expr> ParsePrimaryExpr();
    static std::unique_ptr<Expr> ParseExpr();


    static std::unique_ptr<Expr> ParseParenExpr()
    {
        // eat the  opening parenthesis i.e (
        GetNextToken();

        // Parse some arbitrarily long expression
        auto e = ParseExpr();
        if (!e)
            return nullptr;

        if (GetCurrentToken() != T_CLOSE_PAREN) {
            // Log an error
            return nullptr;
        }

        // eat the closing parenthesis
        GetNextToken();
        return e;

    }

    static std::unique_ptr<Expr> ParseBinOpenRHS(int MinTokPrec, std::unique_ptr<Expr> lhs)
    {
        while (true) {
            int CurrTokPrec = GetTokPrecedence();
        
            // Next operator has lower or similar precedence as previous operator
            if (CurrTokPrec <= MinTokPrec)
                return lhs;

            // save the operator and advance to the next token
            int binOp = GetCurrentToken();
            GetNextToken();
            auto rhs = ParsePrimaryExpr();
            
            if (!rhs)
                return nullptr;

            // Check whether the next operator has higher precendence
            // ParseExpr() would have already advanced to the next token
            // If so merge those 2 expressions into a single expression
            int NextTokPrec = GetTokPrecedence();
            if (CurrTokPrec < NextTokPrec) {
                rhs = ParseBinOpenRHS(CurrTokPrec, std::move(rhs));
                if (!rhs)
                    return nullptr;
            }

            lhs = std::make_unique<BinaryExpr>(binOp, std::move(lhs), std::move(rhs));
        }
        
    }

    static std::unique_ptr<FloatExpr> ParseFloat()
    {
        auto e = std::make_unique<FloatExpr>(LexerFloat, LexerString);
        GetNextToken();
        return e;
    }

    static std::unique_ptr<IntExpr> ParseInt()
    {
        auto e = std::make_unique<IntExpr>(LexerInteger, LexerString);
        GetNextToken();
        return e;
    }

    static std::unique_ptr<StringExpr> ParseStr()
    {
        auto e = std::make_unique<StringExpr>(LexerString);
        GetNextToken();
        return e;
    }

    static std::unique_ptr<Expr> ParseIdentifier()
    {
        auto e = std::make_unique<VariableExpr>(LexerString);
        GetNextToken();
        return e;
    }


    static std::unique_ptr<Expr> ParsePrimaryExpr()
    {
        switch(GetCurrentToken())
        {
            case T_OPEN_PAREN:
                return ParseParenExpr();
            case T_RAW_FLOAT:
                return ParseFloat();
            case T_RAW_VAR:
                return ParseIdentifier();
            case T_RAW_INT:
                return ParseInt();
            case T_RAW_STR:
                return ParseStr();
            default:
                return nullptr;
        }
    }

    static std::unique_ptr<Expr> ParseExpr()
    {
        auto e = ParsePrimaryExpr();
        if (!e)
            return nullptr;
        // If its not a binary expression, it will just return e back to us
        return ParseBinOpenRHS(0, std::move(e));
    }

    static std::unique_ptr<Query> ParseSelectQuery()
    {
        Tok token;

        /* Parse the output arguments */
        std::vector<std::unique_ptr<Expr>> SelectArgs;
        while ( true ) {
            // advance to the next expression
            GetNextToken();
            auto e = ParseExpr();
            if (!e)
                return nullptr;

            // Parse the alias if there is one
            token = GetCurrentToken();
            switch(token) {
            case T_KEY_AS:
                token = GetNextToken();
                // Allow the fallthrough if the alias is found
                if (token != T_RAW_STR && token != T_RAW_VAR) {
                    printf("Unknown token after 'AS' in SELECT clause: %d\n", token);
                    return nullptr;
                }
            case T_RAW_STR:
            case T_RAW_VAR:
                e->alias = LexerString;
                token = GetNextToken();
            default:
                break;
            }

            // Append the output variable
            SelectArgs.push_back(std::move(e));

            // Parse another output arg or move onto the table
            if (token != T_COMMA)
                break;
        }

        /* Parse the Table information if provided */
        std::vector<Table> SelectTables;
        if (GetCurrentToken() == T_KEY_FROM) {
            // Get list of tables to cross-join
            while ( true ) {
                token = GetNextToken();

                // TODO: Support raw tuples as tables
                if (token != T_RAW_VAR) {
                    printf("Invalid table name in FROM clause\n");
                    return nullptr;
                }

                Table t{LexerString};

                // Retrieve alias if available
                token = GetNextToken();
                switch(token) {
                case T_KEY_AS:
                    token = GetNextToken();
                    // Allow the fallthrough if the if fails
                    if (token != T_RAW_STR && token != T_RAW_VAR) {
                        printf("Unknown token after 'AS' in FROM clause: %d\n", token);
                        return nullptr;
                    }
                case T_RAW_STR:
                case T_RAW_VAR:
                    t.alias = LexerString;
                    token = GetNextToken();
                    break;
                default:
                    // Use name as alias
                    t.alias = t.name;
                    break;
                }

                // Parse another table or move on
                SelectTables.push_back(t);
                if (token != T_COMMA)
                    break;
            }
        }

        /* Parse where clause */
        std::vector<Filter> SelectFilters;
        if (token == T_KEY_WHERE) {
            while ( true ) {

                token = GetNextToken();
                auto lhs = ParseExpr();
                if (!lhs) {
                    printf("Failed to parse WHERE clause expression\n");
                    return nullptr;
                }

                token = GetCurrentToken();
                // TODO: Support other operators e.g '!='
                if (token != T_EQUALS) {
                    printf("Invalid WHERE clause expression");
                    return nullptr;
                }

                token = GetNextToken();
                auto rhs = ParseExpr();
                if (!rhs) {
                    printf("Failed to parse WHERE clause expression\n");
                    return nullptr;
                }

                SelectFilters.emplace_back(Filter{std::move(lhs), std::move(rhs), EO_EQUALS});

                if (GetCurrentToken() != T_COMMA) {
                    break;
                }
            }
        }

        /* Order clause */

        /* Group clause */

        /* Limit clause */
        int limit = -1;
        if (GetCurrentToken() == T_KEY_LIMIT) {
            token = GetNextToken();
            if (token != T_RAW_INT) {
                printf("Invalid token in LIMIT clause\n");
                return nullptr;
            }
            auto l = ParseInt();
            limit = l->number;
        }



        printf("| ");
        for (auto &arg : SelectArgs)
        {
            printf("%s | ", arg->GetAlias().c_str());
        }
        printf("\n");

        printf("| ");
        for (auto &arg : SelectArgs)
        {
            printf("%s | ", arg->GetAlias().c_str());
        }
        printf("\n");
        return nullptr;
        
    }

int repl()
{

    std::vector<std::unique_ptr<asql::Query>> q;
    std::unique_ptr<asql::Query> qry = nullptr;
 
    while ( true ) {
        auto token = GetCurrentToken();
        if (token == T_ENTER || token == T_NULL) printf("ASQL> ");
        token = asql::GetNextToken();

        switch (token)
        {
        case asql::T_EOF:
            return -1;
        // wait around for next input
        case asql::T_NULL:
        case asql::T_ENTER:
            break;

        case asql::T_QRY_SELECT:
            qry = asql::ParseSelectQuery();
            if (qry) {
                q.push_back(std::move(qry));
                break;
            }

            //asql::ClearTokenLineBuffer();
            break;

        case asql::T_QRY_INSERT:
        case asql::T_QRY_DELETE:
        case asql::T_QRY_UPDATE:
        case asql::T_QRY_CREATE:
            printf("Query Under Construction. Come back later\n");
            break;
        
        default:
            printf("Malformed SQL query. Only basic SELECT, CREATE, INSERT, UPDATE and DELETE supported\n");
            printf("Token: %d, var: %s\n", token, asql::LexerString.c_str());
            asql::ClearTokenLineBuffer();
            break;
        }
    }

}

}