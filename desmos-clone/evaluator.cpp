#include "parser.h"
#include <cmath>
#include <string>
#include <iostream>
#include <stdexcept>
#include <algorithm>

double evaluate(ASTNode* node, double x) {
    if (!node) throw std::runtime_error("Null node in AST");

    const double EPSILON = 1e-12;

    switch (node->type) {
        case NodeType::NUMBER:
            return std::stod(node->value);

        case NodeType::VARIABLE: {
            std::string var = node->value;
            std::transform(var.begin(), var.end(), var.begin(), ::tolower);
            if (var == "x") return x;
            if (var == "pi") return M_PI;
            if (var == "e") return M_E;
            if (var == "phi") return 1.61803398875; // golden ratio
            if (var == "tau") return 2 * M_PI;
            if (var == "gamma") return 0.5772156649; // Euler–Mascheroni constant
            throw std::runtime_error("Unknown variable: " + node->value);
        }

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
                if (std::fabs(right) < EPSILON)
                    throw std::runtime_error("Divide by zero or near zero");
                return left / right;
            }

            if (node->value == "^") {
                if (left == 0 && right < 0)
                    throw std::runtime_error("Zero cannot be raised to a negative power");
                if (left < 0 && std::floor(right) != right)
                    throw std::runtime_error("Negative base with non-integer exponent is undefined");
                return std::pow(left, right);
            }

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
            if (func == "tan") {
                if (std::fabs(std::cos(args[0])) < EPSILON)
                    throw std::runtime_error("tan undefined near odd multiples of pi/2");
                return std::tan(args[0]);
            }
            if (func == "sqrt") {
                if (args[0] < 0)
                    throw std::runtime_error("sqrt of negative number");
                return std::sqrt(args[0]);
            }
            if (func == "abs") return std::fabs(args[0]);
            if (func == "log") {
                if (args[0] <= 0)
                    throw std::runtime_error("log of non-positive number");
                return std::log(args[0]);
            }
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
                double base = args[0], exponent = args[1];
                if (base == 0 && exponent < 0)
                    throw std::runtime_error("Zero cannot be raised to a negative power");
                if (base < 0 && std::floor(exponent) != exponent)
                    throw std::runtime_error("Negative base with non-integer exponent is undefined");
                return std::pow(base, exponent);
            }
            if (func == "exp") return std::exp(args[0]);
            if (func == "floor") return std::floor(args[0]);
            if (func == "ceil") return std::ceil(args[0]);
            if (func == "round") return std::round(args[0]);

            throw std::runtime_error("Unknown function: " + func);
        }

        default:
            throw std::runtime_error("Unsupported AST node type.");
    }
}
