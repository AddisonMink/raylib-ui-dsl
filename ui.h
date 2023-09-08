#ifndef UI_H
#define UI_H

#include "stdlib.h"
#include "raylib.h"

typedef enum AlignH
{
    LEFT,
    CENTER,
    RIGHT
} AlignH;

typedef enum AlignV
{
    TOP,
    MIDDLE,
    BOTTOM
} AlignV;

typedef struct UIBuilder UIBuilder;

UIBuilder *UIBuilderAlloc(size_t maxTokens);

void UIBuilderFree(UIBuilder *builder);

void UIInit(UIBuilder *builder);

void UIRect(UIBuilder *builder, float width, float height, Color color);

void UIText(UIBuilder *builder, const char *text, int fontSize, Color color);

void UIRow(UIBuilder *builder, float spacing);
void UIRowEnd(UIBuilder *builder);

void UIColumn(UIBuilder *builder, float spacing);
void UIColumnEnd(UIBuilder *builder);

void UIAlignH(UIBuilder *builder, AlignH align);
void UIAlignHEnd(UIBuilder *builder);

void UIAlignV(UIBuilder *builder, AlignV align);
void UIAlignVEnd(UIBuilder *builder);

void UIPadding(UIBuilder *builder, float spacing);
void UIPaddingEnd(UIBuilder *builder);

void UIBorder(UIBuilder *builder, float thickness, Color color);
void UIBorderEnd(UIBuilder *builder);

void UIDraw(UIBuilder *builder, Vector2 position);

#endif