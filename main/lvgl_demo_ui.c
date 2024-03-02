#include <math.h>
#include "lvgl.h"

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

    }

    // Move images when arc animation finished
    // if ((count >= 100) && (count <= 180)) {
    //     lv_coord_t offset = (sinf((count - 140) * 2.25f / 90.0f) + 1) * 20.0f;
    //     lv_obj_align(img_logo, LV_ALIGN_CENTER, 0, -offset);
    //     lv_obj_align(img_text, LV_ALIGN_CENTER, 0, 2 * offset);
    //     lv_obj_set_style_img_opa(img_text, offset / 40.0f * 255, 0);
    // }

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
