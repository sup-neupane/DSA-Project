#include "parser.h"
#include <iostream>
#include <vector>

void testExpression(const std::string& expr) {
    std::cout << "=============================\n";
    std::cout << "Expression: " << expr << "\n";

    // Tokenize
    std::vector<Token> tokens = tokenize(expr);

    // Convert to postfix
    std::vector<Token> postfix = toPostfix(tokens);

    // Build AST
    ASTNode* astRoot = buildAST(postfix);
    if (!astRoot) {
        std::cerr << "Failed to build AST for expression: " << expr << "\n";
        return;
    }

    // Print AST
    std::cout << "AST:\n";
    printAST(astRoot);

    // Free AST memory
    freeAST(astRoot);
    std::cout << "=============================\n\n";
}

int main() {
    std::vector<std::string> expressions = {
        "-x",
        "-5 + x",
        "-sin(x)",
        "max(-1, -2, 3)"
    };

    for (const auto& expr : expressions) {
        testExpression(expr);
    }

    return 0;
}
