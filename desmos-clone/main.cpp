#include "parser.h"
#include <iostream>
#include <vector>

// Convert TokenType enum to readable string
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

// Pretty print a list of tokens
void printTokens(const std::vector<Token>& tokens) {
    for (const Token& token : tokens) {
        std::cout << tokenTypeToString(token.type) << " : \"" << token.value << "\"\n";
    }
}

int main() {
    std::string expr = "pow(2, 3) + max(1, 2, 3)";

    std::cout << "Input Expression:\n" << expr << "\n\n";

    std::vector<Token> tokens = tokenize(expr);

    std::cout << "Tokenized:\n";
    printTokens(tokens);

    std::vector<Token> postfix = toPostfix(tokens);

    std::cout << "\nPostfix:\n";
    for (const Token& token : postfix) {
        std::cout << token.value << " ";
    }
    std::cout << "\n";

    return 0;
}
