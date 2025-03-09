#include <iostream>
#include <vector>
#include <cctype>
#include <memory>
#include <fstream>
enum class TokenType {
    FUNC, IDENTIFIER, INT, CONST, VAR, IF, ELSE, WHILE, NUMBER, ASSIGN, PLUS, MINUS, MULT, DIV, SEMICOLON,
    LPAREN, RPAREN, LBRACE, RBRACE, END, GREATER, LESS, EQUAL, NOTEQUAL
};

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
public:
    Lexer(const std::string& input) : input(input), pos(0) {}
    
    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        
        while (pos < input.size()) {
            if (std::isspace(input[pos])) {
                ++pos;
            } else if (std::isalpha(input[pos])) {
                std::string ident = readIdentifier();
                if (ident == "func") {
                    tokens.push_back({TokenType::FUNC, ident});
                } else if (ident == "int") {
                    tokens.push_back({TokenType::INT, ident});
                } else if (ident == "const") {
                    tokens.push_back({TokenType::CONST, ident});
                } else if (ident == "var") {
                    tokens.push_back({TokenType::VAR, ident});
                } else if (ident == "if") {
                    tokens.push_back({TokenType::IF, ident});
                } else if (ident == "else") {
                    tokens.push_back({TokenType::ELSE, ident});
                } else if (ident == "while") {
                    tokens.push_back({TokenType::WHILE, ident});
                } else {
                    tokens.push_back({TokenType::IDENTIFIER, ident});
                }
            } else if (std::isdigit(input[pos])) {
                tokens.push_back({TokenType::NUMBER, readNumber()});
            } else {
                switch (input[pos]) {
                    case '(': tokens.push_back({TokenType::LPAREN, "("}); break;
                    case ')': tokens.push_back({TokenType::RPAREN, ")"}); break;
                    case '{': tokens.push_back({TokenType::LBRACE, "{"}); break;
                    case '}': tokens.push_back({TokenType::RBRACE, "}"}); break;
                    case '=': tokens.push_back({TokenType::ASSIGN, "="}); break;
                    case '+': tokens.push_back({TokenType::PLUS, "+"}); break;
                    case '-': tokens.push_back({TokenType::MINUS, "-"}); break;
                    case '*': tokens.push_back({TokenType::MULT, "*"}); break;
                    case '/': tokens.push_back({TokenType::DIV, "/"}); break;
                    case ';': tokens.push_back({TokenType::SEMICOLON, ";"}); break;
                    case '>': tokens.push_back({TokenType::GREATER, ">"}); break;
                    case '<': tokens.push_back({TokenType::LESS, "<"}); break;
                    default: std::cerr << "Caractère inattendu : " << input[pos] << std::endl;
                }
                ++pos;
            }
        }
        tokens.push_back({TokenType::END, ""});
        return tokens;
    }
    
private:
    std::string input;
    size_t pos;
    
    std::string readIdentifier() {
        size_t start = pos;
        while (pos < input.size() && std::isalnum(input[pos])) ++pos;
        return input.substr(start, pos - start);
    }
    
    std::string readNumber() {
        size_t start = pos;
        while (pos < input.size() && std::isdigit(input[pos])) ++pos;
        return input.substr(start, pos - start);
    }
};

// AST Nodes
struct ASTNode {
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0;
    virtual void generateAssembly(std::ofstream& out) const = 0;
};

struct ExpressionNode : public ASTNode {
    std::string value;
    ExpressionNode(std::string value) : value(std::move(value)) {}
    
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "Number: " << value << std::endl;
    }
    
    void generateAssembly(std::ofstream& out) const override {
        out << "\tmov eax, " << value << "\n";
    }
};

struct BinaryOpNode : public ASTNode {
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    TokenType op;

    BinaryOpNode(std::unique_ptr<ASTNode> left, TokenType op, std::unique_ptr<ASTNode> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "BinaryOp: ";
        left->print(indent + 2);
        std::cout << std::string(indent + 2, ' ') << (op == TokenType::PLUS ? "+" :
                                                       op == TokenType::MINUS ? "-" :
                                                       op == TokenType::MULT ? "*" : "/") << std::endl;
        right->print(indent + 2);
    }

    void generateAssembly(std::ofstream& out) const override {
        left->generateAssembly(out);
        out << "\tpush eax\n";
        right->generateAssembly(out);
        out << "\tpop ebx\n";
        
        switch (op) {
            case TokenType::PLUS: out << "\tadd eax, ebx\n"; break;
            case TokenType::MINUS: out << "\tsub eax, ebx\n"; break;
            case TokenType::MULT: out << "\timul eax, ebx\n"; break;
            case TokenType::DIV:  out << "\tcdq\n\tidiv ebx\n"; break;
            default: break;
        }
    }
};

