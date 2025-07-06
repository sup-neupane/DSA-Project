#include "parser.h"
#include "evaluator.h"
#include <iostream>
#include <string>
#include <cmath>

void testExpression(const std::string& expr, double xVal) {
    std::cout << "\n=============================\n";
    std::cout << "Expression: " << expr << "\n";

    auto tokens = tokenize(expr);
    auto postfix = toPostfix(tokens);
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
    double xVal = 2;

    // Test new constants
    testExpression("phi + tau + gamma", xVal);

    // Test new unary functions
    testExpression("exp(1)", xVal);
    testExpression("floor(3.7)", xVal);
    testExpression("ceil(3.3)", xVal);
    testExpression("round(3.5)", xVal);

    // Test error handling
    testExpression("sqrt(-4)", xVal);
    testExpression("log(0)", xVal);
    testExpression("1 / 0", xVal);
    testExpression("pow(0, -1)", xVal);
    testExpression("unknownVar + 1", xVal);
    testExpression("unknownFunc(1)", xVal);

    return 0;
}
