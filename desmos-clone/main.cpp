#include "parser.h"
#include <iostream>
#include <vector>

// Helper function to convert TokenType to readable string
std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::CARET: return "CARET";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::COMMA: return "COMMA";
        case TokenType::END_OF_INPUT: return "END_OF_INPUT";
        case TokenType::INVALID: return "INVALID";
        default: return "UNKNOWN";
    }
}

int main() {
    std::string expr = "y = log(x) - 3.5x^2";

    std::vector<Token> tokens = tokenize(expr);

    std::cout << "Tokens:\n";
    for (const Token& token : tokens) {
        std::cout << tokenTypeToString(token.type) << " : \"" << token.value << "\"\n";
    }

    return 0;
}
