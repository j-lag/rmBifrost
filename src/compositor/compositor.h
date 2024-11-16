
#ifndef COMPOSITOR_H
#define COMPOSITOR_H
#include "../gui/lvgl_renderer.h"
#include "../utils/shm_channel.h"
#include "../utils/unix_socket.h"
#include "compositor_client.h"
#include "../gui/system_ui.h"
#include <memory>
#include <thread>

class compositor {
public:
    struct display_config {
        QImage* fb;
        QObject* epfb_inst;
        ScreenUpdateFunc screen_update_func;
    };

    explicit compositor(display_config cfg);

    void start();
    void stop();
private:
    static uint32_t next_client_id;
    std::atomic<bool> running = false;

    display_config cfg;
    std::shared_ptr<lvgl_renderer> renderer;
    std::shared_ptr<system_ui> system_ui_inst;
    std::unique_ptr<unix_socket> socket;
    std::thread listener_thread;
    std::thread render_thread;

    std::mutex client_mutex;
    std::vector<std::shared_ptr<compositor_client>> clients;
    std::shared_ptr<compositor_client> active_client;

    std::vector<std::pair<rect, refresh_type>> pending_refresh_requests;

    int fps = 0;
    std::chrono::time_point<std::chrono::system_clock> last_fps_update;

    std::vector<uint8_t *> canvas_buf_deletion_queue;

    void listener();
    void refresh(point p1, point p2, refresh_type type) const;
    void render_clients();
    void set_active_client(const std::shared_ptr<compositor_client> &client);
    void request_refresh(rect update_region, refresh_type type);
};



#endif //COMPOSITOR_H
