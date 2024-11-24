#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* prompt = "shell> ";

Display* display;
Window window;
GC gc;
XFontStruct* font_info;
Cursor cursor;
int cursorX = 0;
char* text = NULL;
int textLength = 0;
int textCapacity = 1024; // Initial text buffer capacity

void drawText() 
{
    XClearWindow(display, window);
    XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
    XSetFont(display, gc, font_info->fid);
    
    // Display the prompt
    XDrawString(display, window, gc, 10, 20, prompt, strlen(prompt));

    // Display the user's text
    XDrawString(display, window, gc, 10 + XTextWidth(font_info, prompt, strlen(prompt)), 20, text, textLength);
    XDrawLine(display, window, gc, 10 + (cursorX + strlen(prompt)) * font_info->max_bounds.width, 10,
              10 + (cursorX + strlen(prompt)) * font_info->max_bounds.width, 11 + font_info->ascent + font_info->descent);

    XFlush(display);
}


void callYourFunction( char* text) {
    // You can print or perform any action with the text here
    printf("Text received: %s\n", text);
}


void insertChar(char ch) {
    if (textLength >= textCapacity - 1) {
        textCapacity *= 2; // Double the buffer capacity
        text = (char*)realloc(text, textCapacity * sizeof(char));
        if (text == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }
    if (cursorX < textLength) {
        memmove(text + cursorX + 1, text + cursorX, textLength - cursorX);
    }
    text[cursorX] = ch;
    cursorX++;
    textLength++;
}

void deleteChar() {
    if (cursorX > 0 && textLength > 0) {
        memmove(text + cursorX - 1, text + cursorX, textLength - cursorX);
        cursorX--;
        textLength--;
    }
}

int main() {
    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Unable to open X display\n");
        return 1;
    }

    window = XCreateSimpleWindow(display, DefaultRootWindow(display), 10, 10, 400, 200, 1,
                                 BlackPixel(display, DefaultScreen(display)), WhitePixel(display, DefaultScreen(display)));
    XSelectInput(display, window, ExposureMask | KeyPressMask);
    XMapWindow(display, window);
    XStoreName(display, window, "Text Editor");

    gc = XCreateGC(display, window, 0, 0);

    cursor = XCreateFontCursor(display, 68);
    XDefineCursor(display, window, cursor);
    XFlush(display);

    font_info = XLoadQueryFont(display, "fixed");
    if (!font_info) {
        fprintf(stderr, "XLoadQueryFont failed\n");
        return 1;
    }

    text = (char*)calloc(textCapacity, sizeof(char));

    XEvent event;
    while (1) {
        XNextEvent(display, &event);

        if (event.type == Expose) {
            drawText();
        }

        if (event.type == KeyPress) {
            KeySym keysym;
            char buffer[1];
            XLookupString(&event.xkey, buffer, sizeof(buffer), &keysym, NULL);

            if (keysym == XK_Return) {
                // Clear the screen
                XClearWindow(display, window);
                XFlush(display);

                // Call the function and pass the text
                callYourFunction(text);

                // Reset text
                textLength = 0;
                cursorX = 0;
                text[0] = '\0';

                // Redraw the prompt
                drawText();
            } else if (keysym == XK_BackSpace) {
                deleteChar();
                drawText();
            } else if (keysym == XK_Escape) {
                break;
            } else {
                insertChar(buffer[0]);
                drawText();
            }
        }
    }

    XFreeCursor(display, cursor);
    XFreeFontInfo(NULL, font_info, 0);
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    free(text);

    return 0;
}

