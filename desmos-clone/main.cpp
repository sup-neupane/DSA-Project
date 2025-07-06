#include "parser.h"
#include "evaluator.h"
#include <iostream>
#include <string>
#include <cmath>

void testExpression(const std::string& expr, double xVal) {
    std::cout << "\n=============================\n";
    std::cout << "Expression: " << expr << "\n";

    auto tokens = tokenize(expr);
    std::cout << "Tokens: ";
    for (const auto& t : tokens) std::cout << "[" << t.value << "] ";
    std::cout << "\n";

    auto postfix = toPostfix(tokens);
    std::cout << "Postfix: ";
    for (const auto& t : postfix) std::cout << t.value << " ";
    std::cout << "\n";

    auto ast = buildAST(postfix);
    if (ast) {
        std::cout << "Parse Tree:\n";
        printAST(ast);

        try {
            double result = evaluate(ast, xVal);
            std::cout << "Evaluated Result (x = " << xVal << "): " << result << "\n";
        } catch (const std::exception& e) {
            std::cerr << "Evaluation error: " << e.what() << "\n";
        }

        freeAST(ast);
    } else {
        std::cerr << "Failed to build AST.\n";
    }
}

int main() {
    double xVal = 2;

    testExpression("2x + 3sin(x)", xVal);
    testExpression("x(x + 1)", xVal);
    testExpression("2pi + 3e", xVal);

    return 0;
}
