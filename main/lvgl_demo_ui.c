#include <math.h>
#include "lvgl.h"
#include "esp_err.h"

#include "bsp/display.h"

#ifndef PI
#define PI  (3.14159f)
#endif

// LVGL image declare
LV_IMG_DECLARE(esp_logo);
LV_IMG_DECLARE(esp_text);

typedef struct {
    lv_obj_t *scr;
    int count_val;
} my_timer_context_t;

static lv_obj_t *arc[3];
static lv_obj_t *img_logo;
static lv_obj_t *img_text;
static lv_obj_t *slider; // Slider object
static lv_obj_t *label_value; // Label to display the value
static lv_meter_indicator_t *knob_indicator;
static lv_color_t arc_color[] = {
    LV_COLOR_MAKE(232, 87, 116),
    LV_COLOR_MAKE(126, 87, 162),
    LV_COLOR_MAKE(90, 202, 228),
};

static void slider_event_handler(lv_event_t *e) {
    lv_obj_t *slider = lv_event_get_target(e);
    char buf[16];
    int value = lv_slider_get_value(slider);
    snprintf(buf, sizeof(buf), "Teplota: %d°C", value);
    lv_label_set_text(label_value, buf);
    lv_obj_align(label_value, LV_ALIGN_TOP_LEFT, 0, 0);
}

// Slider for screen brightness adjustment
static void brightness_slider_event_handler(lv_event_t *e) {
    lv_obj_t *slider = lv_event_get_target(e);
    int value = lv_slider_get_value(slider);
    bsp_display_brightness_set(value);

}

int8_t bsp_get_battery_level(void);

