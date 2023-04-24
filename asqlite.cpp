#include <cstdio>
#include <vector>
#include <memory>
#include <string>

const char * sql = "SELECT * from table;";



namespace Token {

    enum Tok {
        // Only called at the end of the string
        T_NULL = 0,
        T_EOF = -1,

        // commands

        // primary
        T_CMD = -2,
        T_NUM = -4,
        T_STR = -5,
        T_ID  = -6,
    };


    static std::string LexerString;
    static double      LexerNumber;
    static Tok         CurrToken;


    Tok GetToken() {
        // Initialize with space so we dont return on the first loop
        static int LastChar = ' ';

        // Have to catch the newlines before they get eaten
        if (LastChar == '\n' || LastChar == '\r' || LastChar == ';') {
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
            LexerString = LastChar;
            while (isalnum((LastChar = getchar())))
                LexerString += LastChar;

            // Check for SQL commands
            if (LexerString == "SELECT" || LexerString == "UPDATE")
                return Token::T_CMD;

            // String

            return Token::T_ID;
        }

        // Parse numbers
        if (isdigit(LastChar) || LastChar == '.') {
            std::string NumStr;
        
            do {
                NumStr += LastChar;
                LastChar = getchar();
            } while (isdigit(LastChar) || LastChar == '.');

            LexerNumber = strtod(NumStr.c_str(), 0);
            return Token::T_NUM;
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

}


namespace AST {

    class Expr {
    public:
        virtual ~Expr()  = default;
    };

    class VariableExpr {
    public:
        VariableExpr(const std::string &name): name{name} {}

        static std::unique_ptr<VariableExpr> Create()
        {
            auto e = std::make_unique<VariableExpr>(Token::LexerString);
            Token::GetNextToken();
            return e;
        }

        std::string name;
    };

    class NumberExpr {
    public:
        NumberExpr(double number): number{number} {}
        
        static std::unique_ptr<NumberExpr> Create(double number)
        {
            auto e = std::make_unique<NumberExpr>(number);
            Token::GetNextToken();
            return e;
        }
        
        double number;
    };

    class BinaryExpr {
    public:
        BinaryExpr(char op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs):
            op{op},
            lhs{std::move(lhs)},
            rhs{std::move(rhs)} {}

        char op;
        std::unique_ptr<Expr> lhs, rhs;
    };

    static std::unique_ptr<Expr> ParseParenExpr();

    static std::unique_ptr<Expr> ParseExpr()
    {

        switch(Token::CurrToken)
        {
            case '(':
                return ParseParenExpr();
            case '\'':
            case '\"':
                //return ParseStringExpr(Token::CurrToken);
                return nullptr;

            default:
                // log
                return nullptr;
        }
    }
    


    static std::unique_ptr<Expr> ParseParenExpr()
    {
        // eat the  opening parenthesis i.e (
        Token::GetNextToken();
        auto e = ParseExpr();
        if (!e)
            return nullptr;

        if (Token::CurrToken != ')') {
            // Log an error
            return nullptr;
        }

        // eat the )
        Token::GetNextToken();
        return e;

    }

}

int main()
{

    while ( true ) {
        //printf("SQL> ");
        Token::Tok token = Token::GetNextToken();
        switch (token)
        {
        case Token::T_NULL:
            printf("Got NULL\n");
            break;


        case Token::T_CMD:
            printf("CMD: %s\n", Token::LexerString.c_str());
            break;

        case Token::T_ID:
            printf("ID: %s\n", Token::LexerString.c_str());
            break;
        
        case Token::T_NUM:
            printf("NUM: %f\n", Token::LexerNumber);
            break;

        case Token::T_EOF:
            return 0;
        
        default:
            printf("Other: %c\n", token);
            break;
        }
    }

    

    return 0;
}