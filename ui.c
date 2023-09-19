#include "ui.h"

#pragma region Types
typedef enum TokenType
{
    // Root
    TOKEN_ROOT,

    // Primitives
    TOKEN_RECT,
    TOKEN_TEXT,

    // Containers
    TOKEN_ROW,
    TOKEN_ROW_END,
    TOKEN_COLUMN,
    TOKEN_COLUMN_END,

    // Modifiers
    TOKEN_ALIGN_H,
    TOKEN_ALIGN_V,
    TOKEN_ALIGN,
    TOKEN_PADDING,
    TOKEN_BORDER,
    TOKEN_SHIM,
    TOKEN_SHIM_H,
    TOKEN_SHIM_V,
    TOKEN_BACKROUND
} TokenType;

typedef struct RectToken
{
    float width;
    float height;
    Color color;
} RectToken;

typedef struct TextToken
{
    const char *text;
    int fontSize;
    Color color;
} TextToken;

typedef struct RowToken
{
    float spacing;
} RowToken;

typedef struct ColumnToken
{
    float spacing;
} ColumnToken;

typedef struct AlignHToken
{
    AlignH align;
} AlignHToken;

typedef struct AlignVToken
{
    AlignV align;
} AlignVToken;

typedef struct AlignToken
{
    AlignH alignH;
    AlignV alignV;
} AlignToken;

typedef struct PaddingToken
{
    float spacing;
} PaddingToken;

typedef struct BorderToken
{
    float thickness;
    Color color;
} BorderToken;

typedef struct BackgroundToken
{
    Color color;
} BackgroundToken;

typedef struct Token
{
    TokenType type;
    union
    {
        RectToken rect;
        TextToken text;
        RowToken row;
        ColumnToken column;
        AlignHToken alignH;
        AlignVToken alignV;
        AlignToken align;
        PaddingToken padding;
        BorderToken border;
        BackgroundToken background;
    };
    float width;
    float height;
    Vector2 position;
} Token;

typedef struct UIBuilder
{
    size_t maxTokens;
    size_t numTokens;
    Token *tokenList;
    Token **contextStack;
    size_t stackIndex;
} UIBuilder;
#pragma endregion

#pragma region Initialization
UIBuilder *UIBuilderAlloc(size_t maxTokens)
{
    UIBuilder *builder = MemAlloc(sizeof(UIBuilder));
    builder->maxTokens = maxTokens;
    builder->tokenList = MemAlloc(sizeof(Token) * maxTokens);

    builder->contextStack = MemAlloc(sizeof(Token *) * maxTokens);
    return builder;
}

void UIBuilderFree(UIBuilder *builder)
{
    MemFree(builder->tokenList);
    MemFree(builder->contextStack);
    MemFree(builder);
}

#pragma endregion

#pragma region DSL
static Token *pushToken(UIBuilder *builder, TokenType type)
{
    if (builder->numTokens < builder->maxTokens - 1)
    {
        Token *token = &builder->tokenList[builder->numTokens];
        token->type = type;
        token->width = 0;
        token->height = 0;
        token->position = (Vector2){0, 0};
        builder->numTokens++;
        return token;
    }
    else
    {
        TraceLog(LOG_INFO, "UIBuilder: Max tokens reached.");
        return NULL;
    }
}

void UIInit(UIBuilder *builder)
{
    builder->numTokens = 0;
    builder->stackIndex = 0;

    Token *token = pushToken(builder, TOKEN_ROOT);
    builder->contextStack[0] = token;
}

void UIInitEx(UIBuilder *builder, float width, float height)
{
    UIInit(builder);
    Token *context = builder->contextStack[0];
    context->width = width;
    context->height = height;
}

void UIRect(UIBuilder *builder, float width, float height, Color color)
{
    Token *token = pushToken(builder, TOKEN_RECT);
    if (token)
    {
        token->rect.width = width;
        token->rect.height = height;
        token->rect.color = color;
    }
}

