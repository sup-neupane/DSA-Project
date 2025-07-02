#include "parser.h"
#include <vector>
#include <cctype>   // for isdigit, isalpha, isspace
#include <iostream> // for std::cerr
#include <algorithm>
#include <stack> // for AST building
#include <regex> 

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

    std::vector<int> argCountStack; // Tracks function argument counts

    for (size_t i = 0; i < tokens.size(); ++i) {
        const Token& token = tokens[i];

        if (token.type == TokenType::NUMBER || token.type == TokenType::IDENTIFIER) {
            // Function name → push to stack
            if (token.type == TokenType::IDENTIFIER &&
                i + 1 < tokens.size() && tokens[i + 1].type == TokenType::LPAREN) {
                opStack.push_back(token);      // Function name
                argCountStack.push_back(0);    // Start counting args
            } else {
                output.push_back(token);       // Number or variable
            }
        }

        else if (token.type == TokenType::COMMA) {
            while (!opStack.empty() && opStack.back().type != TokenType::LPAREN) {
                output.push_back(opStack.back());
                opStack.pop_back();
            }
            if (!argCountStack.empty()) {
                argCountStack.back() += 1;
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

            opStack.pop_back(); // pop LPAREN

            // If there was a function before LPAREN, pop and annotate
            if (!opStack.empty() && opStack.back().type == TokenType::IDENTIFIER) {
                std::string funcName = opStack.back().value;
                int argCount = (argCountStack.empty() ? 0 : argCountStack.back() + 1); // count commas + 1
                if (!argCountStack.empty()) argCountStack.pop_back();
                opStack.pop_back();

                output.push_back(Token(TokenType::IDENTIFIER, funcName + "@" + std::to_string(argCount)));
            }
        }

        else if (token.type == TokenType::EQUAL) {
            output.push_back(token); // Optional: handle assignment
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

#include <regex> // For parsing function name and arity

ASTNode* buildAST(const std::vector<Token>& postfix) {
    std::stack<ASTNode*> nodeStack;

    for (const Token& token : postfix) {
        if (token.type == TokenType::NUMBER) {
            nodeStack.push(new ASTNode(NodeType::NUMBER, token.value));
        }

        else if (token.type == TokenType::IDENTIFIER) {
            // Check if token is in the form func@N
            std::regex funcRegex(R"((\w+)@(\d+))");
            std::smatch match;

            if (std::regex_match(token.value, match, funcRegex)) {
                std::string funcName = match[1];
                int argCount = std::stoi(match[2]);

                if (nodeStack.size() < argCount) {
                    std::cerr << "Error: Not enough arguments for function '" << funcName << "'\n";
                    return nullptr;
                }

                ASTNode* funcNode = new ASTNode(NodeType::FUNCTION, funcName);
                std::vector<ASTNode*> args;

                for (int i = 0; i < argCount; ++i) {
                    args.push_back(nodeStack.top());
                    nodeStack.pop();
                }

                std::reverse(args.begin(), args.end()); // Maintain argument order
                funcNode->children = args;
                nodeStack.push(funcNode);
            } else {
                // Regular variable
                nodeStack.push(new ASTNode(NodeType::VARIABLE, token.value));
            }
        }

        else if (isOperator(token.type)) {
            if (nodeStack.size() < 2) {
                std::cerr << "Error: Not enough operands for operator '" << token.value << "'\n";
                return nullptr;
            }

            ASTNode* right = nodeStack.top(); nodeStack.pop();
            ASTNode* left  = nodeStack.top(); nodeStack.pop();

            ASTNode* opNode = new ASTNode(NodeType::BINARY_OP, token.value);
            opNode->children.push_back(left);
            opNode->children.push_back(right);

            nodeStack.push(opNode);
        }
    }

    if (nodeStack.size() != 1) {
        std::cerr << "Error: Invalid AST. Stack size = " << nodeStack.size() << "\n";
        return nullptr;
    }

    return nodeStack.top();
}


void printAST(ASTNode* node, int depth) {
    if (!node) return;
    for (int i = 0; i < depth; ++i) std::cout << "  ";
    std::cout << "- " << node->value << " (" << static_cast<int>(node->type) << ")\n";
    for (ASTNode* child : node->children)
        printAST(child, depth + 1);
}


void freeAST(ASTNode* node) {
    if (!node) return;
    for (ASTNode* child : node->children)
        freeAST(child);
    delete node;
}
