#include "parser.h"
#include <iostream>
#include <vector>

int main() {
    std::string expr = "max(1, 2, 3) + cos(sin(x ^ 2))";

    std::cout << "=============================\n";
    std::cout << "      Parsing Expression     \n";
    std::cout << "=============================\n\n";

    std::cout << "Input Expression:\n" << expr << "\n\n";

    // Tokenize
    std::vector<Token> tokens = tokenize(expr);

    // Convert to postfix
    std::vector<Token> postfix = toPostfix(tokens);

    // Build AST
    ASTNode* astRoot = buildAST(postfix);
    if (!astRoot) {
        std::cerr << "Failed to build AST\n";
        return 1;
    }

    // Print AST
    std::cout << "Parse Tree:\n";
    printAST(astRoot);

    // Free AST memory
    freeAST(astRoot);

    std::cout << "\n=============================\n";
    std::cout << "          Done               \n";
    std::cout << "=============================\n";

    return 0;
}
