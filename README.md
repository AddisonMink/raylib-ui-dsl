# Raylib UI DSL
A DSL for formatting and drawing UI's using Raylib and C.

## Example

This code:
```C
UIInitEx(builder, screenWidth, screenHeight);

UIAlign(builder, CENTER, MIDDLE);
UIBorder(builder, 2, WHITE);
UIPadding(builder, 12);
UIColumn(builder, 10);
{
    UIRect(builder, 100, 100, RED);
    UIAlignH(builder, CENTER);

    UIText(builder, "Hello", 20, WHITE);
    UIRect(builder, 100, 100, BLUE);

    UIRow(builder, 10);
    {
        UIRect(builder, 100, 100, GREEN);

        UIAlignV(builder, MIDDLE);
        UIRect(builder, 50, 50, YELLOW);
    }
    UIRowEnd(builder);
}
UIColumnEnd(builder);

UIDraw(builder, (Vector2){0, 0});
```

Produces this output:

![demo](screenshots/demo.png "demo")


## Purpose
Hard-coding the formatting for menus, textboxes and the like is tedious, error-prone, and extremely difficult to refactor.

The design goals are as follows:
* Provide a declarative, HTML-like syntax for specifying UI elements.
* Support dynamically-sized elements. Container elements like rows, columns, and borders should snugly fit around their contents without the programmer having to explicitly give sizes and positions of elements.
* Work seamlessly alongside the existing 2D drawing capabilities of Raylib.

## How to Use It
1. Copy `ui.c` and `ui.h` into your C project and add a line to your makefile so that `uic.c` is included in the build. You can use this project's makefile as an example.

2. Include "ui.h" in your main file.

3. After calling Raylib's `InitWindow` function, allocate a `UIBuilder`. The parameter to this function is the maximum number of tokens that will be in the UI. This corresponds exactly to the number of times you call functions like `UIText` and `UIRow`.
```
UIBuilder *builder = UIBuilderAlloc(1024);
```


4. After calling Raylib's `BeginDrawing` function, call `UIInit(builder)` to reset the internal state of the `UIBuilder`.

5. Declare a UI.

6. Call `UIDraw(builder,origin)`, where `origin` is a Raylib `Vector2` that specifies the top-left corner of the UI.

## DSL Reference
In addition, to `UIInit` and `UIDraw` functions are provided to declare UI elements.

There are 4 classes of element:

* *Root* - implicitly declared during `UIInit`. It is the parent of all other elements. It will have the same size as the element it contains. `UIInit` lets you manually specify the size of the root element.

* *Primitives* - `UIText` and `UIRect` are leaf elements. Their size is known at the time of declaration.

* *Modifiers* - `UIAlign`, `UIPadding`, and `UIBorder` have exactly 1 child element. Their size is derived from the size of their child element.

* *Containers* - `UIRow` and `UIColumn` have 0 or more children. The end of a container is declared with the corresponding function `UIRowEnd` or `UIColumnEnd`.

### Primitives
- `UIText` - draws a string with a given font size and color.
- `UIRect` - draws a rectangle with the given dimensions and color.

### Modifiers
- `UIAlignH` - Horizontally aligns its child element within its parent element. Inherits the width of its parent element and the height of its child element.

- `UIAlignV` - See `UIAlignH`, but vertical.

- `UIAlign` - Aligns its child element both vertically and horizontally within its parent element. Inherits the width and height of its parent element.

- `UIPadding` - Centers its child inside of an empty margin. 

- `UIBorder` - Wraps a rectangular border around its child.

### Containers
- `UIRow` - Arranges its child elements in a row. Everything between `UIRow` and `UIRowEnd` is included in the row. It *must* be followed by a `UIRowEnd` element. Otherwise, behavior is undefined.

- `UIColumn` - See `UIRow`.

## How it Works
I may do a write-up on this eventually.

TL;DR The `UIBuilder` holds a list of tokens and a stack of contexts. 

Calling a function like `UIRect` pushes a token to the list.

The list of tokens is processed in 2 passes. First, the size of all elements is determined. Then, the positions are determined and the elements are drawn.