void UIText(UIBuilder *builder, const char *text, int fontSize, Color color)
{
    Token *token = pushToken(builder, TOKEN_TEXT);
    if (token)
    {
        token->text.text = text;
        token->text.fontSize = fontSize;
        token->text.color = color;
    }
}

void UIRow(UIBuilder *builder, float spacing)
{
    Token *token = pushToken(builder, TOKEN_ROW);
    if (token)
        token->row.spacing = spacing;
}

void UIRowEnd(UIBuilder *builder)
{
    pushToken(builder, TOKEN_ROW_END);
}

void UIColumn(UIBuilder *builder, float spacing)
{
    Token *token = pushToken(builder, TOKEN_COLUMN);
    if (token)
        token->column.spacing = spacing;
}

void UIColumnEnd(UIBuilder *builder)
{
    pushToken(builder, TOKEN_COLUMN_END);
}

void UIAlignH(UIBuilder *builder, AlignH align)
{
    Token *token = pushToken(builder, TOKEN_ALIGN_H);
    if (token)
        token->alignH.align = align;
}

void UIAlignV(UIBuilder *builder, AlignV align)
{
    Token *token = pushToken(builder, TOKEN_ALIGN_V);
    if (token)
        token->alignV.align = align;
}

void UIAlign(UIBuilder *builder, AlignH alignH, AlignV alignV)
{
    Token *token = pushToken(builder, TOKEN_ALIGN);
    if (token)
    {
        token->align.alignH = alignH;
        token->align.alignV = alignV;
    }
}

void UIPadding(UIBuilder *builder, float spacing)
{
    Token *token = pushToken(builder, TOKEN_PADDING);
    if (token)
        token->padding.spacing = spacing;
}

void UIBorder(UIBuilder *builder, float thickness, Color color)
{
    Token *token = pushToken(builder, TOKEN_BORDER);
    if (token)
    {
        token->border.thickness = thickness;
        token->border.color = color;
    }
}

void UIShim(UIBuilder *builder, float width, float height)
{
    Token *token = pushToken(builder, TOKEN_SHIM);
    if (token)
    {
        token->width = width;
        token->height = height;
    }
}

void UIShimH(UIBuilder *builder, float width)
{
    Token *token = pushToken(builder, TOKEN_SHIM_H);
    if (token)
        token->width = width;
}

void UIShimV(UIBuilder *builder, float height)
{
    Token *token = pushToken(builder, TOKEN_SHIM_V);
    if (token)
        token->height = height;
}

void UIBackground(UIBuilder *builder, Color color)
{
    Token *token = pushToken(builder, TOKEN_BACKROUND);
    if (token)
        token->background.color = color;
}

#pragma endregion

#pragma region stack
static void pushContext(UIBuilder *builder, Token *token)
{
    if (builder->stackIndex < builder->maxTokens - 1)
    {
        builder->stackIndex++;
        builder->contextStack[builder->stackIndex] = token;
    }
    else
        TraceLog(LOG_INFO, "UIBuilder: Max context stack reached.");
}

static void popContext(UIBuilder *builder)
{
    if (builder->stackIndex > 0)
        builder->stackIndex--;
}

static Token *peekContext(UIBuilder *builder)
{
    return builder->contextStack[builder->stackIndex];
}
#pragma endregion

