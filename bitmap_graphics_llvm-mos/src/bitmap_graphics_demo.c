// ---------------------------------------------------------------------------
// bitmap_graphics_demo.c
//
// This code demonstates use of the bitmap_graphics library.
//
// The library was written by tonyvr to simplify bitmap graphics programming
// of the RP6502 picocomputer designed by Rumbledethumps.
//
// This code is an adaptation of the vga_graphics library written by V. Hunter Adams
// from Cornell University, for his excellent RP2040 microcontroller programming course.
//
// https://github.com/vha3/Hunter-Adams-RP2040-Demos/tree/master/VGA_Graphics/VGA_Graphics_Primitives
//
// There doesn't seem to be a copyright or a license associated with his code.
// I don't care what you do with my version either -- have fun!
// ---------------------------------------------------------------------------

#include <rp6502.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "colors.h"
#include "bitmap_graphics.h"

// XRAM locations
#define KEYBOARD_INPUT 0xFF10 // KEYBOARD_BYTES of bitmask data

// HID keycodes that we care about for this demo
#define KEY_ESC 0x29 // Keyboard ESCAPE

// 256 bytes HID code max, stored in 32 uint8
#define KEYBOARD_BYTES 32
uint8_t keystates[KEYBOARD_BYTES] = {0};
bool handled_key = false;

// keystates[code>>3] gets contents from correct byte in array
// 1 << (code&7) moves a 1 into proper position to mask with byte contents
// final & gives 1 if key is pressed, 0 if not
#define key(code) (keystates[code >> 3] & (1 << (code & 7)))

