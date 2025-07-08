#include "ui.h"
#include "parser.h"
#include "evaluator.h"
#include "raylib.h"

#include <vector>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <iostream>

// --- UI Constants ---
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;
const int LEFT_PANEL_WIDTH = 350;
const int HEADER_HEIGHT = 60;
const int EXPRESSION_HEIGHT = 50;
const int EXPRESSION_MARGIN = 8;
const int BUTTON_SIZE = 30;

// --- Colors ---
const Color DESMOS_BLUE = {21, 101, 192, 255};
const Color PANEL_BG = {248, 249, 250, 255};
const Color EXPRESSION_BG = {255, 255, 255, 255};
const Color BORDER_COLOR = {228, 230, 235, 255};
const Color TEXT_COLOR = {55, 53, 47, 255};
const Color PLACEHOLDER_COLOR = {156, 163, 175, 255};
const Color GRAPH_BG = {255, 255, 255, 255};

// Expression colors (Desmos-style)
static Color expressionColors[] = {
    {194, 48, 48, 255},    // Red
    {31, 120, 180, 255},   // Blue  
    {51, 160, 44, 255},    // Green
    {227, 26, 28, 255},    // Dark red
    {255, 127, 0, 255},    // Orange
    {106, 61, 154, 255},   // Purple
    {177, 89, 40, 255},    // Brown
    {166, 206, 227, 255}   // Light blue
};
static int maxColors = sizeof(expressionColors) / sizeof(Color);

// --- Expression struct ---
struct Expression {
    std::string text;
    bool isActive;
    bool isVisible;
    Color color;
    ASTNode* ast;
    
    Expression(const std::string& t, Color c) : text(t), isActive(false), isVisible(true), color(c), ast(nullptr) {}
};

// --- Viewport struct for coordinate transforms ---
struct Viewport {
    double xMin = -10.0, xMax = 10.0;
    double yMin = -10.0, yMax = 10.0;
    int screenX, screenY, screenW, screenH;

    double screenToWorldX(int px) const {
        return xMin + (double)(px - screenX) / screenW * (xMax - xMin);
    }

    int worldToScreenX(double wx) const {
        double frac = (wx - xMin) / (xMax - xMin);
        return screenX + (int)(frac * screenW);
    }

    double screenToWorldY(int py) const {
        double frac = (double)(screenY + screenH - py) / screenH;
        return yMin + frac * (yMax - yMin);
    }

    int worldToScreenY(double wy) const {
        double frac = (wy - yMin) / (yMax - yMin);
        return screenY + screenH - (int)(frac * screenH);
    }
};

// --- Global viewport ---
static Viewport viewport;

// --- UI Helper Functions ---
void DrawRoundedRect(int x, int y, int width, int height, float roundness, Color color) {
    DrawRectangleRounded({(float)x, (float)y, (float)width, (float)height}, roundness, 6, color);
}

void DrawRoundedRectLines(int x, int y, int width, int height, float roundness, Color color) {
    DrawRectangleRoundedLines({(float)x, (float)y, (float)width, (float)height}, roundness, 6, color);
}

bool IsMouseOverRect(int x, int y, int width, int height) {
    Vector2 mousePos = GetMousePosition();
    return mousePos.x >= x && mousePos.x <= x + width && 
           mousePos.y >= y && mousePos.y <= y + height;
}

// --- Parse expression helper ---
void parseExpression(Expression& expr) {
    if (expr.ast) {
        freeAST(expr.ast);
        expr.ast = nullptr;
    }
    try {
        auto tokens = tokenize(expr.text);
        auto postfix = toPostfix(tokens);
        ASTNode* ast = buildAST(postfix);
        if (!ast) throw std::runtime_error("Parse failed");
        expr.ast = ast;
    } catch (const std::exception& e) {
        std::cerr << "Parse error in expression '" << expr.text << "': " << e.what() << std::endl;
        expr.ast = nullptr;
    }
}

// --- UI Drawing Functions ---
void DrawHeader() {
    // Header background
    DrawRectangle(0, 0, WINDOW_WIDTH, HEADER_HEIGHT, DESMOS_BLUE);
    
    // Desmos logo/title
    DrawText("Graphing Calculator", 20, 20, 24, WHITE);
    
    // Header separator
    DrawLine(0, HEADER_HEIGHT, WINDOW_WIDTH, HEADER_HEIGHT, BORDER_COLOR);
}

