#include "../parser/parser.h"
#include "../evaluator/evaluator.h"
#include "ui.h"
#include "raylib.h"

#include <vector>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept> 

static Texture2D eyeOpenTex;
static Texture2D eyeClosedTex;
static Texture2D deleteTex;

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
const Color ERROR_COLOR = {255, 60, 60, 255}; // New color for errors

// Expression colors (keep your cycling)
static Color expressionColors[] = {
    {194, 48, 48, 255},
    {31, 120, 180, 255},
    {51, 160, 44, 255},
    {227, 26, 28, 255},
    {255, 127, 0, 255},
    {106, 61, 154, 255},
    {177, 89, 40, 255},
    {166, 206, 227, 255}
};
static int maxColors = sizeof(expressionColors) / sizeof(Color);

// --- Expression struct ---
struct Expression {
    std::string text;
    bool isActive;
    bool isVisible;
    bool valid;            // New: validity flag after parsing/evaluation
    std::string error;     // New: error message string
    Color color;
    ASTNode* ast;
    Expression(const std::string& t, Color c)
        : text(t), isActive(false), isVisible(true), valid(false), error(""), color(c), ast(nullptr) {}
};

// --- Viewport struct ---
struct Viewport {
    double xMin = -10.0, xMax = 10.0, yMin = -10.0, yMax = 10.0;
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
static Viewport viewport;

// --- UI Helper Functions ---
void DrawRoundedRect(int x, int y, int w, int h, float r, Color c) {
    DrawRectangleRounded({(float)x,(float)y,(float)w,(float)h}, r, 6, c);
}
void DrawRoundedRectLines(int x, int y, int w, int h, float r, Color c) {
    DrawRectangleRoundedLines({(float)x,(float)y,(float)w,(float)h}, r, 6, c);
}
bool IsMouseOverRect(int x, int y, int w, int h) {
    Vector2 m = GetMousePosition();
    return m.x >= x && m.x <= x + w && m.y >= y && m.y <= y + h;
}

// --- Expression parsing with error & validity tracking ---
void parseExpression(Expression& expr) {
    if (expr.ast) {
        freeAST(expr.ast);
        expr.ast = nullptr;
    }

    // Strip LHS like f(x)=
    size_t eq = expr.text.find('=');
    if (eq != std::string::npos) {
        expr.text = expr.text.substr(eq + 1);
    }

    expr.error.clear();
    expr.valid = false;

    if (expr.text.empty() || expr.text.back() == '(') return;

    try {
        auto tokens = tokenize(expr.text);
        auto pf = toPostfix(tokens);
        ASTNode* a = buildAST(pf);
        if (!a) throw std::runtime_error("Parse failed");
        expr.ast = a;

        // Test evaluation at 0 to check for immediate runtime errors
        double testVal = evaluate(expr.ast, 0);
        if (std::isnan(testVal) || std::isinf(testVal))
            throw std::runtime_error("Expression evaluates to NaN or Inf");

        expr.valid = true;
    } catch (const std::exception& e) {
        expr.error = e.what();
        if (expr.ast) {
            freeAST(expr.ast);
            expr.ast = nullptr;
        }
        expr.valid = false;
    }
}

// --- Drawing routines ---
void DrawHeader() {
    DrawRectangle(0, 0, WINDOW_WIDTH, HEADER_HEIGHT, DESMOS_BLUE);
    DrawText("Graphing Calculator", 20, 20, 24, WHITE);
    DrawLine(0, HEADER_HEIGHT, WINDOW_WIDTH, HEADER_HEIGHT, BORDER_COLOR);
}
void DrawAddExpressionButton(int yPos) {
    Color btnColor = DESMOS_BLUE;
    bool hover = IsMouseOverRect(10, yPos, LEFT_PANEL_WIDTH - 20, EXPRESSION_HEIGHT);
    
    if (hover) {
        btnColor.r = (unsigned char)(btnColor.r * 0.9f);
        btnColor.g = (unsigned char)(btnColor.g * 0.9f);
        btnColor.b = (unsigned char)(btnColor.b * 0.9f);
    }
    
    DrawRoundedRect(10, yPos, LEFT_PANEL_WIDTH - 20, EXPRESSION_HEIGHT, 0.1f, btnColor);
    
    // Center the text in the button
    const char* text = "Add Expression";
    int textWidth = MeasureText(text, 18);
    DrawText(text, 
            10 + (LEFT_PANEL_WIDTH - 20 - textWidth)/2, 
            yPos + 15, 
            18, WHITE);
}

// --- Updated Left Panel with inline editing support & error display ---
void DrawLeftPanel(std::vector<Expression>& expressions, int& activeExpression) {
    DrawRectangle(0, HEADER_HEIGHT, LEFT_PANEL_WIDTH, WINDOW_HEIGHT - HEADER_HEIGHT, PANEL_BG);
    DrawLine(LEFT_PANEL_WIDTH, HEADER_HEIGHT, LEFT_PANEL_WIDTH, WINDOW_HEIGHT, BORDER_COLOR);

    int yPos = HEADER_HEIGHT + 20;
    Vector2 mousePos = GetMousePosition();
    bool mouseClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    for (size_t i = 0; i < expressions.size(); ++i) {
        Expression& e = expressions[i];
        bool hover = IsMouseOverRect(10, yPos, LEFT_PANEL_WIDTH - 20, EXPRESSION_HEIGHT);
        Color bg = (activeExpression == (int)i) ? WHITE : (hover ? WHITE : EXPRESSION_BG);
        DrawRoundedRect(10, yPos, LEFT_PANEL_WIDTH - 20, EXPRESSION_HEIGHT, 0.1f, bg);
        if (activeExpression == (int)i || hover)
            DrawRoundedRectLines(10, yPos, LEFT_PANEL_WIDTH - 20, EXPRESSION_HEIGHT, 0.1f, BORDER_COLOR);

        DrawCircle(25, yPos + EXPRESSION_HEIGHT / 2, 8, e.color);

        if (activeExpression == (int)i) {
            // Editing mode: show input box
            static char inputBuffer[256] = {0};
            static int inputLength = 0;

            // Initialize inputBuffer on first frame of editing
            static int lastActive = -1;
            if (lastActive != activeExpression) {
                strncpy(inputBuffer, e.text.c_str(), 255);
                inputBuffer[255] = '\0';
                inputLength = (int)strlen(inputBuffer);
                lastActive = activeExpression;
            }

            // Draw text box background
            DrawRoundedRect(45, yPos + 10, LEFT_PANEL_WIDTH - 90, 30, 0.1f, EXPRESSION_BG);
            DrawRoundedRectLines(45, yPos + 10, LEFT_PANEL_WIDTH - 90, 30, 0.1f, BORDER_COLOR);

            // Draw editable text using Raylib GuiTextBox-like logic:
            // We handle input manually here

            // Draw input text
            DrawText(inputBuffer, 50, yPos + 18, 18, TEXT_COLOR);

            // Handle keyboard input while editing
            int c = GetCharPressed();
            while (c > 0 && inputLength < 255) {
                if (c >= 32 && c < 127) {
                    inputBuffer[inputLength++] = (char)c;
                    inputBuffer[inputLength] = '\0';
                }
                c = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && inputLength > 0) {
                inputBuffer[--inputLength] = '\0';
            }

            // Commit on Enter or focus loss (click outside)
            if (IsKeyPressed(KEY_ENTER) || (mouseClicked && !IsMouseOverRect(10, yPos, LEFT_PANEL_WIDTH - 20, EXPRESSION_HEIGHT))) {
                e.text = std::string(inputBuffer);
                parseExpression(e);
                activeExpression = -1; // end editing
            }
        } else {
            // Display mode: show text or placeholder
            const char* disp = e.text.empty() ? "Enter an equation..." : e.text.c_str();
            Color dispColor = e.text.empty() ? PLACEHOLDER_COLOR : TEXT_COLOR;
            DrawText(disp, 45, yPos + 15, 18, dispColor);

            // On click, enter editing mode
            if (hover && mouseClicked) {
                activeExpression = (int)i;
            }
        }

        // Show error below expression if any
        if (!e.valid && !e.error.empty()) {
            DrawText(e.error.c_str(), 45, yPos + EXPRESSION_HEIGHT - 15, 12, ERROR_COLOR);
        }

        // Draw visibility (eye) icon
        Rectangle eye = {(float)(LEFT_PANEL_WIDTH - 40), (float)(yPos + 10), 30, 30};
        bool eyeHover = CheckCollisionPointRec(mousePos, eye);
        if (eyeHover) DrawRoundedRect(eye.x, eye.y, eye.width, eye.height, 0.2f, BORDER_COLOR);
        DrawTexture(e.isVisible ? eyeOpenTex : eyeClosedTex, eye.x + 5, eye.y + 5, WHITE);

        // Draw delete icon
        Rectangle del = {(float)(LEFT_PANEL_WIDTH - 70), (float)(yPos + 10), 25, 30};
        bool delHover = CheckCollisionPointRec(mousePos, del);
        if (delHover) DrawRoundedRect(del.x, del.y, del.width, del.height, 0.2f, BORDER_COLOR);
        DrawTexture(deleteTex, del.x + 2, del.y + 5, WHITE);

        yPos += EXPRESSION_HEIGHT + EXPRESSION_MARGIN;
    }

    int buttonY = yPos;
    DrawAddExpressionButton(buttonY);

    int settingsY = WINDOW_HEIGHT - 200;
    DrawLine(10, settingsY, LEFT_PANEL_WIDTH - 10, settingsY, BORDER_COLOR);
    DrawText("Settings", 20, settingsY + 10, 16, TEXT_COLOR);
    DrawText("Zoom:", 20, settingsY + 40, 14, TEXT_COLOR);

    Rectangle zin = {80, (float)(settingsY+35), 25, 25};
    Rectangle zout = {110, (float)(settingsY+35), 25, 25};
    Rectangle reset = {140, (float)(settingsY+35), 50, 25};

    bool zin_h = CheckCollisionPointRec(GetMousePosition(), zin);
    bool zout_h = CheckCollisionPointRec(GetMousePosition(), zout);
    bool reset_h = CheckCollisionPointRec(GetMousePosition(), reset);

    DrawRoundedRect((int)zin.x,(int)zin.y,(int)zin.width,(int)zin.height,.2f, zin_h ? BORDER_COLOR : EXPRESSION_BG);
    DrawRoundedRect((int)zout.x,(int)zout.y,(int)zout.width,(int)zout.height,.2f, zout_h ? BORDER_COLOR : EXPRESSION_BG);
    DrawRoundedRect((int)reset.x,(int)reset.y,(int)reset.width,(int)reset.height,.2f, reset_h ? BORDER_COLOR : EXPRESSION_BG);

    DrawText("+", (int)zin.x + 8, (int)zin.y + 5, 16, TEXT_COLOR);
    DrawText("-", (int)zout.x + 9, (int)zout.y + 5, 16, TEXT_COLOR);
    DrawText("Reset", (int)reset.x + 8, (int)reset.y + 5, 12, TEXT_COLOR);
}

// --- Full DrawGraphArea with domain clipping and grid labels ---
void DrawGraphArea(const std::vector<Expression>& expressions) {
    int graphX = LEFT_PANEL_WIDTH + 20;
    int graphY = HEADER_HEIGHT + 20;
    int graphW = WINDOW_WIDTH - LEFT_PANEL_WIDTH - 40;
    int graphH = WINDOW_HEIGHT - HEADER_HEIGHT - 40;

    viewport.screenX = graphX;
    viewport.screenY = graphY;
    viewport.screenW = graphW;
    viewport.screenH = graphH;

    DrawRectangle(graphX, graphY, graphW, graphH, GRAPH_BG);
    DrawRectangleLines(graphX, graphY, graphW, graphH, BORDER_COLOR);

    const Color GRID_COLOR = {230,230,230,255};
    const Color AXIS_COLOR = {180,180,180,255};
    const int GRID_SPACING = 1;

    // Draw vertical grid lines and labels
    double xStart = ceil(viewport.xMin / GRID_SPACING) * GRID_SPACING;
    for (double x = xStart; x <= viewport.xMax; x += GRID_SPACING) {
        int sx = viewport.worldToScreenX(x);
        if (fabs(x) > 1e-6) { // Skip axis line (draw separately)
            DrawLine(sx, graphY, sx, graphY + graphH, GRID_COLOR);
        }
    }

    // Draw horizontal grid lines and labels
    double yStart = ceil(viewport.yMin / GRID_SPACING) * GRID_SPACING;
    for (double y = yStart; y <= viewport.yMax; y += GRID_SPACING) {
        int sy = viewport.worldToScreenY(y);
        if (fabs(y) > 1e-6) { // Skip axis line (draw separately)
            DrawLine(graphX, sy, graphX + graphW, sy, GRID_COLOR);
        }
    }

    // Draw axes
    int zeroX = viewport.worldToScreenX(0);
    int zeroY = viewport.worldToScreenY(0);
    if (zeroX >= graphX && zeroX <= graphX + graphW)
        DrawLine(zeroX, graphY, zeroX, graphY + graphH, AXIS_COLOR);
    if (zeroY >= graphY && zeroY <= graphY + graphH)
        DrawLine(graphX, zeroY, graphX + graphW, zeroY, AXIS_COLOR);

    // Draw grid labels
    const int LABEL_OFFSET = 5, LABEL_FONT = 12;
    for (double x = xStart; x <= viewport.xMax; x += GRID_SPACING) {
        if (fabs(x) < 1e-6) continue;
        int sx = viewport.worldToScreenX(x);
        char label[16]; snprintf(label,16,"%.0f",x);
        int w = MeasureText(label, LABEL_FONT);
        DrawText(label, sx - w/2, zeroY + LABEL_OFFSET, LABEL_FONT, TEXT_COLOR);
    }
    for (double y = yStart; y <= viewport.yMax; y += GRID_SPACING) {
        if (fabs(y) < 1e-6) continue;
        int sy = viewport.worldToScreenY(y);
        char label[16]; snprintf(label,16,"%.0f",y);
        DrawText(label, zeroX + LABEL_OFFSET, sy - LABEL_FONT/2, LABEL_FONT, TEXT_COLOR);
    }

    // Plot expressions
    const int numPoints = 1000;
    double step = (viewport.xMax - viewport.xMin) / numPoints;
    for (const auto& expr : expressions) {
        if (!expr.isVisible || expr.ast == nullptr || !expr.valid) continue;
        bool hasPrev = false;
        int pX=0, pY=0;
        for (int i=0; i <= numPoints; i++) {
            double wx = viewport.xMin + i * step;
            double wy;
            try {
                wy = evaluate(expr.ast, wx);
                if (std::isnan(wy) || std::isinf(wy) || wy < viewport.yMin - 1 || wy > viewport.yMax + 1) {
                    hasPrev = false;
                    continue;
                }
            } catch (...) {
                hasPrev = false;
                continue;
            }
            int sx = viewport.worldToScreenX(wx);
            int sy = viewport.worldToScreenY(wy);
            if (hasPrev) {
                for (int off = -1; off <= 1; off++) {
                    DrawLine(pX + off, pY, sx + off, sy, expr.color);
                    DrawLine(pX, pY + off, sx, sy + off, expr.color);
                }
            }
            pX = sx; pY = sy; hasPrev = true;
        }
    }

    // Legend
    if (!expressions.empty()) {
        int legendX = graphX + graphW - 310;
        int legendY = graphY + 10;
        int legendW = 300;
        int legendH = (int)expressions.size() * (EXPRESSION_HEIGHT / 2) + 20;
        DrawRectangle(legendX, legendY, legendW, legendH, {240,240,240,200});
        DrawRectangleLines(legendX, legendY, legendW, legendH, BORDER_COLOR);
        int ty = legendY + 10;
        for (const auto& expr : expressions) {
            if (!expr.isVisible) continue;
            DrawRectangle(legendX + 10, ty + 5, 20, 20, expr.color);
            const char* disp = expr.text.empty() ? "(empty)" : expr.text.c_str();
            char trunc[40];
            if (strlen(disp) > 35) {
                strncpy(trunc, disp, 35);
                trunc[35] = '\0';
                strcat(trunc, "...");
                disp = trunc;
            }
            DrawText(disp, legendX + 40, ty + 10, 16, TEXT_COLOR);
            ty += EXPRESSION_HEIGHT / 2;
        }
    }
}

// --- Pan viewport handler ---
void HandlePan(Viewport& vp) {
    static bool dragging = false;
    static Vector2 lastMouse = {0,0};
    if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)
        || (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && IsKeyDown(KEY_LEFT_CONTROL))) {
        Vector2 m = GetMousePosition();
        if (!dragging) { dragging=true; lastMouse=m; }
        else {
            double dx = m.x - lastMouse.x;
            double dy = m.y - lastMouse.y;
            double wx = dx / vp.screenW * (vp.xMax - vp.xMin);
            double wy = -dy / vp.screenH * (vp.yMax - vp.yMin);
            vp.xMin -= wx; vp.xMax -= wx;
            vp.yMin -= wy; vp.yMax -= wy;
            lastMouse = m;
        }
    } else dragging = false;
}

