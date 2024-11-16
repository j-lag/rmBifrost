//

#ifndef MESSAGE_BOX_H
#define MESSAGE_BOX_H
#include <functional>
#include <string>
#include <stack>
#include <src/misc/lv_style.h>
#include <src/misc/lv_types.h>


class message_box {
public:
    struct callback_data {
        message_box* inst;
        size_t id;
    };

    explicit message_box(lv_obj_t *parent);
    void add_title(const std::string &title) const;
    void add_button(std::string text, std::function<void()> callback);
    ~message_box();

    void show(int width, int height);

    lv_obj_t * msgbox = nullptr;
private:
    lv_obj_t * parent = nullptr;

    std::stack<lv_obj_t *> deletion_queue;
    lv_obj_t * msgbox_content = nullptr;
    lv_obj_t * msgbox_footer = nullptr;

    bool has_footer = false;

    std::vector<std::function<void()>> button_callbacks;
    std::vector<callback_data *> button_data;

    lv_style_t btn_default_style;
    lv_style_t btn_pressed_style;
    lv_style_transition_dsc_t btn_style_transition;
    lv_style_prop_t btn_style_props[3] = {LV_STYLE_BG_COLOR, LV_STYLE_TEXT_COLOR};

    static void button_event_cb(lv_event_t * e);
};



#endif //MESSAGE_BOX_H
