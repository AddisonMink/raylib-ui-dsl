#include "ui.h"

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "UI Test");
    UIBuilder *builder = UIBuilderAlloc(1024);

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        UIInitEx(builder, screenWidth, screenHeight);

        UIAlign(builder, CENTER, MIDDLE);
                UIBorder(builder, 2, WHITE);
                    UIPadding(builder, 12);
                        UIColumn(builder, 10);
                            UIRect(builder, 100, 100, RED);
                            UIAlignH(builder, CENTER);
                                UIText(builder, "Hello", 20, WHITE);
                            UIAlignHEnd(builder);
                            UIRect(builder, 100, 100, BLUE);
                        UIColumnEnd(builder);
                    UIPaddingEnd(builder);
                UIBorderEnd(builder);
        UIAlignEnd(builder);

        UIDraw(builder, (Vector2){0, 0});

        EndDrawing();
    }

    CloseWindow();
    UIBuilderFree(builder);

    return 0;
}