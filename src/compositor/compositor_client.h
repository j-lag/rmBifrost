
#ifndef COMPOSITOR_CLIENT_H
#define COMPOSITOR_CLIENT_H
#include "../utils/data_structs.h"
#include "../utils/shm_channel.h"
#include "../utils/unix_socket.h"
#include "packets/begin_session_response.h"
#include "packets/packet.h"
#include "packets/submit_frame_packet.h"

#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include <optional>
#include <src/misc/lv_types.h>

class compositor_client {
public:
    struct compositor_client_config {
        uint32_t id;
        uint32_t navbar_height;
        extent swapchain_extent;
        point pos;
    };

    enum class client_state {
        CONNECTED = 0,
        SESSION_STARTED = 1,
        DISCONNECTED = 2
    };

    explicit compositor_client(std::unique_ptr<unix_socket::connection> conn, compositor_client_config cfg);


    void start();
    void stop();
    std::optional<std::tuple<uint32_t, submit_frame_packet, rect, uint64_t>> get_swapchain_image();
    void release_swapchain_image(uint32_t frame_id);
    std::optional<std::pair<rect, refresh_type>> blit_to_canvas();

    std::string application_name = "Untitled";
    std::string window_title = "Untitled";
    bool prefer_full_screen = false;
    client_state state = client_state::CONNECTED;
    uint8_t *lvgl_canvas_buffer;
private:
    void handle_packet(const std::shared_ptr<packet>& packet);
    void send_packet(const std::shared_ptr<packet>& resp);
    std::vector<uint64_t> create_swapchain_images();
    void create_lvgl_canvas();

    compositor_client_config cfg;
    std::thread client_thread;
    std::unique_ptr<unix_socket::connection> conn;
    std::atomic<bool> running;

    uint32_t swapchain_image_count = 1;
    extent swapchain_extent = {SCREEN_WIDTH, SCREEN_HEIGHT};
    rect composite_region = {{0, 0}, {SCREEN_WIDTH, SCREEN_HEIGHT}};

    std::string session_name;
    size_t aligned_image_size;
    std::unique_ptr<shm_channel> shared_memory;
    std::vector<bool> framebuffer_in_flight;
    std::vector<submit_frame_packet> submission_info;
    std::queue<uint32_t> submitted_frame_ids;

    std::mutex packet_write_mutex;

    lv_obj_t* lvgl_canvas = nullptr;
};

#endif // COMPOSITOR_CLIENT_H
