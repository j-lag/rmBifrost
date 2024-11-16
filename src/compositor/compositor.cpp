#include "compositor.h"

#include <spdlog/spdlog.h>
#include "../gui/boot_screen.h"
#include "../gui/ChooseFile_screen.h"
#include "../BookConfig.h"
#include <utility>

compositor::compositor(display_config cfg)
    : cfg(cfg)
      , socket(std::make_unique<unix_socket>("/run/bifrost_comp_ctl.sock", true))
      , renderer(std::make_shared<lvgl_renderer>(
          cfg.fb,
          [this](auto &&PH1, auto &&PH2) {
              request_refresh(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
          })) {
    cfg.fb->fill(QColor(255, 255, 255));
    renderer->initialize();
}

void compositor::start() {
    spdlog::info("Starting bifrost compositor");
    running = true;

    render_thread = std::thread([this]() {
        int freq = 1000000 / 85;
        long last_tick = 0;
        while (running) {
            render_clients();
            renderer->tick();

            for (auto &buffer: canvas_buf_deletion_queue) {
                delete[] buffer;
            }
            canvas_buf_deletion_queue.clear();

            for (const auto &[req_region, req_type]: pending_refresh_requests) {
                refresh(req_region.p1, req_region.p2, req_type);
            }
            pending_refresh_requests.clear();
            long now = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            long sleep_time = freq - (now - last_tick);
            if (sleep_time > 0) {
                std::this_thread::sleep_for(std::chrono::microseconds(sleep_time));
            }
            last_tick = now;

            if (std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now() - last_fps_update).count() >= 10) {
                spdlog::info("FPS: {}", fps);
                fps = 0;
                last_fps_update = std::chrono::system_clock::now();
            } else {
                fps++;
            }
        }
    });


    /*
    auto boot_screen_inst = std::make_shared<boot_screen>(renderer);
    auto boot_screen_thread = std::thread([&] { boot_screen_inst->start(); });

    boot_screen_thread.join();

    if (boot_screen_inst->state == RM_STOCK_OS) {
        running = false;;
        render_thread.join();

        spdlog::debug("Relinquished control flow to the stock OS");
        return;
    }

    spdlog::debug("User selected Bifrost; starting system UI");
    boot_screen_inst.reset();


    system_ui_inst = std::make_shared<system_ui>(renderer);
    system_ui_inst->initialize();
    system_ui_inst->set_content({system_ui::content_type::BIFROST, "Bifrost"});
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    */

   {
       spdlog::debug("Starting App");
       //read configuration
       BookConfig::GetInstance().Init("/home/root/BookConfig.json");
       //run app
       auto ChooseFile_screen_inst = std::make_shared<ChooseFile_screen>(renderer);
       auto ChooseFile_screen_thread = std::thread([&] { ChooseFile_screen_inst->start(BookConfig::GetInstance().GetCurrentFolder().c_str()); });
       ChooseFile_screen_thread.join();
       spdlog::debug("End App");
       ChooseFile_screen_inst.reset();
   }

   

    listener_thread = std::thread(&compositor::listener, this);
    render_thread.join();
}

void compositor::request_refresh(rect update_region, refresh_type type) {
    if (pending_refresh_requests.empty()) {
        pending_refresh_requests.emplace_back(update_region, type);
    } else {
        for (auto &[req_region, req_type]: pending_refresh_requests) {
            if (req_region.intersects(update_region)) {
                req_region = req_region.union_(update_region);
                req_type = std::max(req_type, type);
                return;
            }
        }
        pending_refresh_requests.emplace_back(update_region, type);
    }
}

void compositor::render_clients() {
    std::lock_guard lock(client_mutex);

    if (system_ui_inst && system_ui_inst->requested_application_exit()) {
        active_client->stop();
        set_active_client(nullptr);
    }

    // Remove disconnected clients
    clients.erase(std::remove_if(clients.begin(), clients.end(), [this](const auto &client) {
        if (client->state == compositor_client::client_state::DISCONNECTED) {
            spdlog::info("Client {} has disconnected", client->application_name);
            canvas_buf_deletion_queue.push_back(client->lvgl_canvas_buffer);
            return true;
        }
        return false;
    }), clients.end());

    for (const auto &client: clients) {
        auto refresh_area = client->blit_to_canvas();
        if (!refresh_area) {
            return;
        }
        request_refresh(refresh_area->first, refresh_area->second);
    }
}

void compositor::set_active_client(const std::shared_ptr<compositor_client> &client) {
    if (client) {
        system_ui_inst->set_content({system_ui::content_type::APPLICATION, client->window_title});
    } else {
        system_ui_inst->set_content({system_ui::content_type::BIFROST, "Bifrost"});
    }
    active_client = client;
}

void compositor::stop() {
    running = false;
    for (const auto &client: clients) {
        client->stop();
    }
    listener_thread.join();
}

void compositor::listener() {
    while (running) {
        auto connection = socket->accept_connection();
        auto client = std::make_shared<compositor_client>(std::move(connection),
                                                          compositor_client::compositor_client_config{
                                                              .id = next_client_id++,
                                                              .navbar_height = 100,
                                                              .swapchain_extent = {SCREEN_WIDTH, SCREEN_HEIGHT},
                                                              .pos = {0, 0}
                                                          });

        {
            std::lock_guard lock(client_mutex);
            clients.push_back(client);
            set_active_client(client);
        }

        client->start();
    }
}

void compositor::refresh(const point p1, const point p2, const refresh_type type) const {
    spdlog::debug("Refreshing area: {}x{}-{}x{} with type {}", p1.x, p1.y, p2.x, p2.y, static_cast<int>(type));
    switch (type) {
        case MONOCHROME:
            cfg.screen_update_func(cfg.epfb_inst, p1, p2, 0, 0, 0);
            break;
        case COLOR_ANIMATION:
            cfg.screen_update_func(cfg.epfb_inst, p1, p2, 1, 0, 0);
            break;
        case COLOR_CONTENT:
            cfg.screen_update_func(cfg.epfb_inst, p1, p2, 1, 4, 0);
            break;
        case COLOR_FAST:
            cfg.screen_update_func(cfg.epfb_inst, p1, p2, 1, 1, 0);
            break;
        case COLOR_1:
            cfg.screen_update_func(cfg.epfb_inst, p1, p2, 1, 3, 0);
            break;
        case COLOR_2:
            cfg.screen_update_func(cfg.epfb_inst, p1, p2, 1, 5, 0);
            break;
        case FULL:
            cfg.screen_update_func(cfg.epfb_inst, p1, p2, 1, 4, 1);
            break;
        default: break;
    }
}

uint32_t compositor::next_client_id = 0;
