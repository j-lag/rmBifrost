#include "system_ui.h"

#include <spdlog/spdlog.h>

system_ui::system_ui(std::shared_ptr<lvgl_renderer> renderer, int title_bar_height)
    : renderer(renderer), title_bar_height(title_bar_height)
{
    
}

static void pressing_cb(lv_event_t * e)
{
    // print coordinates
    lv_point_t point;
    lv_indev_get_point(lv_indev_active(), &point);
    spdlog::info("x: {}, y: {}", point.x, point.y);
}

void system_ui::initialize()
{
    instance = shared_from_this();

    std::lock_guard<std::mutex> lock(g_lvgl_mutex);

    navbar = lv_obj_create(lv_layer_top());
    lv_obj_set_size(navbar, LV_PCT(100), title_bar_height);
    lv_obj_set_style_border_side(navbar, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(navbar, lv_color_black(), 0);
    lv_obj_set_style_border_width(navbar, 5, 0);
    lv_obj_set_style_bg_color(navbar, lv_color_white(), 0);
    lv_obj_set_style_radius(navbar, 0, 0);
    lv_obj_set_flex_flow(navbar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(navbar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(navbar, 0, 0);

    auto button_size = title_bar_height - 5;

    left_action_wrapper = lv_obj_create(navbar);
    lv_obj_set_size(left_action_wrapper, button_size, button_size);
    lv_obj_set_flex_flow(left_action_wrapper, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(left_action_wrapper, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(left_action_wrapper, 0, 0);
    lv_obj_set_style_border_width(left_action_wrapper, 0, 0);

    left_action = lv_label_create(left_action_wrapper);
    lv_label_set_text(left_action, "\uEB4B");
    lv_obj_set_size(left_action, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(left_action, &ionicons, 0);

    title_label = lv_label_create(navbar);
    lv_label_set_text(title_label, "Bifrost");
    lv_obj_set_style_margin_top(title_label, 15, 0);
    lv_obj_set_style_text_font(title_label, &ebgaramond_48, 0);
    lv_obj_set_size(title_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);


    right_action_wrapper = lv_obj_create(navbar);
    lv_obj_set_size(right_action_wrapper, button_size, button_size);
    lv_obj_set_flex_flow(right_action_wrapper, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(right_action_wrapper, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(right_action_wrapper, 0, 0);
    lv_obj_set_style_border_width(right_action_wrapper, 0, 0);
    right_action = lv_label_create(right_action_wrapper);
    lv_label_set_text(right_action, "\uEB42");
    lv_obj_set_style_text_font(right_action, &ionicons, 0);
    lv_obj_set_size(right_action, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    deletion_queue.push(navbar);
    deletion_queue.push(left_action);
    deletion_queue.push(title_label);
    deletion_queue.push(right_action);

    renderer->request_full_refresh();

    lv_obj_add_event_cb(lv_layer_top(), gesture_cb, LV_EVENT_GESTURE, nullptr);
    lv_obj_add_event_cb(left_action_wrapper, left_action_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(right_action_wrapper, right_action_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(lv_layer_top(), pressing_cb, LV_EVENT_PRESSING, nullptr);

    // add slider

}

system_ui::~system_ui()
{
    while (!deletion_queue.empty()) {
        lv_obj_del(deletion_queue.top());
        deletion_queue.pop();
    }
}

void system_ui::set_content(content_info info)
{
    std::lock_guard<std::mutex> lock(g_lvgl_mutex);
    if (info.type == content_type::BIFROST) {
        lv_label_set_text(title_label, info.title.c_str());
        lv_obj_add_flag(left_action, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(right_action, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(left_action_wrapper, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_flag(right_action_wrapper, LV_OBJ_FLAG_CLICKABLE);
    } else if (info.type == content_type::APPLICATION) {
        lv_label_set_text(title_label, info.title.c_str());
        lv_obj_remove_flag(left_action, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(right_action, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(left_action_wrapper, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(right_action_wrapper, LV_OBJ_FLAG_CLICKABLE);
    }
    current_content = info;
}

bool system_ui::requested_application_exit() {
    if (application_exit_requested) {
        application_exit_requested = false;
        return true;
    }
    return false;
}

void system_ui::gesture_cb(lv_event_t * e)
{
    auto instance = system_ui::instance.lock();
    if (!instance) {
        return;
    }

    lv_obj_t *screen = reinterpret_cast<lv_obj_t *>(lv_event_get_current_target(e));
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());

    lv_indev_t *indev = lv_indev_active();
    if (indev == NULL)
        return;

    lv_point_t point;
    lv_indev_get_point(indev, &point);

    if (dir == LV_DIR_BOTTOM && point.y <= 2.5 * instance->title_bar_height) {
        lv_obj_remove_flag(instance->navbar, LV_OBJ_FLAG_HIDDEN);
        spdlog::info("Swipe gesture from top edge detected. Unhiding title bar.");
    }
}

void system_ui::left_action_cb(lv_event_t * e)
{
    auto instance = system_ui::instance.lock();
    if (!instance) {
        return;
    }
    assert(instance->current_content.type == content_type::APPLICATION);

    instance->application_exit_requested = true;
}

void system_ui::right_action_cb(lv_event_t * e)
{
    auto instance = system_ui::instance.lock();
    if (!instance) {
        return;
    }
    assert(instance->current_content.type == content_type::APPLICATION);

    lv_obj_add_flag(instance->navbar, LV_OBJ_FLAG_HIDDEN);
}

std::weak_ptr<system_ui> system_ui::instance;