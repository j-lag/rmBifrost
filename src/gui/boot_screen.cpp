#include "boot_screen.h"

#include <thread>
#include <spdlog/spdlog.h>

#include "../constants.h"
#include "../resources.h"
#include <src/core/lv_obj_pos.h>
#include <src/display/lv_display.h>
#include <src/misc/lv_anim.h>
#include <src/widgets/lottie/lv_lottie.h>
#include <src/misc/lv_anim.h>

#include "components/message_box.h"

void boot_screen::start() {
    spdlog::debug("Presenting launch options");
    lvgl_renderer_inst->set_global_refresh_hint(MONOCHROME);

    /*
    {
        std::lock_guard lock(g_lvgl_mutex);
        spdlog::debug("Creating message box");
        auto msg_box = new message_box(lv_layer_top());
        msg_box->add_title("Welcome to Bifrost");
        msg_box->add_button("OK", [] {});
        msg_box->add_button("Cancel", [] {});
        msg_box->show(300, 200);
        lv_obj_set_size(msg_box->msgbox, LV_PCT(80), LV_PCT(30));
        spdlog::debug("Message box created");
    }

    // msg_box.add_button("OK", [] {});
    // msg_box.add_button("Cancel", [] {});
    std::this_thread::sleep_for(std::chrono::milliseconds(5000000));
    */
    // setup_animation();
    // std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    lvgl_renderer_inst->set_global_refresh_hint(MONOCHROME);
    setup_boot_selection();

    // wait for the user to select an option
    std::unique_lock lk(cv_m);
    cv.wait(lk, [this] { return state != IN_FLIGHT; });
    lk.unlock();

    spdlog::debug("User selected an option; exiting boot screen");
}

boot_screen::~boot_screen() {
    std::lock_guard lock(g_lvgl_mutex);
    while (!deletion_queue.empty()) {
        lv_obj_delete(deletion_queue.top());
        deletion_queue.pop();
    }
    spdlog::debug("Boot screen deleted");
}

void boot_screen::setup_animation() {
    std::lock_guard lock(g_lvgl_mutex);

    welcome_json = get_resource_file("animations/hello.json");

    lottie_obj = lv_lottie_create(lv_layer_top());
    lv_lottie_set_src_data(lottie_obj, welcome_json.data(), welcome_json.size());
    lv_anim_set_repeat_count(lv_lottie_get_anim(lottie_obj), 1);

    int width = static_cast<int>(std::round(SCREEN_WIDTH / 1.5));
    int height = width / 3;
    lottie_buf.resize(width * height * 4);
    lv_lottie_set_buffer(lottie_obj, width, height, lottie_buf.data());

    lv_obj_align(lottie_obj, LV_ALIGN_CENTER, 0, 0);

    deletion_queue.push(lottie_obj);
}

lv_obj_t *boot_screen::create_boot_option(const char *title) {
    auto btn = lv_obj_create(lv_layer_top());
    lv_obj_set_size(btn, LV_PCT(40), LV_SIZE_CONTENT);
    lv_obj_set_style_border_color(btn, lv_color_black(), 0);
    lv_obj_set_style_border_width(btn, 5, 0);
    lv_obj_set_style_bg_color(btn, lv_color_white(), 0);
    lv_obj_set_style_radius(btn, 0, 0);
    lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(btn, 40, 0);
    lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Create the title label
    auto title_label = lv_label_create(btn);
    lv_label_set_text(title_label, title);
    lv_obj_set_style_text_font(title_label, &ebgaramond_48, 0);
    lv_obj_set_size(title_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    // offset the title label
    lv_obj_set_style_margin_top(title_label, 10, 0);

    // Create the subtitle label
    auto subtitle_label = lv_label_create(btn);
    lv_label_set_text(subtitle_label, LV_SYMBOL_RIGHT);
    lv_obj_set_size(subtitle_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    // Add the box to the deletion queue
    deletion_queue.push(btn);
    deletion_queue.push(title_label);
    deletion_queue.push(subtitle_label);
    return btn;
}

void boot_screen::setup_boot_selection() {
    std::lock_guard lock(g_lvgl_mutex);

    auto remarkable = create_boot_option("reMarkable OS");
    lv_obj_align(remarkable, LV_ALIGN_BOTTOM_MID, 0, -425);

    lv_obj_add_event_cb(remarkable, [](lv_event_t *event) {
        spdlog::debug("Launching reMarkable OS");
        if (auto instance = boot_screen::instance.lock()) {
            std::lock_guard<std::mutex> lk(instance->cv_m);
            instance->state = RM_STOCK_OS;
            instance->cv.notify_one();
        }
    }, LV_EVENT_CLICKED, nullptr);

    auto bifrost = create_boot_option("Bifrost");
    lv_obj_align(bifrost, LV_ALIGN_BOTTOM_MID, 0, -200);

    instance = shared_from_this();

    lv_obj_add_event_cb(bifrost, [](lv_event_t *event) {
        spdlog::debug("Launching Bifrost");
        if (auto instance = boot_screen::instance.lock()) {
            std::lock_guard<std::mutex> lk(instance->cv_m);
            instance->state = BIFROST;
            instance->cv.notify_one();
        }
    }, LV_EVENT_CLICKED, nullptr);
}

std::weak_ptr<boot_screen> boot_screen::instance;
