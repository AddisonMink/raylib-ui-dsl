#include "ui.h"

typedef enum TokenType
{
    TOKEN_ROOT,
    TOKEN_RECT,
    TOKEN_TEXT,
    TOKEN_ROW,
    TOKEN_ROW_END,
    TOKEN_COLUMN,
    TOKEN_COLUMN_END,
    TOKEN_ALIGN_H,
    TOKEN_ALIGN_H_END,
    TOKEN_ALIGN_V,
    TOKEN_ALIGN_V_END,
    TOKEN_PADDING,
    TOKEN_PADDING_END,
    TOKEN_BORDER,
    TOKEN_BORDER_END,
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
    float width;
} AlignHToken;

typedef struct AlignVToken
{
    AlignV align;
    float height;
} AlignVToken;

typedef struct PaddingToken
{
    float spacing;
} PaddingToken;

typedef struct BorderToken
{
    float thickness;
    Color color;
} BorderToken;

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
        PaddingToken padding;
        BorderToken border;
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
    {
        token->row.spacing = spacing;
    }
}

void UIRowEnd(UIBuilder *builder)
{
    pushToken(builder, TOKEN_ROW_END);
}

void UIColumn(UIBuilder *builder, float spacing)
{
    Token *token = pushToken(builder, TOKEN_COLUMN);
    if (token)
    {
        token->column.spacing = spacing;
    }
}

void UIColumnEnd(UIBuilder *builder)
{
    pushToken(builder, TOKEN_COLUMN_END);
}

void UIAlignH(UIBuilder *builder, AlignH align, float width)
{
    Token *token = pushToken(builder, TOKEN_ALIGN_H);
    if (token)
    {
        token->alignH.align = align;
        token->alignH.width = width;
    }
}

void UIAlignHEnd(UIBuilder *builder)
{
    pushToken(builder, TOKEN_ALIGN_H_END);
}

void UIAlignV(UIBuilder *builder, AlignV align, float height)
{
    Token *token = pushToken(builder, TOKEN_ALIGN_V);
    if (token)
    {
        token->alignV.align = align;
        token->alignV.height = height;
    }
}

void UIAlignVEnd(UIBuilder *builder)
{
    pushToken(builder, TOKEN_ALIGN_V_END);
}

void UIPadding(UIBuilder *builder, float spacing)
{
    Token *token = pushToken(builder, TOKEN_PADDING);
    if (token)
    {
        token->padding.spacing = spacing;
    }
}

