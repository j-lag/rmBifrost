#include "bifrost_impl.h"

#include "constants.h"

#include <InlineHook.h>
#include <QImage>
#include <QMetaEnum>
#include <QPainter>
#include <spdlog/spdlog.h>

#include "hook_typedefs.h"
#include "BookConfig.h"
#include "gui/boot_screen.h"
#include "gui/ChooseFile_screen.h"

std::weak_ptr<bifrost_impl> bifrost_impl::instance;

void bifrost_impl::initialize()
{
    instance = shared_from_this();
    initialize_hooks();
}

QObject* bifrost_impl::create_ep_fb()
{
    if (hook_passthrough) {
        return create_ep_fb_original();
    }
    spdlog::debug("create_ep_fb hook called");
    auto epfb_inst = create_ep_fb_original();
    start_bifrost(epfb_inst);
    return epfb_inst;
}

void bifrost_impl::start_bifrost(QObject* epfb_inst)
{
    QImage* fb = *reinterpret_cast<QImage**>(reinterpret_cast<void*>(epfb_inst) + 0xc0);
    spdlog::debug("fb: {}x{}x{}", fb->width(), fb->height(), fb->depth());
    spdlog::debug("fb format: {}", QMetaEnum::fromType<QImage::Format>().valueToKey(fb->format()));

    auto screen_update_func = reinterpret_cast<ScreenUpdateFunc>(screen_update_func_addr);

    lvgl_renderer_inst = std::make_shared<lvgl_renderer>(display_config { fb, epfb_inst, screen_update_func });
    lvgl_renderer_inst->initialize();

    // call lvgl_renderer_inst->start() in a separate thread
    auto renderer_thread = std::thread([this] { lvgl_renderer_inst->start(); });
    
    //boot screen selection
    //{
    //    auto boot_screen_inst = std::make_shared<boot_screen>(lvgl_renderer_inst);
    //    auto boot_screen_thread = std::thread([&] { boot_screen_inst->start(); });
    //
    //    boot_screen_thread.join();
    //
    //    if (boot_screen_inst->state == RM_STOCK_OS) {
    //        lvgl_renderer_inst->stop();
    //        renderer_thread.join();
    //
    //        spdlog::debug("Relinquished control flow to the stock OS");
    //        hook_passthrough = true;
    //        return;
    //    }
    //}
    

    //app
    {
        spdlog::debug("Starting App");
        //read configuration
        BookConfig::GetInstance().Init("/BookConfig.json");
        //run app
        auto ChooseFile_screen_inst = std::make_shared<ChooseFile_screen>(lvgl_renderer_inst);
        auto ChooseFile_screen_thread = std::thread([&] { ChooseFile_screen_inst->start(BookConfig::GetInstance().GetCurrentFolder().c_str()); });
        ChooseFile_screen_thread.join();
    }
    spdlog::debug("End App");
    renderer_thread.join();
}

void bifrost_impl::initialize_hooks()
{
    BWSR_InlineHook(
        create_ep_fb_addr, reinterpret_cast<void*>(create_ep_fb_hook),
        reinterpret_cast<void**>(&create_ep_fb_original), nullptr, nullptr);
}
