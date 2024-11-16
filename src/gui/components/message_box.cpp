//

#include "message_box.h"

#include <spdlog/spdlog.h>
#include <src/core/lv_obj_tree.h>
#include <src/widgets/msgbox/lv_msgbox.h>

message_box::message_box(lv_obj_t *parent)
    : parent(parent) {
    msgbox = lv_msgbox_create(parent);
    lv_obj_set_style_radius(msgbox, 0, 0);
    lv_obj_set_style_border_color(msgbox, lv_color_black(), 0);
    lv_obj_set_style_border_width(msgbox, 5, 0);
    lv_obj_set_style_bg_color(msgbox, lv_color_white(), 0);
    deletion_queue.push(msgbox);

    msgbox_content = lv_msgbox_get_content(msgbox);
    lv_obj_set_style_border_side(msgbox_content, static_cast<lv_border_side_t>(LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_TOP), 0);
    lv_obj_set_style_border_color(msgbox_content, lv_color_black(), 0);
    lv_obj_set_style_border_width(msgbox_content, 5, 0);

    lv_style_init(&btn_default_style);
    lv_style_init(&btn_pressed_style);

    lv_style_set_radius(&btn_default_style, 0);
    lv_style_set_radius(&btn_pressed_style, 0);

    lv_style_set_bg_color(&btn_default_style, lv_color_white());
    lv_style_set_bg_color(&btn_pressed_style, lv_color_black());

    lv_style_set_text_color(&btn_default_style, lv_color_black());
    lv_style_set_text_color(&btn_pressed_style, lv_color_white());

    lv_style_set_flex_grow(&btn_default_style, 1);
    lv_style_set_flex_grow(&btn_pressed_style, 1);

    lv_style_transition_dsc_init(&btn_style_transition, btn_style_props, lv_anim_path_linear, 0, 0, nullptr);
    lv_style_set_transition(&btn_pressed_style, &btn_style_transition);
}

void message_box::add_title(const std::string &title) const {
    lv_msgbox_add_title(msgbox, title.c_str());
    auto title_bar = lv_msgbox_get_header(msgbox);
    lv_obj_set_style_border_width(title_bar, 0, 0);
    lv_obj_set_style_bg_color(title_bar, lv_color_white(), 0);
    lv_obj_set_style_radius(title_bar, 0, 0);
    lv_obj_set_style_pad_left(title_bar, 20, 0);
    lv_obj_set_height(title_bar, LV_PCT(18));
}

void message_box::add_button(std::string text, std::function<void()> callback) {
    has_footer = true;
    button_callbacks.push_back(callback);
    auto btn = lv_msgbox_add_footer_button(msgbox, text.c_str());
    auto data = new callback_data {this, button_callbacks.size() - 1};
    button_data.push_back(data);

    lv_obj_add_event_cb(btn, button_event_cb, LV_EVENT_CLICKED, data);
    deletion_queue.push(btn);

    // if (button_callbacks.size() > 1) {
    //     lv_obj_set_style_border_side(btn, LV_BORDER_SIDE_LEFT, 0);
    //     lv_obj_set_style_border_color(btn, lv_color_black(), 0);
    //     lv_obj_set_style_border_width(btn, 5, 0);
    // }

    lv_obj_add_style(btn, &btn_default_style, 0);
    lv_obj_add_style(btn, &btn_pressed_style, LV_STATE_PRESSED);
}

message_box::~message_box() {
    while (!deletion_queue.empty()) {
        lv_obj_delete(deletion_queue.top());
        deletion_queue.pop();
    }

    for (auto data : button_data) {
        delete data;
    }
}

void message_box::show(int width, int height) {
    if (has_footer) {
        msgbox_footer = lv_msgbox_get_footer(msgbox);
        lv_obj_set_style_border_width(msgbox_footer, 0, 0);
        lv_obj_set_style_pad_all(msgbox_footer, 0, 0);
        lv_obj_set_height(msgbox_footer, LV_PCT(18));
        lv_obj_set_style_pad_gap(msgbox_footer, 0, 0);
    }

    lv_obj_set_size(msgbox, width, height);
}

void message_box::button_event_cb(lv_event_t *e) {
    auto data = reinterpret_cast<callback_data *>(lv_event_get_user_data(e));
    data->inst->button_callbacks[data->id]();
}