void UIPaddingEnd(UIBuilder *builder)
{
    pushToken(builder, TOKEN_PADDING_END);
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

void UIBorderEnd(UIBuilder *builder)
{
    pushToken(builder, TOKEN_BORDER_END);
}

static void pushContext(UIBuilder *builder, Token *token)
{
    if (builder->stackIndex < builder->maxTokens - 1)
    {
        builder->stackIndex++;
        builder->contextStack[builder->stackIndex] = token;
    }
    else
    {
        TraceLog(LOG_INFO, "UIBuilder: Max context stack reached.");
    }
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

static void updateContextSize(UIBuilder *builder, Token *token)
{
    Token *context = peekContext(builder);
    switch (context->type)
    {
    case TOKEN_ROW:
    {
        context->width += token->width + context->row.spacing;
        if (context->height < token->height)
            context->height = token->height;
    }
    break;

    case TOKEN_COLUMN:
    {
        context->height += token->height + context->column.spacing;
        if (context->width < token->width)
            context->width = token->width;
    }
    break;

    case TOKEN_ALIGN_H:
    {
        context->width = context->alignH.width;
        if (context->height < token->height)
            context->height = token->height;
    }
    break;

    case TOKEN_ALIGN_V:
    {
        context->height = context->alignV.height;
        if (context->width < token->width)
            context->width = token->width;
    }
    break;

    case TOKEN_PADDING:
    {
        context->width += token->width + context->padding.spacing * 2;
        context->height += token->height + context->padding.spacing * 2;
    }
    break;

    case TOKEN_BORDER:
    {
        context->width += token->width;
        context->height += token->height;
    }
    break;

    default:
        break;
    }
}

static void setSizes(UIBuilder *builder)
{
    for (size_t i = 0; i < builder->numTokens; i++)
    {
        Token *token = &builder->tokenList[i];
        switch (token->type)
        {
        case TOKEN_ROOT:
            break;

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

        case TOKEN_ROW:
        {
            pushContext(builder, token);
        }
        break;

        case TOKEN_ROW_END:
        {
            Token *context = peekContext(builder);
            context->width -= context->row.spacing;
            popContext(builder);
            updateContextSize(builder, context);
        }
        break;

        case TOKEN_COLUMN:
        {
            pushContext(builder, token);
        }
        break;

        case TOKEN_COLUMN_END:
        {
            Token *context = peekContext(builder);
            context->height -= context->column.spacing;
            popContext(builder);
            updateContextSize(builder, context);
        }
        break;

        case TOKEN_ALIGN_H:
        {
            pushContext(builder, token);
        }
        break;

        case TOKEN_ALIGN_H_END:
        {
            Token *context = peekContext(builder);
            popContext(builder);
            updateContextSize(builder, context);
        }
        break;

        case TOKEN_ALIGN_V:
        {
            pushContext(builder, token);
        }
        break;

        case TOKEN_ALIGN_V_END:
        {
            Token *context = peekContext(builder);
            popContext(builder);
            updateContextSize(builder, context);
        }
        break;

        case TOKEN_PADDING:
        {
            pushContext(builder, token);
        }
        break;

        case TOKEN_PADDING_END:
        {
            Token *context = peekContext(builder);
            popContext(builder);
            updateContextSize(builder, context);
        }
        break;

        case TOKEN_BORDER:
        {
            pushContext(builder, token);
        }
        break;

        case TOKEN_BORDER_END:
        {
            Token *context = peekContext(builder);
            popContext(builder);
            updateContextSize(builder, context);
        }
        break;
        }
    }

    if (builder->stackIndex > 0)
        TraceLog(LOG_INFO, "UIBuilder: Context stack not empty.");
}

static void updateContextPosition(UIBuilder *builder, Token *token)
{
    Token *context = peekContext(builder);
    switch (context->type)
    {
    case TOKEN_ROW:
    {
        context->position.x += token->width + context->row.spacing;
    }
    break;

    case TOKEN_COLUMN:
    {
        context->position.y += token->height + context->column.spacing;
    }
    break;

    default:
        break;
    }
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
        {
            Token *rowContext = peekContext(builder);
            popContext(builder);
            updateContextPosition(builder, rowContext);
        }
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
            token->position = peekContext(builder)->position;

            switch (token->alignH.align)
            {
            case LEFT:
                break;
            case CENTER:
                token->position.x += token->width / 2 - nextToken->width / 2;
                break;
            case RIGHT:
                token->position.x += token->width - nextToken->width;
                break;
            }

            pushContext(builder, token);
        }
        break;

        case TOKEN_ALIGN_H_END:
        {
            Token *alignHContext = peekContext(builder);
            popContext(builder);
            updateContextPosition(builder, alignHContext);
        }
        break;

        case TOKEN_ALIGN_V:
        {
            Token *nextToken = &builder->tokenList[i + 1];
            token->position = peekContext(builder)->position;

            switch (token->alignV.align)
            {
            case TOP:
                break;
            case MIDDLE:
                token->position.y += token->height / 2 - nextToken->height / 2;
                break;
            case BOTTOM:
                token->position.y += token->height - nextToken->height;
                break;
            }

            pushContext(builder, token);
        }
        break;

        case TOKEN_ALIGN_V_END:
        {
            Token *alignVContext = peekContext(builder);
            popContext(builder);
            updateContextPosition(builder, alignVContext);
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

        case TOKEN_PADDING_END:
        {
            Token *paddingContext = peekContext(builder);
            popContext(builder);
            updateContextPosition(builder, paddingContext);
        }
        break;

        case TOKEN_BORDER:
        {
            token->position = peekContext(builder)->position;
            DrawRectangleLinesEx((Rectangle){token->position.x, token->position.y, token->width, token->height}, token->border.thickness, token->border.color);
            pushContext(builder, token);
        }
        break;

        case TOKEN_BORDER_END:
        {
            Token *borderContext = peekContext(builder);
            popContext(builder);
            updateContextPosition(builder, borderContext);
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