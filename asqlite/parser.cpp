#include <memory>
#include <string>

#include "lexer.h"

namespace asql {

    /* Expressions */
    class Expr {
    public:
        virtual ~Expr()  = default;
        virtual float eval() {return 0;}
    };

    class FunctionExpr: public Expr {
    public:
    FunctionExpr(const std::string &name): name{name} {}
    std::string name;
    std::vector<Expr> args;
    };

    class VariableExpr: public Expr {
    public:
        VariableExpr(const std::string &name): name{name} {}
        std::string name;
    };

    class FloatExpr: public Expr {
    public:
        FloatExpr(float number): number{number} {}
        float number;
        float eval() { return number;}
    };

    class IntExpr: public Expr {
    public:
        IntExpr(int number): number{number} {}
        int number;
        float eval() { return number;}
    };

    class BinaryExpr: public Expr {
    public:
        BinaryExpr(int op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs):
            op{op},
            lhs{std::move(lhs)},
            rhs{std::move(rhs)} {}

            virtual float eval() {
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

        int op;
        std::unique_ptr<Expr> lhs, rhs;
    };

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

        if (GetCurrentToken() != ')') {
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
        auto e = std::make_unique<FloatExpr>(LexerFloat);
        GetNextToken();
        return e;
    }

    static std::unique_ptr<IntExpr> ParseInt()
    {
        auto e = std::make_unique<IntExpr>(LexerInteger);
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
            case '(':
                return ParseParenExpr();
            case T_RAW_FLOAT:
                return ParseFloat();
            case T_RAW_VAR:
                return ParseIdentifier();
            case T_RAW_INT:
                return ParseInt();
            //case '\'':
            //case '\"':
            default:
                printf("Can't parse primary expression: %d\n", GetCurrentToken());
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


    class Query {
    public:
        virtual ~Query()  = default;
    };

    class SelectQuery : Query {
    public:
        SelectQuery() {}
    };

    static std::unique_ptr<Query> ParseSelectQuery()
    {
        std::vector<std::unique_ptr<asql::Expr>> SelectArgs;

        while ( true ) {
            // parse the columns to select
            GetNextToken();
            auto e = ParseExpr();
            if (!e)
                return nullptr;
            SelectArgs.push_back(std::move(e));

            // Check for identifier for the column
            // check for AS identifier

            if (GetCurrentToken() != ',')
                break;            
        }

        printf("Parsed select args: ");
        for (auto &arg : SelectArgs)
        {
            printf("%.4f, ", arg->eval());
        }
        printf("\n");
        return nullptr;

        // Parsed all of the select arguments, now need check the table
        
    }

int repl()
{

    std::vector<std::unique_ptr<asql::Query>> q;
    std::unique_ptr<asql::Query> qry = nullptr;
 
    while ( true ) {
        printf("SQL> ");
        auto token = asql::GetNextToken();
        switch (token)
        {
        case asql::T_EOF:
            return -1;

        case asql::T_NULL:
        case asql::T_ENTER:
            printf("Enter or NULL: %d\n", token);
            break;

        case asql::T_QRY_SELECT:
            qry = asql::ParseSelectQuery();
            if (qry) {
                q.push_back(std::move(qry));
                break;
            }

            asql::ClearTokenLineBuffer();
            break;


        case asql::T_QRY_INSERT:
        case asql::T_QRY_DELETE:
        case asql::T_QRY_UPDATE:
        case asql::T_QRY_CREATE:
            printf("The Query: %d\n", token);
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