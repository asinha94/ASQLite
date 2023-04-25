#include <cstdio>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <algorithm>

#define LOG(...) printf(__VA_ARGS__)

namespace asql {

    enum Tok {
        // Only called at the end of the string or statement
        T_NULL   =  0,
        T_EOF    = -1,
        T_ENTER  = -2,

        // commands
        T_QRY_SELECT = -3,
        T_QRY_DELETE = -4,
        T_QRY_UPDATE = -5,
        T_QRY_INSERT = -6,
        T_QRY_CREATE = -7,

        // Keywords        
        T_KEY_FROM   = -12,
        T_KEY_WHERE  = -13,
        T_KEY_LIMIT  = -14,
        T_KEY_ORDER  = -15,
        T_KEY_BY     = -16,
        T_KEY_GROUP  = -17,
        T_KEY_INTO   = -18,
        T_KEY_VALUES = -19,
        T_KEY_JOIN   = -20,
        T_KEY_ON     = -21,
        T_KEY_AS     = -22,
        T_KEY_TABLE  = -23,

        // Raw values or variables
        T_RAW_FLOAT  = -30,
        T_RAW_STR    = -31,
        T_RAW_VAR    = -32,
    };

    /* Defines precendence for binary operations */
    static std::unordered_map<char, int> LexerBinOpPrecedent = {
        {'+', 10},
        {'-', 10},
        {'/', 20},
        {'*', 20}
    };

    /* Defines */
    static std::unordered_map<std::string, int> LexerKeywords = {
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


    static std::string LexerString;
    static float       LexerFloat;
    static int         LexerInteger;
    static Tok         CurrToken;


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
        if (isdigit(LastChar) || LastChar == '.') {
            std::string NumStr;
        
            do {
                NumStr += LastChar;
                LastChar = getchar();
            } while (isdigit(LastChar) || LastChar == '.');

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

            // Check for end of file.  Don't eat the EOF.
            if (LastChar == EOF) return T_EOF;

        }

        // Otherwise, just return the character as its ascii value.
        int PrevChar = LastChar;
        LastChar = getchar();
        return (Tok) PrevChar;
    }

    static Tok GetNextToken() {
        return CurrToken = GetToken();
    }

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
        virtual float eval() { return number;}
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
                printf("l: %.4f, r: %.4f, l%cr = %.4f", l, r, l+r);

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
        if (auto f = LexerBinOpPrecedent.find(CurrToken); f != LexerBinOpPrecedent.end())
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

        if (CurrToken != ')') {
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
            int binOp = CurrToken;
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

            lhs = std::make_unique<Expr>(BinaryExpr(binOp, std::move(lhs), std::move(rhs)));
        }
        
    }

    static std::unique_ptr<FloatExpr> ParseFloat()
    {
        auto e = std::make_unique<FloatExpr>(FloatExpr(LexerFloat));
        GetNextToken();
        return e;
    }

    static std::unique_ptr<Expr> ParseIdentifier()
    {
        auto e = std::make_unique<VariableExpr>(VariableExpr(LexerString));
        GetNextToken();
        return e;
    }


    static std::unique_ptr<Expr> ParsePrimaryExpr()
    {
        switch(CurrToken)
        {
            case '(':
                return ParseParenExpr();
            case T_RAW_FLOAT:
                return ParseFloat();
            case T_RAW_VAR:
                return ParseIdentifier();
            //case '\'':
            //case '\"':
            default:
                printf("Can't parse primary expression: %d\n", CurrToken);
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

            if (CurrToken != ',')
                break;            
        }

        double result = 0;
        printf("Parsed select args: ");
        for (auto &arg : SelectArgs)
        {
            printf("%.4f, ", arg->eval());
        }
        printf("\n");
        return nullptr;

        // Parsed all of the select arguments, now need check the table
        
    }

    void ClearTokenLineBuffer()
    {
        while (CurrToken != asql::T_ENTER && CurrToken != asql::T_NULL)
            asql::GetNextToken();
    }
}


int main()
{

    std::vector<std::unique_ptr<asql::Query>> q;
    std::unique_ptr<asql::Query> qry = nullptr;
 
    while ( true ) {
        printf("SQL> ");
        auto token = asql::GetNextToken();
        switch (token)
        {
        case asql::T_EOF:
            return 0;

        case asql::T_NULL:
            printf("NULL");
            //asql::ParseExpression(v);
            break;

        case asql::T_ENTER:
            printf("Enter\n");
            break;

        case asql::T_QRY_SELECT:
            qry = asql::ParseSelectQuery();
            if (qry) {
                q.push_back(std::move(qry));
                break;
            }

            printf("failied to parse query!\n");
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

    return 0;
}