#pragma region Sizes
static void updateContextSize(UIBuilder *builder, Token *token)
{
    bool cont = false;
    do
    {
        cont = false;
        Token *context = peekContext(builder);

        switch (context->type)
        {
        case TOKEN_ROW:
        {
            if (token->type == TOKEN_ROW_END)
            {
                context->width -= context->row.spacing;
                popContext(builder);
                token = context;
                cont = true;
            }
            else
            {
                context->width += token->width + context->row.spacing;
                if (context->height < token->height)
                    context->height = token->height;
            }
        }
        break;

        case TOKEN_COLUMN:
        {
            if (token->type == TOKEN_COLUMN_END)
            {
                context->height -= context->column.spacing;
                popContext(builder);
                token = context;
                cont = true;
            }
            else
            {
                context->height += token->height + context->column.spacing;
                if (context->width < token->width)
                    context->width = token->width;
            }
        }
        break;

        case TOKEN_ALIGN_H:
        {
            context->width = token->width;
            if (context->height < token->height)
                context->height = token->height;
            popContext(builder);
            token = context;
            cont = true;
        }
        break;

        case TOKEN_ALIGN_V:
        {
            context->height = token->height;
            if (context->width < token->width)
                context->width = token->width;
            popContext(builder);
            token = context;
            cont = true;
        }
        break;

        case TOKEN_ALIGN:
        {
            context->width = token->width;
            context->height = token->height;
            popContext(builder);
            token = context;
            cont = true;
        }
        break;

        case TOKEN_PADDING:
        {
            context->width += token->width + context->padding.spacing * 2;
            context->height += token->height + context->padding.spacing * 2;
            popContext(builder);
            token = context;
            cont = true;
        }
        break;

        case TOKEN_BORDER:
        {
            context->width += token->width;
            context->height += token->height;
            popContext(builder);
            token = context;
            cont = true;
        }
        break;

        case TOKEN_SHIM:
        {
            popContext(builder);
            token = context;
            cont = true;
        }
        break;

        case TOKEN_SHIM_H:
        {
            context->height = token->height;
            popContext(builder);
            token = context;
            cont = true;
        }
        break;

        case TOKEN_SHIM_V:
        {
            context->width = token->width;
            popContext(builder);
            token = context;
            cont = true;
        }
        break;

        case TOKEN_BACKROUND:
        {
            context->width = token->width;
            context->height = token->height;
            popContext(builder);
            token = context;
            cont = true;
        }
        break;

        default:
            break;
        }
    } while (cont);
}

static void setSizes(UIBuilder *builder)
{
    for (size_t i = 0; i < builder->numTokens; i++)
    {
        Token *token = &builder->tokenList[i];
        switch (token->type)
        {
        // Root
        case TOKEN_ROOT:
            break;

        // Primitives
        case TOKEN_RECT:
        {
            token->width = token->rect.width;
            token->height = token->rect.height;
            updateContextSize(builder, token);
        }
        break;
        case TOKEN_TEXT:
        {
            token->width = MeasureText(token->text.text, token->text.fontSize);
            token->height = token->text.fontSize;
            updateContextSize(builder, token);
        }
        break;

        // Container Ends
        case TOKEN_ROW_END:
            updateContextSize(builder, token);
            break;
        case TOKEN_COLUMN_END:
            updateContextSize(builder, token);
            break;

        // Containers and Modifiers
        default:
            pushContext(builder, token);
            break;
        }
    }

    if (builder->stackIndex > 0)
        TraceLog(LOG_INFO, "UIBuilder: Context stack not empty.");
}
#pragma endregion

#pragma region Draw
static void updateContextPosition(UIBuilder *builder, Token *token)
{
    bool cont = false;
    do
    {
        cont = false;
        Token *context = peekContext(builder);

        switch (context->type)
        {
        // Root
        case TOKEN_ROOT:
            break;

        // Containers
        case TOKEN_ROW:
        {
            if (token->type == TOKEN_ROW_END)
            {
                popContext(builder);
                token = context;
                cont = true;
            }
            else
                context->position.x += token->width + context->row.spacing;
        }
        break;
        case TOKEN_COLUMN:
        {
            if (token->type == TOKEN_COLUMN_END)
            {
                popContext(builder);
                token = context;
                cont = true;
            }
            else
                context->position.y += token->height + context->column.spacing;
        }
        break;

        // Modifiers
        default:
        {
            popContext(builder);
            token = context;
            cont = true;
        }
        break;
        }
    } while (cont);
}

