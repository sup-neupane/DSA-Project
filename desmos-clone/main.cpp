#include "parser.h"
#include "evaluator.h"
#include <iostream>
#include <vector>

int main() {
    std::string expr = "-sin(x) + 2^3";

    auto tokens = tokenize(expr);
    auto postfix = toPostfix(tokens);

    std::cout << "Postfix: ";
    for (auto& t : postfix) std::cout << t.value << " ";
    std::cout << "\n";

    auto ast = buildAST(postfix);

    if (ast) {
        double xVal = 2.0;
        double result = evaluate(ast, xVal);
        std::cout << "Result: " << result << "\n";
        freeAST(ast);
    } else {
        std::cerr << "Failed to build AST.\n";
    }

    return 0;
}