void DrawLeftPanel(std::vector<Expression>& expressions, int& activeExpression, char* inputBuffer, int inputLength) {
    // Panel background
    DrawRectangle(0, HEADER_HEIGHT, LEFT_PANEL_WIDTH, WINDOW_HEIGHT - HEADER_HEIGHT, PANEL_BG);
    
    // Panel border
    DrawLine(LEFT_PANEL_WIDTH, HEADER_HEIGHT, LEFT_PANEL_WIDTH, WINDOW_HEIGHT, BORDER_COLOR);
    
    // Expressions list
    int yPos = HEADER_HEIGHT + 20;
    
    for (size_t i = 0; i < expressions.size(); ++i) {
        Expression& expr = expressions[i];
        
        // Expression container
        bool isHovered = IsMouseOverRect(10, yPos, LEFT_PANEL_WIDTH - 20, EXPRESSION_HEIGHT);
        Color bgColor = (activeExpression == (int)i) ? WHITE : (isHovered ? WHITE : EXPRESSION_BG);
        
        DrawRoundedRect(10, yPos, LEFT_PANEL_WIDTH - 20, EXPRESSION_HEIGHT, 0.1f, bgColor);
        
        if (activeExpression == (int)i || isHovered) {
            DrawRoundedRectLines(10, yPos, LEFT_PANEL_WIDTH - 20, EXPRESSION_HEIGHT, 0.1f, BORDER_COLOR);
        }
        
        // Color indicator
        DrawCircle(25, yPos + EXPRESSION_HEIGHT/2, 8, expr.color);
        
        // Expression text
        const char* displayText = expr.text.empty() ? "Enter an equation..." : expr.text.c_str();
        Color textColor = expr.text.empty() ? PLACEHOLDER_COLOR : TEXT_COLOR;
        
        DrawText(displayText, 45, yPos + 15, 18, textColor);
        
        // Visibility toggle (eye icon)
        Rectangle eyeBtn = {(float)(LEFT_PANEL_WIDTH - 40), (float)(yPos + 10), 30, 30};
        bool eyeHovered = CheckCollisionPointRec(GetMousePosition(), eyeBtn);
        
        if (eyeHovered) {
            DrawRoundedRect((int)eyeBtn.x, (int)eyeBtn.y, (int)eyeBtn.width, (int)eyeBtn.height, 0.2f, BORDER_COLOR);
        }
        
        // Simple eye icon (you can replace with actual icon)
        DrawText(expr.isVisible ? "👁" : "👁‍🗨", (int)eyeBtn.x + 5, (int)eyeBtn.y + 5, 16, TEXT_COLOR);
        
        yPos += EXPRESSION_HEIGHT + EXPRESSION_MARGIN;
    }
    
    // Add new expression button
    if (yPos < WINDOW_HEIGHT - 100) {
        bool addBtnHovered = IsMouseOverRect(10, yPos, LEFT_PANEL_WIDTH - 20, EXPRESSION_HEIGHT);
        Color addBtnColor = addBtnHovered ? WHITE : EXPRESSION_BG;
        
        DrawRoundedRect(10, yPos, LEFT_PANEL_WIDTH - 20, EXPRESSION_HEIGHT, 0.1f, addBtnColor);
        DrawRoundedRectLines(10, yPos, LEFT_PANEL_WIDTH - 20, EXPRESSION_HEIGHT, 0.1f, BORDER_COLOR);
        
        DrawText("+ Add Expression", 25, yPos + 15, 18, DESMOS_BLUE);
        
        // Handle click
        if (addBtnHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Color newColor = expressionColors[expressions.size() % maxColors];
            expressions.push_back(Expression("", newColor));
            activeExpression = expressions.size() - 1;
        }
    }
    
    // Settings panel at bottom
    int settingsY = WINDOW_HEIGHT - 200;
    DrawLine(10, settingsY, LEFT_PANEL_WIDTH - 10, settingsY, BORDER_COLOR);
    
    DrawText("Settings", 20, settingsY + 10, 16, TEXT_COLOR);
    
    // Zoom controls
    DrawText("Zoom:", 20, settingsY + 40, 14, TEXT_COLOR);
    
    Rectangle zoomInBtn = {80, (float)(settingsY + 35), 25, 25};
    Rectangle zoomOutBtn = {110, (float)(settingsY + 35), 25, 25};
    Rectangle resetBtn = {140, (float)(settingsY + 35), 50, 25};
    
    bool zoomInHovered = CheckCollisionPointRec(GetMousePosition(), zoomInBtn);
    bool zoomOutHovered = CheckCollisionPointRec(GetMousePosition(), zoomOutBtn);
    bool resetHovered = CheckCollisionPointRec(GetMousePosition(), resetBtn);
    
    DrawRoundedRect((int)zoomInBtn.x, (int)zoomInBtn.y, (int)zoomInBtn.width, (int)zoomInBtn.height, 0.2f, 
                   zoomInHovered ? BORDER_COLOR : EXPRESSION_BG);
    DrawRoundedRect((int)zoomOutBtn.x, (int)zoomOutBtn.y, (int)zoomOutBtn.width, (int)zoomOutBtn.height, 0.2f, 
                   zoomOutHovered ? BORDER_COLOR : EXPRESSION_BG);
    DrawRoundedRect((int)resetBtn.x, (int)resetBtn.y, (int)resetBtn.width, (int)resetBtn.height, 0.2f, 
                   resetHovered ? BORDER_COLOR : EXPRESSION_BG);
    
    DrawText("+", (int)zoomInBtn.x + 8, (int)zoomInBtn.y + 5, 16, TEXT_COLOR);
    DrawText("-", (int)zoomOutBtn.x + 9, (int)zoomOutBtn.y + 5, 16, TEXT_COLOR);
    DrawText("Reset", (int)resetBtn.x + 8, (int)resetBtn.y + 5, 12, TEXT_COLOR);
}

