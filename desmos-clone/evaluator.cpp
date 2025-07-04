#include "parser.h"
#include <cmath>
#include <string>
#include <iostream>
#include <stdexcept>
#include <algorithm>

double evaluate(ASTNode* node, double x) {
    if (!node) throw std::runtime_error("Null node in AST");

    switch (node->type) {
        case NodeType::NUMBER:
            return std::stod(node->value);

        case NodeType::VARIABLE:
            if (node->value == "x") return x;
            if (node->value == "pi") return M_PI;
            if (node->value == "e") return M_E;
            throw std::runtime_error("Unknown variable: " + node->value);

        case NodeType::UNARY_OP: {
            double child = evaluate(node->children[0], x);
            if (node->value == "-") return -child;
            if (node->value == "+") return +child;
            throw std::runtime_error("Unknown unary operator: " + node->value);
        }

        case NodeType::BINARY_OP: {
            double left = evaluate(node->children[0], x);
            double right = evaluate(node->children[1], x);
            if (node->value == "+") return left + right;
            if (node->value == "-") return left - right;
            if (node->value == "*") return left * right;
            if (node->value == "/") {
                if (right == 0) throw std::runtime_error("Divide by zero");
                return left / right;
            }
            if (node->value == "^") return std::pow(left, right);
            throw std::runtime_error("Unknown binary operator: " + node->value);
        }

        case NodeType::FUNCTION: {
            std::string func = node->value;
            std::transform(func.begin(), func.end(), func.begin(), ::tolower);
            std::vector<double> args;
            for (ASTNode* arg : node->children) {
                args.push_back(evaluate(arg, x));
            }

            if (func == "sin") return std::sin(args[0]);
            if (func == "cos") return std::cos(args[0]);
            if (func == "tan") return std::tan(args[0]);
            if (func == "sqrt") return std::sqrt(args[0]);
            if (func == "abs") return std::fabs(args[0]);
            if (func == "log") return std::log(args[0]); // natural log
            if (func == "max") {
                if (args.empty()) throw std::runtime_error("max requires at least 1 argument");
                return *std::max_element(args.begin(), args.end());
            }
            if (func == "min") {
                if (args.empty()) throw std::runtime_error("min requires at least 1 argument");
                return *std::min_element(args.begin(), args.end());
            }
            if (func == "pow") {
                if (args.size() != 2) throw std::runtime_error("pow requires 2 arguments");
                return std::pow(args[0], args[1]);
            }

            throw std::runtime_error("Unknown function: " + func);
        }

        default:
            throw std::runtime_error("Unsupported AST node type.");
    }
}
