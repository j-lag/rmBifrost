#ifndef BOOT_SCREEN_H
#define BOOT_SCREEN_H
#include <condition_variable>
#include <vector>
#include <cstdint>
#include <stack>

#include "lvgl_renderer.h"

enum launch_state {
    IN_FLIGHT,
    RM_STOCK_OS,
    BIFROST,
};


class boot_screen : public std::enable_shared_from_this<boot_screen> {
public:
    launch_state state = IN_FLIGHT;

    void start();

    explicit boot_screen(const std::shared_ptr<lvgl_renderer> &shared) : lvgl_renderer_inst(shared) {
    }

    ~boot_screen();

private:
    static std::weak_ptr<boot_screen> instance;

    std::vector<uint8_t> lottie_buf;
    std::vector<uint8_t> welcome_json;
    std::shared_ptr<lvgl_renderer> lvgl_renderer_inst;
    std::stack<lv_obj_t *> deletion_queue;

    std::condition_variable cv;
    std::mutex cv_m;

    lv_obj_t *lottie_obj;

    void setup_animation();

    lv_obj_t *create_boot_option(const char *title);

    void setup_boot_selection();
};


#endif //BOOT_SCREEN_H