static void draw(UIBuilder *builder, Vector2 position)
{
    Token *root = peekContext(builder);
    root->position = position;

    for (size_t i = 0; i < builder->numTokens; i++)
    {
        Token *token = &builder->tokenList[i];
        switch (token->type)
        {
        case TOKEN_ROOT:
            break;

        case TOKEN_RECT:
        {
            token->position = peekContext(builder)->position;
            DrawRectangle(token->position.x, token->position.y, token->rect.width, token->rect.height, token->rect.color);
            updateContextPosition(builder, token);
        }
        break;

        case TOKEN_TEXT:
        {
            token->position = peekContext(builder)->position;
            DrawText(token->text.text, token->position.x, token->position.y, token->text.fontSize, token->text.color);
            updateContextPosition(builder, token);
        }
        break;

        case TOKEN_ROW:
        {
            token->position = peekContext(builder)->position;
            pushContext(builder, token);
        }
        break;

        case TOKEN_ROW_END:
            updateContextPosition(builder, token);
            break;

        case TOKEN_COLUMN:
        {
            token->position = peekContext(builder)->position;
            pushContext(builder, token);
        }
        break;

        case TOKEN_COLUMN_END:
        {
            Token *columnContext = peekContext(builder);
            popContext(builder);
            updateContextPosition(builder, columnContext);
        }
        break;

        case TOKEN_ALIGN_H:
        {
            Token *nextToken = &builder->tokenList[i + 1];
            Token *context = peekContext(builder);
            token->position = context->position;
            float width = context->width;

            switch (token->alignH.align)
            {
            case LEFT:
                break;
            case CENTER:
                token->position.x += width / 2 - nextToken->width / 2;
                break;
            case RIGHT:
                token->position.x += width - nextToken->width;
                break;
            }

            pushContext(builder, token);
        }
        break;

        case TOKEN_ALIGN_V:
        {
            Token *nextToken = &builder->tokenList[i + 1];
            Token *context = peekContext(builder);
            token->position = context->position;
            float height = context->height;

            switch (token->alignV.align)
            {
            case TOP:
                break;
            case MIDDLE:
                token->position.y += height / 2 - nextToken->height / 2;
                break;
            case BOTTOM:
                token->position.y += height - nextToken->height;
                break;
            }

            pushContext(builder, token);
        }
        break;

        case TOKEN_ALIGN:
        {
            Token *nextToken = &builder->tokenList[i + 1];
            Token *context = peekContext(builder);
            token->position = context->position;
            float width = context->width;
            float height = context->height;

            switch (token->align.alignH)
            {
            case LEFT:
                break;
            case CENTER:
                token->position.x += width / 2 - nextToken->width / 2;
                break;
            case RIGHT:
                token->position.x += width - nextToken->width;
                break;
            }

            switch (token->align.alignV)
            {
            case TOP:
                break;
            case MIDDLE:
                token->position.y += height / 2 - nextToken->height / 2;
                break;
            case BOTTOM:
                token->position.y += height - nextToken->height;
                break;
            }

            pushContext(builder, token);
        }
        break;

        case TOKEN_PADDING:
        {
            token->position = peekContext(builder)->position;
            token->position.x += token->padding.spacing;
            token->position.y += token->padding.spacing;
            pushContext(builder, token);
        }
        break;

        case TOKEN_BORDER:
        {
            token->position = peekContext(builder)->position;
            DrawRectangleLinesEx((Rectangle){token->position.x, token->position.y, token->width, token->height}, token->border.thickness, token->border.color);
            pushContext(builder, token);
        }
        break;

        case TOKEN_SHIM:
        {
            token->position = peekContext(builder)->position;
            pushContext(builder, token);
        }
        break;

        case TOKEN_SHIM_H:
        {
            token->position = peekContext(builder)->position;
            pushContext(builder, token);
        }
        break;

        case TOKEN_SHIM_V:
        {
            token->position = peekContext(builder)->position;
            pushContext(builder, token);
        }
        break;

        case TOKEN_BACKROUND:
        {
            token->position = peekContext(builder)->position;
            DrawRectangle(token->position.x, token->position.y, token->width, token->height, token->background.color);
            pushContext(builder, token);
        }
        break;
        }
    }
}

void UIDraw(UIBuilder *builder, Vector2 position)
{
    setSizes(builder);
    draw(builder, position);
}
#pragma endregion