struct AssignmentNode : public ASTNode {
    std::string varName;
    std::unique_ptr<ASTNode> expression;

    AssignmentNode(std::string varName, std::unique_ptr<ASTNode> expression)
        : varName(std::move(varName)), expression(std::move(expression)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "Assignment: " << varName << " = " << std::endl;
        expression->print(indent + 2);
    }

    void generateAssembly(std::ofstream& out) const override {
        expression->generateAssembly(out);
        out << "\tmov [" << varName << "], eax\n";
    }
};

struct FunctionNode : public ASTNode {
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> body;

    FunctionNode(std::string name) : name(std::move(name)) {}

    void print(int indent = 0) const override {
        std::cout << "Function: " << name << std::endl;
        for (const auto& stmt : body) {
            stmt->print(indent + 2);
        }
    }

    void generateAssembly(std::ofstream& out) const override {
        out << "." << name << ":\n";
        for (const auto& stmt : body) {
            stmt->generateAssembly(out);
        }
        out << "\tret\n";
    }
};

struct StatementNode : public ASTNode {

};


struct VariableDeclarationNode : public StatementNode {
    std::string name;
    std::unique_ptr<ExpressionNode> value;

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "VarDecl: " << name << std::endl;
    }

    void generateAssembly(std::ofstream& out) const override {}
};

struct IfNode : public StatementNode {
    std::unique_ptr<ExpressionNode> condition;
    std::vector<std::unique_ptr<ASTNode>> thenBranch;
    std::vector<std::unique_ptr<ASTNode>> elseBranch;

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "If Statement" << std::endl;
    }

    void generateAssembly(std::ofstream& out) const override {}
};

struct WhileNode : public StatementNode {
    std::unique_ptr<ExpressionNode> condition;
    std::vector<std::unique_ptr<ASTNode>> body;

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "While Statement" << std::endl;
    }

    void generateAssembly(std::ofstream& out) const override {}
};



class Parser {
public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens), pos(0) {}
    
    std::unique_ptr<ASTNode> parse() {
        if (match(TokenType::FUNC)) {
            consume(TokenType::FUNC);
            std::string funcName = consume(TokenType::IDENTIFIER).value;
            consume(TokenType::LPAREN);
            consume(TokenType::RPAREN);
            consume(TokenType::LBRACE);
            std::vector<std::unique_ptr<StatementNode>> body;
            while (!match(TokenType::RBRACE)) {
                body.push_back(parseStatement());
            }
            consume(TokenType::RBRACE);
            return std::make_unique<FunctionNode>(funcName);
        }
        return nullptr;
    }
    
private:
    const std::vector<Token>& tokens;
    size_t pos;
    
    bool match(TokenType type) {
        return pos < tokens.size() && tokens[pos].type == type;
    }
    
    Token consume(TokenType type) {
        if (match(type)) {
            return tokens[pos++];
        }
        std::cout << "type: " << (int)type << std::endl;
        throw std::runtime_error("Unexpected token: expected " + std::to_string(static_cast<int>(type)) + 
        ", got " + std::to_string(static_cast<int>(tokens[pos].type)) + 
        " (" + tokens[pos].value + ")");
    }

    std::unique_ptr<StatementNode> parseStatement() { return nullptr; }
};

int main() {
    std::string code = R"(
    
    def MaFonction()
        var x
        x = 5 + 3
        
    
    )";
    Lexer lexer(code);
    std::vector<Token> tokens = lexer.tokenize();

    for (const auto& token : tokens) {
        std::cout << "Token: " << static_cast<int>(token.type) << " (" << token.value << ")\n";
    }
    
    
    try {
        Parser parser(tokens);
        std::unique_ptr<ASTNode> ast = parser.parse();

        
    
    if (ast) {
        ast->print();
        
        std::ofstream outFile("output.asm");
        if (outFile.is_open()) {
            ast->generateAssembly(outFile);
            outFile.close();
            std::cout << "Code assembleur généré dans output.asm" << std::endl;
        } else {
            std::cerr << "Erreur lors de l'ouverture du fichier" << std::endl;
        }
    } else {
        std::cerr << "Erreur de parsing" << std::endl;
    }

} catch(const std::exception &e) {
    std::cerr << "Erreur de parsing: " << e.what() << std::endl;
}
    
    return 0;
}