void DrawGraphArea(const Viewport& viewport) {
    int graphX = LEFT_PANEL_WIDTH + 20;
    int graphY = HEADER_HEIGHT + 60;
    int graphW = WINDOW_WIDTH - LEFT_PANEL_WIDTH - 40;
    int graphH = WINDOW_HEIGHT - HEADER_HEIGHT - 80;

    DrawRectangle(graphX, graphY, graphW, graphH, GRAPH_BG);
    DrawRectangleLines(graphX, graphY, graphW, graphH, BORDER_COLOR);

    // Draw axes (x=0 and y=0 lines)
    int zeroX = viewport.worldToScreenX(0);
    int zeroY = viewport.worldToScreenY(0);

    DrawLine(zeroX, graphY, zeroX, graphY + graphH, LIGHTGRAY);
    DrawLine(graphX, zeroY, graphX + graphW, zeroY, LIGHTGRAY);

    const int numPoints = 800;
    double step = (viewport.xMax - viewport.xMin) / numPoints;

    for (const Expression& expr : *(std::vector<Expression>*)nullptr) {
        // This cast is temporary to be replaced by correct param — replaced below
    }
}

// Overload with expressions vector (fix for the above)
void DrawGraphArea(const std::vector<Expression>& expressions) {
    int graphX = LEFT_PANEL_WIDTH + 20;
    int graphY = HEADER_HEIGHT + 60;
    int graphW = WINDOW_WIDTH - LEFT_PANEL_WIDTH - 40;
    int graphH = WINDOW_HEIGHT - HEADER_HEIGHT - 80;

    DrawRectangle(graphX, graphY, graphW, graphH, GRAPH_BG);
    DrawRectangleLines(graphX, graphY, graphW, graphH, BORDER_COLOR);

    // Draw axes (x=0 and y=0 lines)
    int zeroX = viewport.worldToScreenX(0);
    int zeroY = viewport.worldToScreenY(0);

    DrawLine(zeroX, graphY, zeroX, graphY + graphH, LIGHTGRAY);
    DrawLine(graphX, zeroY, graphX + graphW, zeroY, LIGHTGRAY);

    const int numPoints = 800;
    double step = (viewport.xMax - viewport.xMin) / numPoints;

    for (const Expression& expr : expressions) {
        if (!expr.isVisible || expr.ast == nullptr) continue;

        bool hasPrev = false;
        int prevX = 0, prevY = 0;

        for (int i = 0; i <= numPoints; ++i) {
            double wx = viewport.xMin + i * step;
            double wy;

            try {
                wy = evaluate(expr.ast, wx);
            } catch (...) {
                hasPrev = false;  // skip bad points
                continue;
            }

            // Skip points outside viewport y range for optimization
            if (wy < viewport.yMin || wy > viewport.yMax) {
                hasPrev = false;
                continue;
            }

            int sx = viewport.worldToScreenX(wx);
            int sy = viewport.worldToScreenY(wy);

            if (hasPrev) {
                DrawLine(prevX, prevY, sx, sy, expr.color);
            }

            prevX = sx;
            prevY = sy;
            hasPrev = true;
        }
    }
}

void HandleExpressionInput(std::vector<Expression>& expressions, int activeExpression, char* inputBuffer, int& inputLength) {
    if (activeExpression < 0 || activeExpression >= (int)expressions.size()) return;
    
    // Handle character input
    int key = GetCharPressed();
    while (key > 0 && inputLength < 255) {
        if (key >= 32 && key <= 125) {
            inputBuffer[inputLength++] = (char)key;
            inputBuffer[inputLength] = '\0';
            expressions[activeExpression].text = std::string(inputBuffer);
        }
        key = GetCharPressed();
    }
    
    // Handle backspace
    if (IsKeyPressed(KEY_BACKSPACE) && inputLength > 0) {
        inputLength--;
        inputBuffer[inputLength] = '\0';
        expressions[activeExpression].text = std::string(inputBuffer);
    }
    
    // Handle Enter key
    if (IsKeyPressed(KEY_ENTER)) {
        parseExpression(expressions[activeExpression]);
        // You may optionally deactivate editing here:
        // activeExpression = -1;
    }
}

