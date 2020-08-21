//  Simple port of LVGL to WebAssembly.
//  Renders UI controls to HTML Canvas but touch input not handled yet.
//  To build see lvgl.sh
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../lvgl.h"
#include "../demo/lv_demo_widgets.h"
#include "lv_port_disp.h"

////////////////////////////////////////////////////////////////////
//  Device and Display Buffers

///  RGBA WebAssembly Display Buffer that will be rendered to HTML Canvas
#define DISPLAY_BYTES_PER_PIXEL 4
uint8_t display_buffer[LV_HOR_RES_MAX * LV_VER_RES_MAX * DISPLAY_BYTES_PER_PIXEL];

///  Plot a pixel on the WebAssembly Display Buffer
void put_display_px(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    assert(x >= 0); assert(x < LV_HOR_RES_MAX);
    assert(y >= 0); assert(y < LV_VER_RES_MAX);
    //  TODO: Map RGB565 to RGBA
    int i = (y * LV_HOR_RES_MAX * DISPLAY_BYTES_PER_PIXEL) + (x * DISPLAY_BYTES_PER_PIXEL);
    display_buffer[i++] = r;  //  Red
    display_buffer[i++] = g;  //  Green
    display_buffer[i++] = b;  //  Blue
    display_buffer[i++] = a;  //  Alpha
}

///  Return the WebAssembly Address of the WebAssembly Display Buffer
unsigned get_display_buffer(void) {
    uint8_t *p = &display_buffer[0];
    return (unsigned) p;
}

///  Return the width of the WebAssembly Display Buffer
unsigned get_display_width(void) { return LV_HOR_RES_MAX; }

///  Return the height of the WebAssembly Display Buffer
unsigned get_display_height(void) { return LV_VER_RES_MAX; }

///  Render a colour box to the WebAssembly Display Buffer
int test_display(void) {
    puts("In C: Testing display...");
    for (uint16_t x = 0; x < LV_HOR_RES_MAX; x++) {
        for (uint16_t y = 0; y < LV_VER_RES_MAX; y++) {     
            uint8_t r = (x * 256 / 20) & 0xff;
            uint8_t g = (y * 256 / 20) & 0xff;
            uint8_t b = ((x + y) * 256 / 40) & 0xff;
            uint8_t a = 0xff;
            put_display_px(x, y, r, g, b, a);
        }
    }
    return 0;
}

////////////////////////////////////////////////////////////////////
//  Render LVGL

/// Init the LVGL display
void init_display(void) {
    puts("In C: Init display...");

    //  Init the LVGL display
    lv_init();
    lv_port_disp_init();
}

/// Render a Button Widget and a Label Widget
void render_widgets(void) {
    puts("In C: Rendering widgets...");
    lv_obj_t * btn = lv_btn_create(lv_scr_act(), NULL);     //  Add a button the current screen
    lv_obj_set_pos(btn, 10, 10);                            //  Set its position
    lv_obj_set_size(btn, 120, 50);                          //  Set its size

    lv_obj_t * label = lv_label_create(btn, NULL);          //  Add a label to the button
    lv_label_set_text(label, "Button");                     //  Set the labels text
}

/// Render the LVGL display
void render_display(void) {
    puts("In C: Rendering display...");
    //  Must tick at least 100 milliseconds to force LVGL to update display
    lv_tick_inc(100);
    //  LVGL will flush our display driver
    lv_task_handler();
}

////////////////////////////////////////////////////////////////////
//  Rust LVGL Interface

/// Create LVGL Screen. Called by rust/wasm/src/lib.rs
lv_obj_t *create_screen(void) {
    lv_obj_t *screen = lv_obj_create(NULL, NULL);
    return screen;
}

////////////////////////////////////////////////////////////////////
//  RIOT Interface

/// Month names. From https://github.com/lupyuen/pinetime-rust-riot/blob/master/modules/controller/controller_time.c
#define MONSPERYEAR     12
static const char *mon_short_names[MONSPERYEAR + 1] = {
    [0] = "Inv",
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "June",
    "July",
    "Aug",
    "Sept",
    "Oct",
    "Nov",
    "Dec",
};

/// Date time format. From https://github.com/lupyuen/pinetime-rust-riot/blob/master/modules/controller/include/controller/time.h
typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t dayofmonth;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t fracs;
} controller_time_spec_t;

/// Get month name. From https://github.com/lupyuen/pinetime-rust-riot/blob/master/modules/controller/controller_time.c
const char *controller_time_month_get_short_name(controller_time_spec_t *time) {
    assert(time->month < MONSPERYEAR);
    return mon_short_names[time->month];
}

////////////////////////////////////////////////////////////////////
//  LVGL Compatibility

/// Change LVGL v6 lv_obj_set_style() to LVGL v7 lv_obj_reset_style_list() and lv_obj_add_style()
/// Called by rust/app/src/watch_face.rs
#undef lv_obj_set_style
void lv_obj_set_style(lv_obj_t *obj, const lv_style_t *style) {
    lv_obj_reset_style_list(obj, LV_LABEL_PART_MAIN);  //  TODO: Handle non-label styles
    lv_obj_add_style(obj, LV_LABEL_PART_MAIN, style);  //  TODO: Handle non-label styles
}

////////////////////////////////////////////////////////////////////
//  Main

/// Do nothing
int main(int argc, char **argv) {
    puts("In C: main()");
    return 0;
}