// --- Main UI loop ---
void runUI() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Graphing Calculator");
    SetTargetFPS(60);

    // Load icon textures
    eyeOpenTex = LoadTexture("assets/eye_open.png");
    eyeClosedTex = LoadTexture("assets/eye_closed.png");
    deleteTex = LoadTexture("assets/delete.png");

    viewport.screenX = LEFT_PANEL_WIDTH;
    viewport.screenY = HEADER_HEIGHT;
    viewport.screenW = WINDOW_WIDTH - LEFT_PANEL_WIDTH;
    viewport.screenH = WINDOW_HEIGHT - HEADER_HEIGHT;

    std::vector<Expression> expressions;
    int activeExpression = -1;

    expressions.emplace_back("", expressionColors[0]);
    activeExpression = -1;

    while (!WindowShouldClose()) {
        Vector2 mp = GetMousePosition();
        HandlePan(viewport);

        // Zoom tiles
        int sy = WINDOW_HEIGHT - 200;
        Rectangle zin = {80, (float)(sy+35),25,25};
        Rectangle zout = {110,(float)(sy+35),25,25};
        Rectangle reset = {140,(float)(sy+35),50,25};
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            if (CheckCollisionPointRec(mp, zin)) {
                double xc=(viewport.xMin+viewport.xMax)/2;
                double yc=(viewport.yMin+viewport.yMax)/2;
                double xr=(viewport.xMax-viewport.xMin)*.75;
                double yr=(viewport.yMax-viewport.yMin)*.75;
                viewport.xMin=xc-xr/2; viewport.xMax=xc+xr/2;
                viewport.yMin=yc-yr/2; viewport.yMax=yc+yr/2;
            }
            else if (CheckCollisionPointRec(mp, zout)) {
                double xc=(viewport.xMin+viewport.xMax)/2;
                double yc=(viewport.yMin+viewport.yMax)/2;
                double xr=(viewport.xMax-viewport.xMin)/.75;
                double yr=(viewport.yMax-viewport.yMin)/.75;
                viewport.xMin=xc-xr/2; viewport.xMax=xc+xr/2;
                viewport.yMin=yc-yr/2; viewport.yMax=yc+yr/2;
            }
            else if (CheckCollisionPointRec(mp, reset)) {
                viewport.xMin=-10; viewport.xMax=10;
                viewport.yMin=-10; viewport.yMax=10;
            }
        }

        // Visibility & delete click
        int y2 = HEADER_HEIGHT+20;
        for (size_t i=0;i<expressions.size();i++){
            Rectangle eye = {(float)(LEFT_PANEL_WIDTH-40),(float)(y2+10),30,30};
            Rectangle del = {(float)(LEFT_PANEL_WIDTH-70),(float)(y2+10),25,30};
            if (CheckCollisionPointRec(mp, eye) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                expressions[i].isVisible = !expressions[i].isVisible;
                break;
            }
            if (CheckCollisionPointRec(mp, del) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                if (expressions[i].ast) freeAST(expressions[i].ast);
                expressions.erase(expressions.begin()+i);
                if (activeExpression == (int)i) activeExpression = expressions.empty() ? -1 : (int)i-1;
                break;
            }
            y2 += EXPRESSION_HEIGHT + EXPRESSION_MARGIN;
        }

        // Add expression
        int buttonY = HEADER_HEIGHT + 20 + (int)expressions.size()*(EXPRESSION_HEIGHT+EXPRESSION_MARGIN);
        if (buttonY < WINDOW_HEIGHT - 100 &&
            IsMouseOverRect(10, buttonY, LEFT_PANEL_WIDTH - 20, EXPRESSION_HEIGHT) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            expressions.emplace_back("", expressionColors[expressions.size()%maxColors]);
            activeExpression = (int)expressions.size()-1;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawHeader();
        DrawLeftPanel(expressions, activeExpression);
        DrawGraphArea(expressions);
        EndDrawing();
    }

    for (auto& e : expressions)
        if (e.ast) freeAST(e.ast);

    UnloadTexture(eyeOpenTex);
    UnloadTexture(eyeClosedTex);
    UnloadTexture(deleteTex);

    CloseWindow();
}
