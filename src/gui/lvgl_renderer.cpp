#include "lvgl_renderer.h"

#include "../constants.h"

#include <spdlog/spdlog.h>

std::weak_ptr<lvgl_renderer> lvgl_renderer::instance;

uint32_t lv_tick_cb()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void lvgl_renderer::lv_display_flush(lv_display_t* disp, const lv_area_t* area, uint8_t* color_p)
{
    //spdlog::debug("full refresh");
    //cfg.screen_update_func(cfg.epfb_inst, { area->x1, area->y1 }, { area->x2, area->y2 }, 1, 1, 0);
    //spdlog::debug("flush refresh");
    //lv_display_flush_ready(disp);
    //spdlog::debug("end refresh");
    //return;

    int fb_width = cfg.fb->width();
    int fb_height = cfg.fb->height();
    int fb_depth = cfg.fb->depth() / 8;

    point p1 = { area->x2, area->y2 };
    point p2 = { area->x1, area->y1 };
    bool pixel_changed = false;
    // copy the area from the lvgl buffer to the QImage buffer
    for (int y = area->y1; y <= area->y2; y++) {
        QRgb* line = reinterpret_cast<QRgb*>(cfg.fb->scanLine(y));
        int lvgl_idx = (y * fb_width + area->x1) * fb_depth;
        for (int x = area->x1; x <= area->x2; x++) {
            // find the smallest rectangle that contains pixel difference
            if (line[x] != qRgb(color_p[lvgl_idx + 2], color_p[lvgl_idx + 1], color_p[lvgl_idx + 0])) {
                pixel_changed = true;
                p1.x = std::min(p1.x, x);
                p1.y = std::min(p1.y, y);
                p2.x = std::max(p2.x, x);
                p2.y = std::max(p2.y, y);
            }
            if (fb_depth == 4) {
                line[x] = qRgba(color_p[lvgl_idx + 2], color_p[lvgl_idx + 1], color_p[lvgl_idx + 0], 255);
            } else {
                line[x] = qRgb(color_p[lvgl_idx + 1], color_p[lvgl_idx + 2], color_p[lvgl_idx + 3]);
            }
            lvgl_idx += 4;
        }
    }

    spdlog::debug("requested flushing area: {} -> {}x{}-{}x{}; actual flushing area: {}x{}-{}x{}",
                  pixel_changed, area->x1, area->y1, area->x2, area->y2, p1.x, p1.y, p2.x, p2.y);

    if (pixel_changed) {
        refresh(p1, p2, global_refresh_hint);
    } else {
        // auto current_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        // if (current_time - last_full_refresh_time < 250) {
        //     spdlog::debug("skip full refresh");
        //     lv_display_flush_ready(disp);
        //     return;
        // }
        // spdlog::debug("full refresh");
        // last_full_refresh_time = current_time;
        // cfg.screen_update_func(cfg.epfb_inst, {area->x1, area->y1}, {area->x2, area->y2}, 1, 1, 0);
    }
    lv_display_flush_ready(disp);

}

void lvgl_renderer::lv_display_flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* color_p)
{
    auto instance = lvgl_renderer::instance.lock();
    if (!instance)
        return;

    instance->lv_display_flush(disp, area, color_p);
}

void lvgl_renderer::initialize()
{
    instance = shared_from_this();

    lv_init();
    lv_tick_set_cb(lv_tick_cb);
    display = lv_display_create(cfg.fb->width(), cfg.fb->height());

    lv_indev_t* touch = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event3");
    lv_evdev_set_calibration(touch, 0, 0, 2160, 2880);
    lv_indev_set_display(touch, display);

    auto buf_size = cfg.fb->width() * cfg.fb->height() * cfg.fb->depth() / 8;
    compose_buffer = new uint8_t[buf_size];
    lv_display_set_buffers(display, compose_buffer, nullptr, buf_size, LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_flush_cb(display, lv_display_flush_cb);

    lv_obj_set_style_bg_color(lv_screen_active(), LV_COLOR_MAKE(255, 255, 255), LV_STATE_DEFAULT);
    lv_obj_set_style_opa(lv_screen_active(), LV_OPA_COVER, LV_STATE_DEFAULT);

    spdlog::info("lvgl renderer initialized.");
}

void lvgl_renderer::start()
{
    spdlog::debug("lvgl renderer main loop.");
    int freq = 1000000 / 85;
    long last_tick = 0;
    while (running) {
        tick();
        long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        long sleep_time = freq - (now - last_tick);
        if (sleep_time > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(sleep_time));
        }
        last_tick = now;
    }
}

void lvgl_renderer::stop() {
    running = false;
}

void lvgl_renderer::refresh(point p1, point p2, refresh_type type) {
    switch (type) {
        case MONOCHROME:
            cfg.screen_update_func(cfg.epfb_inst, p1, p2, 0, 0, 0);
            break;
        case COLOR_FAST:
            cfg.screen_update_func(cfg.epfb_inst, p1, p2, 1, 0, 0);
            break;
        case COLOR_CONTENT:
            cfg.screen_update_func(cfg.epfb_inst, p1, p2, 1, 4, 0);
            break;
        case COLOR_1:
            cfg.screen_update_func(cfg.epfb_inst, p1, p2, 1, 1, 0);
            break;
        case COLOR_2:
            cfg.screen_update_func(cfg.epfb_inst, p1, p2, 1, 3, 0);
            break;
        case COLOR_3:
            cfg.screen_update_func(cfg.epfb_inst, p1, p2, 1, 5, 0);
            break;
        case FULL:
            cfg.screen_update_func(cfg.epfb_inst, p1, p2, 1, 4, 1);
            break;
    }
}

lvgl_renderer::~lvgl_renderer() {
    // TODO: free resources
}

void lvgl_renderer::tick()
{
    lv_timer_handler();
}
