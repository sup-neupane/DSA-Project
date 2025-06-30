#include "parser.h"
#include <vector>
#include <cctype>   // for isdigit, isalpha, isspace
#include <iostream> // for std::cerr

std::vector<Token> tokenize(const std::string& input) {
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < input.size()) {
        char ch = input[i];

        // Skip whitespace
        if (std::isspace(ch)) {
            ++i;
            continue;
        }

        // Number token
        if (std::isdigit(ch) || (ch == '.' && i + 1 < input.size() && std::isdigit(input[i + 1]))) {
            std::string numStr;
            bool hasDecimal = false;

            while (i < input.size() && (std::isdigit(input[i]) || input[i] == '.')) {
                if (input[i] == '.') {
                    if (hasDecimal) {
                        std::cerr << "Error: Multiple decimal points in number at position " << i << "\n";
                        break;
                    }
                    hasDecimal = true;
                }
                numStr += input[i];
                ++i;
            }

            tokens.emplace_back(TokenType::NUMBER, numStr);
            continue;
        }

        // Identifier token (letters or underscore followed by letters/digits)
        if (std::isalpha(ch)) {
            std::string idStr;

            while (i < input.size() && (std::isalnum(input[i]) || input[i] == '_')) {
                idStr += input[i];
                ++i;
            }

            tokens.emplace_back(TokenType::IDENTIFIER, idStr);
            continue;
        }

        // Operators and punctuation
        switch (ch) {
            case '+':
                tokens.emplace_back(TokenType::PLUS, "+"); break;
            case '-':
                tokens.emplace_back(TokenType::MINUS, "-"); break;
            case '*':
                tokens.emplace_back(TokenType::STAR, "*"); break;
            case '/':
                tokens.emplace_back(TokenType::SLASH, "/"); break;
            case '^':
                tokens.emplace_back(TokenType::CARET, "^"); break;
            case '=':
                tokens.emplace_back(TokenType::EQUAL, "="); break;
            case '(':
                tokens.emplace_back(TokenType::LPAREN, "("); break;
            case ')':
                tokens.emplace_back(TokenType::RPAREN, ")"); break;
            case ',':
                tokens.emplace_back(TokenType::COMMA, ","); break;
            default:
                // Error handling for invalid characters
                std::cerr << "Error: Unknown character '" << ch << "' at position " << i << "\n";
                tokens.emplace_back(TokenType::INVALID, std::string(1, ch));
                break;
        }

        ++i;
    }

    tokens.emplace_back(TokenType::END_OF_INPUT, "");
    return tokens;
}
