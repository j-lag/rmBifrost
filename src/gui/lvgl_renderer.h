
#ifndef LVGL_APP_H
#define LVGL_APP_H

#include "../constants.h"
#include "../hook_typedefs.h"
#include "lv_conf.h"

#include <QImage>
#include <condition_variable>
#include <lvgl.h>
#include <memory>

class lvgl_renderer : public std::enable_shared_from_this<lvgl_renderer> {
public:
    explicit lvgl_renderer(QImage* fb, std::function<void(rect, refresh_type)> refresh_func)
        : fb(fb),refresh_func(refresh_func)
    {
    }
    void initialize();
    void request_full_refresh();
    void set_global_refresh_hint(refresh_type hint) { global_refresh_hint = hint; }
    void tick();
    ~lvgl_renderer();
private:
    static std::weak_ptr<lvgl_renderer> instance;
    QImage* fb;
    std::function<void(rect, refresh_type)> refresh_func;
    lv_display_t* display;
    uint8_t* composite_buffer;
    long last_full_refresh_time = 0;
    bool full_refresh_requested = false;

    refresh_type global_refresh_hint = MONOCHROME;

    void lv_display_flush(lv_display_t* disp, const lv_area_t* area, uint8_t* color_p);
    static void lv_display_flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* color_p);
};

#endif // LVGL_APP_H
