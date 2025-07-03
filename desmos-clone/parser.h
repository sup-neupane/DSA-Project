#ifndef PARSER_H
#define PARSER_H


#include <string> 
#include <vector> 

enum class TokenType {
    NUMBER,
    IDENTIFIER,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    CARET,
    LPAREN,
    RPAREN,
    EQUAL,
    COMMA,
    END_OF_INPUT,
    INVALID,
    UMINUS,  
    UPLUS 
};


struct Token{
    TokenType type;
    std::string value;

    Token(TokenType t , const std::string& val){
        type = t;
        value = val;
    }
};

enum class NodeType{
    NUMBER,
    VARIABLE,
    BINARY_OP,
    FUNCTION,
    UNARY_OP
};

struct ASTNode {
    NodeType type;
    std::string value; // For number, variable name, or function name
    std::vector<ASTNode*> children; // Left/right for binary ops, args for funcs

    ASTNode(NodeType t, const std::string& val) {
        type = t;
        value = val;
    }
};




std::vector<Token> tokenize(const std::string& input);
std::vector<Token> toPostfix(const std::vector<Token>& tokens);
ASTNode* buildAST(const std::vector<Token>& postfix);
void printAST(ASTNode* node, int depth = 0);
void freeAST(ASTNode* node);

#endif