#include "parser.h"
#include <vector>
#include <cctype>   // for isdigit, isalpha, isspace
#include <iostream> // for std::cerr

// Tokenizer
std::vector<Token> tokenize(const std::string& input) {
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < input.size()) {
        char ch = input[i];

        if (std::isspace(ch)) {
            ++i;
            continue;
        }

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

        if (std::isalpha(ch)) {
            std::string idStr;
            while (i < input.size() && (std::isalnum(input[i]) || input[i] == '_')) {
                idStr += input[i];
                ++i;
            }
            tokens.emplace_back(TokenType::IDENTIFIER, idStr);
            continue;
        }

        switch (ch) {
            case '+': tokens.emplace_back(TokenType::PLUS, "+"); break;
            case '-': tokens.emplace_back(TokenType::MINUS, "-"); break;
            case '*': tokens.emplace_back(TokenType::STAR, "*"); break;
            case '/': tokens.emplace_back(TokenType::SLASH, "/"); break;
            case '^': tokens.emplace_back(TokenType::CARET, "^"); break;
            case '=': tokens.emplace_back(TokenType::EQUAL, "="); break;
            case '(': tokens.emplace_back(TokenType::LPAREN, "("); break;
            case ')': tokens.emplace_back(TokenType::RPAREN, ")"); break;
            case ',': tokens.emplace_back(TokenType::COMMA, ","); break;
            default:
                std::cerr << "Error: Unknown character '" << ch << "' at position " << i << "\n";
                tokens.emplace_back(TokenType::INVALID, std::string(1, ch));
                break;
        }

        ++i;
    }

    tokens.emplace_back(TokenType::END_OF_INPUT, "");
    return tokens;
}

// Helpers
bool isOperator(TokenType type) {
    return type == TokenType::PLUS || type == TokenType::MINUS ||
           type == TokenType::STAR || type == TokenType::SLASH ||
           type == TokenType::CARET;
}

int getPrecedence(TokenType type) {
    switch (type) {
        case TokenType::CARET: return 3;
        case TokenType::STAR:
        case TokenType::SLASH: return 2;
        case TokenType::PLUS:
        case TokenType::MINUS: return 1;
        default: return 0;
    }
}

bool isRightAssociative(TokenType type) {
    return type == TokenType::CARET;
}

// toPostfix
std::vector<Token> toPostfix(const std::vector<Token>& tokens) {
    std::vector<Token> output;
    std::vector<Token> opStack;

    for (size_t i = 0; i < tokens.size(); ++i) {
        const Token& token = tokens[i];

        if (token.type == TokenType::NUMBER || token.type == TokenType::IDENTIFIER) {
            // If IDENTIFIER followed by LPAREN → function
            if (token.type == TokenType::IDENTIFIER &&
                i + 1 < tokens.size() && tokens[i + 1].type == TokenType::LPAREN) {
                opStack.push_back(token); // function name to stack
            } else {
                output.push_back(token);
            }
        }

        else if (token.type == TokenType::COMMA) {
            while (!opStack.empty() && opStack.back().type != TokenType::LPAREN) {
                output.push_back(opStack.back());
                opStack.pop_back();
            }
        }

        else if (isOperator(token.type)) {
            while (!opStack.empty() && isOperator(opStack.back().type)) {
                TokenType top = opStack.back().type;
                if ((getPrecedence(top) > getPrecedence(token.type)) ||
                    (getPrecedence(top) == getPrecedence(token.type) &&
                     !isRightAssociative(token.type))) {
                    output.push_back(opStack.back());
                    opStack.pop_back();
                } else {
                    break;
                }
            }
            opStack.push_back(token);
        }

        else if (token.type == TokenType::LPAREN) {
            opStack.push_back(token);
        }

        else if (token.type == TokenType::RPAREN) {
            while (!opStack.empty() && opStack.back().type != TokenType::LPAREN) {
                output.push_back(opStack.back());
                opStack.pop_back();
            }
            if (opStack.empty()) {
                std::cerr << "Error: Mismatched parentheses.\n";
                return {};
            }
            opStack.pop_back(); // remove LPAREN

            // If function name is on top, pop to output
            if (!opStack.empty() && opStack.back().type == TokenType::IDENTIFIER) {
                output.push_back(opStack.back());
                opStack.pop_back();
            }
        }

        else if (token.type == TokenType::EQUAL) {
            // You may ignore or handle assignment separately.
            output.push_back(token);
        }
    }

    while (!opStack.empty()) {
        if (opStack.back().type == TokenType::LPAREN || opStack.back().type == TokenType::RPAREN) {
            std::cerr << "Error: Mismatched parentheses at end.\n";
            return {};
        }
        output.push_back(opStack.back());
        opStack.pop_back();
    }

    return output;
}
