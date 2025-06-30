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
    INVALID
};


struct Token{
    TokenType type;
    std::string value;

    Token(TokenType t , const std::string& val){
        type = t;
        value = val;
    }
};

std::vector<Token> tokenize(const std::string& input);

#endif