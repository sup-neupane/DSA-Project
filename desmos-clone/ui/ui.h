#ifndef UI_H
#define UI_H

#include <string>
#include <vector>


void runUI();

struct ExpressionEntry {
    std::string expr;
    std::vector<struct Token> tokens;
    struct ASTNode* ast;
    bool valid;
    std::string error;
};

#endif