void runUI() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Desmos Clone");
    SetTargetFPS(60);
    
    viewport.screenX = LEFT_PANEL_WIDTH;
    viewport.screenY = HEADER_HEIGHT;
    viewport.screenW = WINDOW_WIDTH - LEFT_PANEL_WIDTH;
    viewport.screenH = WINDOW_HEIGHT - HEADER_HEIGHT;
    viewport.xMin = -10;
    viewport.xMax = 10;
    viewport.yMin = -10;
    viewport.yMax = 10;
    
    std::vector<Expression> expressions;
    int activeExpression = -1;
    
    // Text input state
    char inputBuffer[256] = {0};
    int inputLength = 0;
    
    // Add initial empty expression
    expressions.push_back(Expression("", expressionColors[0]));
    activeExpression = 0;
    
    // Parse initial expressions
    for (auto& expr : expressions) {
        parseExpression(expr);
    }
    
    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();
        
        // Zoom controls handling
        int settingsY = WINDOW_HEIGHT - 200;
        Rectangle zoomInBtn = {80, (float)(settingsY + 35), 25, 25};
        Rectangle zoomOutBtn = {110, (float)(settingsY + 35), 25, 25};
        Rectangle resetBtn = {140, (float)(settingsY + 35), 50, 25};
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mousePos, zoomInBtn)) {
                double xCenter = (viewport.xMin + viewport.xMax) / 2;
                double yCenter = (viewport.yMin + viewport.yMax) / 2;
                double xRange = (viewport.xMax - viewport.xMin) * 0.75;
                double yRange = (viewport.yMax - viewport.yMin) * 0.75;
                viewport.xMin = xCenter - xRange / 2;
                viewport.xMax = xCenter + xRange / 2;
                viewport.yMin = yCenter - yRange / 2;
                viewport.yMax = yCenter + yRange / 2;
            }
            else if (CheckCollisionPointRec(mousePos, zoomOutBtn)) {
                double xCenter = (viewport.xMin + viewport.xMax) / 2;
                double yCenter = (viewport.yMin + viewport.yMax) / 2;
                double xRange = (viewport.xMax - viewport.xMin) / 0.75;
                double yRange = (viewport.yMax - viewport.yMin) / 0.75;
                viewport.xMin = xCenter - xRange / 2;
                viewport.xMax = xCenter + xRange / 2;
                viewport.yMin = yCenter - yRange / 2;
                viewport.yMax = yCenter + yRange / 2;
            }
            else if (CheckCollisionPointRec(mousePos, resetBtn)) {
                viewport.xMin = -10;
                viewport.xMax = 10;
                viewport.yMin = -10;
                viewport.yMax = 10;
            }
        }
        
        // Handle expression selection
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int yPos = HEADER_HEIGHT + 20;
            int clickedExpression = -1;
            
            for (size_t i = 0; i < expressions.size(); ++i) {
                if (mousePos.x >= 10 && mousePos.x <= LEFT_PANEL_WIDTH - 10 &&
                    mousePos.y >= yPos && mousePos.y <= yPos + EXPRESSION_HEIGHT) {
                    clickedExpression = (int)i;
                    break;
                }
                yPos += EXPRESSION_HEIGHT + EXPRESSION_MARGIN;
            }
            
            if (clickedExpression != -1) {
                activeExpression = clickedExpression;
                std::string& exprText = expressions[activeExpression].text;
                strncpy(inputBuffer, exprText.c_str(), sizeof(inputBuffer)-1);
                inputBuffer[sizeof(inputBuffer)-1] = '\0';
                inputLength = (int)strlen(inputBuffer);
            }
        }
        
        // Handle visibility toggle clicks
        int eyeYPos = HEADER_HEIGHT + 20;
        for (size_t i = 0; i < expressions.size(); ++i) {
            Rectangle eyeBtn = {(float)(LEFT_PANEL_WIDTH - 40), (float)(eyeYPos + 10), 30, 30};
            if (CheckCollisionPointRec(mousePos, eyeBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                expressions[i].isVisible = !expressions[i].isVisible;
                break;
            }
            eyeYPos += EXPRESSION_HEIGHT + EXPRESSION_MARGIN;
        }
        
        // Input handling if active expression
        if (activeExpression >= 0) {
            HandleExpressionInput(expressions, activeExpression, inputBuffer, inputLength);
        }
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        DrawHeader();
        DrawLeftPanel(expressions, activeExpression, inputBuffer, inputLength);
        DrawGraphArea(expressions);
        
        EndDrawing();
    }
    
    // Cleanup AST nodes
    for (auto& expr : expressions) {
        if (expr.ast) {
            freeAST(expr.ast);
            expr.ast = nullptr;
        }
    }
    
    CloseWindow();
}
