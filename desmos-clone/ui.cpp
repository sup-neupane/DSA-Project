#include "ui.h"
#include "parser.h"
#include "evaluator.h"
#include "raylib.h"

#include <string>
#include <iostream>
#include <set>
#include <algorithm>
#include <cctype>

// Set of known function names (all lowercase)
const std::set<std::string> knownFunctions = {
    "sin", "cos", "tan", "sqrt", "abs", "log", "max", "min", "pow",
    "exp", "floor", "ceil", "round"
};

// Case-insensitive compare
static bool iequals(const std::string& a, const std::string& b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end(),
                      [](char a, char b) { return tolower(a) == tolower(b); });
}

// Check if input string is a known function name alone (without parentheses)
static bool isKnownFunctionAlone(const std::string& s) {
    for (const auto& fn : knownFunctions) {
        if (iequals(s, fn)) return true;
    }
    return false;
}

void runUI() {
    InitWindow(800, 400, "Desmos Clone - Expression Evaluator");
    SetTargetFPS(60);

    std::string inputText = "";
    std::string outputText = "Enter expression and press Enter";
    std::string xValueStr = "0";

    Rectangle inputBox = { 100, 80, 600, 48 };
    Rectangle xInputBox = { 100, 160, 200, 40 };

    bool inputBoxActive = false;
    bool xBoxActive = false;

    Font font = GetFontDefault();

    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();

        // Detect mouse clicks for input focus
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mousePos, inputBox)) {
                inputBoxActive = true;
                xBoxActive = false;
            } else if (CheckCollisionPointRec(mousePos, xInputBox)) {
                xBoxActive = true;
                inputBoxActive = false;
            } else {
                inputBoxActive = false;
                xBoxActive = false;
            }
        }

        // Capture keyboard input for active box
        int key = GetCharPressed();
        while (key > 0) {
            if (inputBoxActive && key >= 32 && key <= 125 && inputText.size() < 256) {
                inputText.push_back((char)key);
            }
            else if (xBoxActive && key >= 32 && key <= 125 && xValueStr.size() < 16) {
                xValueStr.push_back((char)key);
            }
            key = GetCharPressed();
        }

        // Handle backspace
        if (inputBoxActive && IsKeyPressed(KEY_BACKSPACE) && !inputText.empty()) {
            inputText.pop_back();
        }
        if (xBoxActive && IsKeyPressed(KEY_BACKSPACE) && !xValueStr.empty()) {
            xValueStr.pop_back();
        }

        // Evaluate expression on Enter key
        if ((inputBoxActive || xBoxActive) && IsKeyPressed(KEY_ENTER)) {
            if (inputText.empty()) {
                outputText = "Error: Input expression is empty!";
            } else {
                // Parse x value
                double xVal = 0.0;
                try {
                    if (!xValueStr.empty()) {
                        xVal = std::stod(xValueStr);
                    }
                } catch (...) {
                    outputText = "Error: Invalid x value";
                    goto DRAW_UI;
                }

                // Tokenize
                auto tokens = tokenize(inputText);
                if (tokens.empty()) {
                    outputText = "Error: Failed to tokenize input";
                    goto DRAW_UI;
                }

                // Convert to postfix
                auto postfix = toPostfix(tokens);
                if (postfix.empty()) {
                    outputText = "Error: Failed to convert to postfix notation";
                    goto DRAW_UI;
                }

                // Build AST
                ASTNode* ast = buildAST(postfix);
                if (!ast) {
                    outputText = "Error: Failed to build AST";
                    goto DRAW_UI;
                }

                // Check if AST root is a variable that's actually a function name alone (no args)
                if (ast->type == NodeType::VARIABLE) {
                    if (isKnownFunctionAlone(ast->value)) {
                        outputText = "Error: Function '" + ast->value + "' missing parentheses and arguments";
                        freeAST(ast);
                        goto DRAW_UI;
                    }
                }

                // Evaluate expression with x
                try {
                    double result = evaluate(ast, xVal);
                    outputText = "Result: " + std::to_string(result);
                }
                catch (const std::exception& e) {
                    outputText = std::string("Error: ") + e.what();
                }

                freeAST(ast);
            }
        }

    DRAW_UI:
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Title text
        DrawTextEx(font, "Desmos Clone - Expression Evaluator", {100, 20}, 28, 2, DARKGRAY);

        // Input expression box
        Color inputBgColor = inputBoxActive ? LIGHTGRAY : (Color){230,230,230,255};
        DrawRectangleRounded(inputBox, 0.15f, 8, inputBgColor);
        DrawRectangleRoundedLines(inputBox, 0.15f, 8, inputBoxActive ? BLUE : DARKGRAY);

        if (inputText.empty() && !inputBoxActive) {
            DrawTextEx(font, "Enter expression, e.g. sin(x) + 2", {inputBox.x + 12, inputBox.y + 12}, 24, 1, GRAY);
        }
        else {
            DrawTextEx(font, inputText.c_str(), {inputBox.x + 12, inputBox.y + 12}, 24, 1, BLACK);
        }

        // Blinking cursor for input box
        if (inputBoxActive) {
            int cursorX = inputBox.x + 12 + MeasureTextEx(font, inputText.c_str(), 24, 1).x;
            if ((GetTime() - (int)GetTime()) < 0.5f) {
                DrawLine(cursorX, inputBox.y + 10, cursorX, inputBox.y + 38, BLACK);
            }
        }

        // x value input box with label
        DrawText("x =", 30, 165, 26, DARKGRAY);
        Color xBoxBg = xBoxActive ? LIGHTGRAY : (Color){240,240,240,255};
        DrawRectangleRounded(xInputBox, 0.15f, 6, xBoxBg);
        DrawRectangleRoundedLines(xInputBox, 0.15f, 6, xBoxActive ? BLUE : DARKGRAY);

        if (xValueStr.empty() && !xBoxActive) {
            DrawTextEx(font, "0", {xInputBox.x + 8, xInputBox.y + 8}, 22, 1, GRAY);
        }
        else {
            DrawTextEx(font, xValueStr.c_str(), {xInputBox.x + 8, xInputBox.y + 8}, 22, 1, BLACK);
        }

        // Blinking cursor for x input box
        if (xBoxActive) {
            int cursorX = xInputBox.x + 8 + MeasureTextEx(font, xValueStr.c_str(), 22, 1).x;
            if ((GetTime() - (int)GetTime()) < 0.5f) {
                DrawLine(cursorX, xInputBox.y + 7, cursorX, xInputBox.y + 33, BLACK);
            }
        }

        // Output / result message
        DrawTextEx(font, outputText.c_str(), {100, 230}, 26, 1.2f, DARKGRAY);

        // Instructions
        DrawTextEx(font, "Press Enter to evaluate | Click input boxes to type", {100, 280}, 18, 1, GRAY);

        EndDrawing();
    }

    CloseWindow();
}
