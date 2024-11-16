#ifndef SYSTEM_UI_H
#define SYSTEM_UI_H
#include <stack>

#include "../gui/lvgl_renderer.h"

#include <memory>

class system_ui : public std::enable_shared_from_this<system_ui> {
public:
    enum content_type {
        BIFROST,
        APPLICATION
    };
    struct content_info {
        content_type type;
        std::string title;
    };

    explicit system_ui(std::shared_ptr<lvgl_renderer> renderer, int title_bar_height = 100);
    void initialize();
    void set_content(content_info info);
    bool requested_application_exit();

    ~system_ui();
private:
    static std::weak_ptr<system_ui> instance;
    std::shared_ptr<lvgl_renderer> renderer;
    std::stack<lv_obj_t *> deletion_queue; 
    int title_bar_height;

    lv_obj_t* navbar;
    lv_obj_t* title_label;
    lv_obj_t* left_action;
    lv_obj_t* left_action_wrapper;
    lv_obj_t* right_action;
    lv_obj_t* right_action_wrapper;

    content_info current_content = {content_type::BIFROST, "Bifrost"};
    bool application_exit_requested = false;


    static void gesture_cb(lv_event_t * e);
    static void left_action_cb(lv_event_t * e);
    static void right_action_cb(lv_event_t * e);
};



#endif //SYSTEM_UI_H