// ---------------------------------------------------------------------------
// This routine draws a bunch of stuff using the bitmap_graphics.c library,
// then wais for a keypress to continue. Key events are sent to the serial monitor.
// ---------------------------------------------------------------------------
static void draw()
{
    uint8_t i = 0;
    char msg[80] = {0};
    uint8_t bpp = bits_per_pixel();

    // draw some lines and shapes
    fill_circle(random(color(BLACK, bpp==16),
                       color(WHITE, bpp==16)), canvas_width()/2, canvas_height()/2, canvas_width()/8);
    draw_hline(color(GREEN, bpp==16), canvas_width()/3, (canvas_height()/2), canvas_width()/3);
    draw_vline(color(RED,bpp==16), canvas_width()/2, 0, canvas_height());
    draw_line(color(YELLOW,bpp==16), canvas_width()/3, canvas_height(), 2*canvas_width()/3, 0);
    draw_line(color(BLUE,bpp==16), canvas_width()/3, 0, 2*canvas_width()/3, canvas_height());
    draw_rect(color(WHITE,bpp==16), canvas_width()/3, 0, (canvas_width()/3)+1, canvas_height());
    draw_circle(color(CYAN,bpp==16), canvas_width()/2, canvas_height()/2, canvas_width()/4);
    draw_rounded_rect(color(DARK_CYAN,bpp==16), 0, 0, canvas_width()/4, canvas_height()/4, 10);
    fill_rounded_rect(random(color(BLACK, bpp==16), color(WHITE, bpp==16)),
                      (3*canvas_width()/4), (3*canvas_height()/4),
                      (canvas_width()/4)-1, (canvas_height()/4)-1, 10);

    // draw title
    set_text_multiplier((bpp == 16) ? 1 : ((bpp <= 2) ? 4 : 2));
    if (bpp > 1) {
        set_text_colors(color(YELLOW,bpp==16), color(DARK_RED,bpp==16));
    } else {
        set_text_color(color(YELLOW,bpp==16));
    }
    set_cursor(canvas_width()/16, canvas_height()/10);
    draw_string("Bitmap Graphics Demo");

    // draw current bpp and canvas dimensions
    set_text_multiplier((bpp <= 2) ? 2 : 1);
    set_text_color(color(WHITE,bpp==16));
    set_cursor(4*canvas_width()/5, 5 + canvas_height()/4);
    sprintf(msg, "bpp%u", bpp);
    draw_string(msg);
    set_cursor(4*canvas_width()/5, ((bpp <= 2) ? 25 : 18) + canvas_height()/4);
    sprintf(msg, "%ux%u", canvas_width(), canvas_height());
    draw_string(msg);

    // draw a bunch of random lines
    while(++i != 0) {// wait for rollover
        draw_line(random(color(BLACK, bpp==16), color(WHITE, bpp==16)),
                  random(0, canvas_width()/4), random(canvas_height()/2, canvas_height()),
                  random(0, canvas_width()/4), random(canvas_height()/2, canvas_height()));
    }

    // draw prompt to continue
    set_text_multiplier((bpp <= 2) ? 3 : 1);
    set_text_color(color(WHITE,bpp==16));
    set_cursor(0, 5 + canvas_height()/4);
    if (bpp == 16) {
        draw_string("Press ESC \nkey to \nexit!");
    } else {
        draw_string("Press any \nkey to \ncontinue");
    }

    // Although completely unnecessary here, I've included an example
    // of keypress detection. After the final screen, you can see
    // the key events in the serial terminal, and experiment until
    // you get bored and exit by pressing the ESC key. Have fun!

    // wait for a keypress
    xregn( 0, 0, 0, 1, KEYBOARD_INPUT);
    RIA.addr0 = KEYBOARD_INPUT;
    RIA.step0 = 0;
    while (1) {
        uint8_t i;

        // fill the keystates bitmask array
        for (i = 0; i < KEYBOARD_BYTES; i++) {
            uint8_t j, new_keys;
            RIA.addr0 = KEYBOARD_INPUT + i;
            new_keys = RIA.rw0;
///*
            // check for change in any and all keys
            for (j = 0; j < 8; j++) {
                uint8_t new_key = (new_keys & (1<<j));
                if ((((i<<3)+j)>3) && (new_key != (keystates[i] & (1<<j)))) {
                    printf( "key %d %s\n", ((i<<3)+j), (new_key ? "pressed" : "released"));
                }
            }
//*/
            keystates[i] = new_keys;
        }

        // check for a key down
        if (!(keystates[0] & 1)) {
            if (!handled_key) { // handle only once per single keypress
                // handle the keystrokes
                if (key(KEY_ESC)) {
                    break;
                } else if (bpp != 16) {
                    break;
                }
                handled_key = true;
            }
        } else { // no keys down
            handled_key = false;
        }
    }
}

// ---------------------------------------------------------------------------
// This procedes through all the different bitmap modes currently supported
// for the amazing RP6502 picocomputer designed by Rumbledethumps:
// https://github.com/picocomputer
// ---------------------------------------------------------------------------
int main(void)
{
    // plane=0, canvas=3, w=640, h=480, bpp1
    init_bitmap_graphics(0xFF00, 0x0000, 0, 3, 640, 480, 1);
    erase_canvas();
    draw();

    // plane=0, canvas=4, w=640, h=360, bpp2
    init_bitmap_graphics(0xFF00, 0x0000, 0, 4, 640, 360, 2);
    erase_canvas();
    draw();

    // plane=0, canvas=1, w=320, h=240, bpp4
    init_bitmap_graphics(0xFF00, 0x0000, 0, 1, 320, 240, 4);
    erase_canvas();
    draw();

    // plane=0, canvas=2, w=320, h=180, bpp8
    init_bitmap_graphics(0, 0, 0, 0, 0, 0, 0); //use defaults
    //init_bitmap_graphics(0xFF00, 0x0000, 0, 2, 320, 180, 8);
    erase_canvas();
    draw();

    // plane=0, canvas=2, w=240, h=124, bpp16
    init_bitmap_graphics(0xFF00, 0x0000, 0, 2, 240, 124, 16);
    erase_canvas();
    draw();

    printf("Goodbye!\n");
}
