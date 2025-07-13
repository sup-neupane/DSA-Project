#include "../parser/parser.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <string>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <vector>

// Helper to convert degrees to radians (not needed if trig expects radians directly)
inline double deg2rad(double degrees) {
    return degrees * M_PI / 180.0;
}

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
            if (var == "tau") return 2 * M_PI;
            if (var == "phi") return 1.61803398875;
            if (var == "gamma") return 0.5772156649;

            throw std::runtime_error("Unknown variable: " + node->value);
        }

        case NodeType::UNARY_OP: {
            double childVal = evaluate(node->children[0], x);
            if (node->value == "-") return -childVal;
            if (node->value == "+") return +childVal;
            throw std::runtime_error("Unknown unary operator: " + node->value);
        }

        case NodeType::BINARY_OP: {
            double leftVal = evaluate(node->children[0], x);
            double rightVal = evaluate(node->children[1], x);

            if (node->value == "+") return leftVal + rightVal;
            if (node->value == "-") return leftVal - rightVal;
            if (node->value == "*") return leftVal * rightVal;
            if (node->value == "/") {
                if (std::fabs(rightVal) < EPSILON)
                    throw std::runtime_error("Divide by zero");
                return leftVal / rightVal;
            }
            if (node->value == "^") {
                if (leftVal == 0 && rightVal < 0)
                    throw std::runtime_error("Zero to negative power");
                if (leftVal < 0 && std::floor(rightVal) != rightVal)
                    throw std::runtime_error("Negative base with non-integer exponent");
                return std::pow(leftVal, rightVal);
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

            // Trig functions (args in radians)
            if (func == "sin") return std::sin(args[0]);
            if (func == "cos") return std::cos(args[0]);
            if (func == "tan") {
                double angleMod = std::fmod(args[0] * 180.0 / M_PI, 180.0);
                if (std::fabs(angleMod - 90.0) < EPSILON)
                    throw std::runtime_error("tan undefined at 90 + k*180 degrees");
                return std::tan(args[0]);
            }
            if (func == "cot") {
                double angleMod = std::fmod(args[0] * 180.0 / M_PI, 180.0);
                if (std::fabs(angleMod) < EPSILON)
                    throw std::runtime_error("cot undefined at k*180 degrees");
                return 1.0 / std::tan(args[0]);
            }
            if (func == "sec") {
                if (std::fabs(std::cos(args[0])) < EPSILON)
                    throw std::runtime_error("sec undefined at 90 + k*180 degrees");
                return 1.0 / std::cos(args[0]);
            }
            if (func == "csc") {
                if (std::fabs(std::sin(args[0])) < EPSILON)
                    throw std::runtime_error("csc undefined at k*180 degrees");
                return 1.0 / std::sin(args[0]);
            }

            // Single-arg functions
            if (func == "sqrt") {
                if (args[0] < 0) throw std::runtime_error("sqrt of negative");
                return std::sqrt(args[0]);
            }
            if (func == "abs") return std::fabs(args[0]);
            if (func == "sign") return (args[0] > 0) - (args[0] < 0);
            if (func == "floor") return std::floor(args[0]);
            if (func == "ceil") return std::ceil(args[0]);
            if (func == "round") return std::round(args[0]);
            if (func == "log" || func == "ln") {
                if (args[0] <= 0) throw std::runtime_error("log of non-positive");
                return std::log(args[0]);
            }
            if (func == "log10") {
                if (args[0] <= 0) throw std::runtime_error("log10 of non-positive");
                return std::log10(args[0]);
            }
            if (func == "log2") {
                if (args[0] <= 0) throw std::runtime_error("log2 of non-positive");
                return std::log2(args[0]);
            }
            if (func == "exp") return std::exp(args[0]);

            // Two-arg functions
            if (func == "pow") {
                if (args.size() != 2) throw std::runtime_error("pow requires 2 args");
                return std::pow(args[0], args[1]);
            }
            if (func == "mod" && args.size() == 2) {
                if (std::fabs(args[1]) < EPSILON)
                    throw std::runtime_error("mod by zero");
                return std::fmod(args[0], args[1]);
            }
            if (func == "log" && args.size() == 2) {
                if (args[0] <= 0 || args[1] <= 0 || args[1] == 1)
                    throw std::runtime_error("invalid log base");
                return std::log(args[0]) / std::log(args[1]);
            }
            if (func == "atan2") {
                if (args.size() != 2) throw std::runtime_error("atan2 needs 2 args");
                return std::atan2(args[0], args[1]);
            }

            // Variadic functions
            if (func == "max") {
                if (args.empty()) throw std::runtime_error("max needs args");
                return *std::max_element(args.begin(), args.end());
            }
            if (func == "min") {
                if (args.empty()) throw std::runtime_error("min needs args");
                return *std::min_element(args.begin(), args.end());
            }

            throw std::runtime_error("Unknown function: " + func);
        }

        default:
            throw std::runtime_error("Unsupported AST node type.");
    }
}
