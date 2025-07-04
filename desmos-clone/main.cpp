#include "parser.h"
#include "evaluator.h"
#include <iostream>
#include <vector>
#include <string>
#include <cmath>  // for M_PI

void testExpression(const std::string& expr, double xVal) {
    std::cout << "\n=============================\n";
    std::cout << "Expression: " << expr << "\n";

    auto tokens = tokenize(expr);
    auto postfix = toPostfix(tokens);

    std::cout << "Postfix: ";
    for (const auto& t : postfix) std::cout << t.value << " ";
    std::cout << "\n";

    auto ast = buildAST(postfix);

    if (ast) {
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
    double xVal = M_PI / 2;  // pi/2

    testExpression("x / cos(x)", xVal);
    testExpression("x^2 + 2*x + 1", xVal);

    return 0;
}