static void anim_timer_cb(lv_timer_t *timer) {
    my_timer_context_t *timer_ctx = (my_timer_context_t *) timer->user_data;
    int count = timer_ctx->count_val;
    lv_obj_t *scr = timer_ctx->scr;

    // Play arc animation
    if (count < 90) {
        lv_coord_t arc_start = count > 0 ? (1 - cosf(count / 180.0f * PI)) * 270 : 0;
        lv_coord_t arc_len = (sinf(count / 180.0f * PI) + 1) * 135;

        for (size_t i = 0; i < sizeof(arc) / sizeof(arc[0]); i++) {
            lv_arc_set_bg_angles(arc[i], arc_start, arc_len);
            lv_arc_set_rotation(arc[i], (count + 120 * (i + 1)) % 360);
        }
    }

    // Delete arcs and create rotary knob when animation finished
    if (count == 90) {
        for (size_t i = 0; i < sizeof(arc) / sizeof(arc[0]); i++) {
            lv_obj_del(arc[i]);
        }

        // Create new image and make it transparent
        img_text = lv_img_create(scr);
        lv_img_set_src(img_text, &esp_text);
        lv_obj_set_style_img_opa(img_text, 0, 0);


        // Create label for displaying the slider value
        label_value = lv_label_create(scr);
        lv_label_set_text(label_value, "Teplota: 0");

        lv_obj_align(label_value, LV_ALIGN_TOP_LEFT, 0, 0);

        // Create a slider
        slider = lv_slider_create(scr);
        lv_obj_set_width(slider, 200); // Set the slider's width
        lv_obj_align(slider, LV_ALIGN_CENTER, 0, 50); // Position the slider
        lv_slider_set_range(slider, -20, 40); // Set the slider's range

        // Add event callback to the slider
        lv_obj_add_event_cb(slider, slider_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

        // Vertical slider for screen brightness adjustment
        lv_obj_t *slider2 = lv_slider_create(scr);
        lv_obj_set_width(slider2, 20); // Set the slider's width
        lv_obj_set_height(slider2, 200); // Set the slider's height
        lv_obj_align(slider2, LV_ALIGN_RIGHT_MID, 0, 0); // Position the slider
        lv_slider_set_range(slider2, 0, 100); // Set the slider's range
        lv_slider_set_value(slider2, 50, LV_ANIM_OFF); // Set the slider's value

        // Callback for the vertical slider to adjust the screen brightness
        lv_obj_add_event_cb(slider2, brightness_slider_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

      // Create a label for the battery status
        lv_obj_t *battery_label = lv_label_create(scr);

        // Define styles for different battery levels
        static lv_style_t style_green, style_yellow, style_red;
        lv_style_init(&style_green);
        lv_style_set_text_color(&style_green, lv_color_hex(0x00FF00)); // Green
        lv_style_init(&style_yellow);
        lv_style_set_text_color(&style_yellow, lv_color_hex(0xFFFF00)); // Yellow
        lv_style_init(&style_red);
        lv_style_set_text_color(&style_red, lv_color_hex(0xFF0000)); // Red

        // Get the current battery level
        int8_t battery_level = bsp_get_battery_level();

        // Determine the appropriate symbol and style based on the battery level
        const char *battery_symbol = LV_SYMBOL_BATTERY_FULL;  // Default to full battery symbol
        lv_style_t *battery_style = &style_green; // Default to green color
        if (battery_level < 0) {
            battery_symbol = LV_SYMBOL_BATTERY_EMPTY;  // Error or unknown battery status
            battery_style = &style_red; // Use red color for error/unknown status
        } else if (battery_level <= 20) {
            battery_symbol = LV_SYMBOL_BATTERY_1;
            battery_style = &style_red;
        } else if (battery_level <= 40) {
            battery_symbol = LV_SYMBOL_BATTERY_2;
            battery_style = &style_yellow;
        } else if (battery_level <= 60) {
            battery_symbol = LV_SYMBOL_BATTERY_3;
            battery_style = &style_yellow;
        } else if (battery_level <= 80) {
            battery_symbol = LV_SYMBOL_BATTERY_FULL;
            battery_style = &style_yellow;
        } // Above 80% remains green

        // Create a buffer to store the full label text including the battery level
        char battery_label_text[64];
        snprintf(battery_label_text, sizeof(battery_label_text), "%s Battery: %d%%", battery_symbol, battery_level);

        // Update the label with the battery symbol, level, and apply the color style
        lv_label_set_text(battery_label, battery_label_text);
        lv_obj_add_style(battery_label, battery_style, 0); // Apply the style
        lv_obj_align(battery_label, LV_ALIGN_BOTTOM_RIGHT, -10, 0);





        // int8_t battery_level = bsp_get_battery_level();
        // // Combine a battery symbol with the text
        // lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_FULL " Battery: 72%");
        // lv_obj_align(battery_label, LV_ALIGN_BOTTOM_RIGHT, -10, 0);
    }



    // Delete timer when all animation finished
    if ((count += 5) == 220) {
        lv_timer_del(timer);
    } else {
        timer_ctx->count_val = count;
    }
}

void example_lvgl_demo_ui(lv_obj_t *scr) {
    // Create image
    img_logo = lv_img_create(scr);
    lv_img_set_src(img_logo, &esp_logo);
    lv_obj_center(img_logo);

    // Create arcs
    for (size_t i = 0; i < sizeof(arc) / sizeof(arc[0]); i++) {
        arc[i] = lv_arc_create(scr);
        lv_obj_set_size(arc[i], 220 - 30 * i, 220 - 30 * i);
        lv_arc_set_bg_angles(arc[i], 120 * i, 10 + 120 * i);
        lv_arc_set_value(arc[i], 0);
        lv_obj_remove_style(arc[i], NULL, LV_PART_KNOB);
        lv_obj_set_style_arc_width(arc[i], 10, 0);
        lv_obj_set_style_arc_color(arc[i], arc_color[i], 0);
        lv_obj_center(arc[i]);
    }

    // Create timer for animation
    static my_timer_context_t my_tim_ctx = {
        .count_val = -90,
    };
    my_tim_ctx.scr = scr;
    lv_timer_create(anim_timer_cb, 20, &my_tim_ctx);
}
