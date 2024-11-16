#include "bifrost_impl.h"

#include "constants.h"

#include <InlineHook.h>
#include <QImage>
#include <QMetaEnum>
#include <QPainter>
#include <spdlog/spdlog.h>

#include "hook_typedefs.h"
#include "gui/boot_screen.h"

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
    QImage* fb = *reinterpret_cast<QImage**>(reinterpret_cast<uintptr_t>(epfb_inst) + 0xc0);
    spdlog::debug("fb: {}x{}x{}", fb->width(), fb->height(), fb->depth());
    spdlog::debug("fb format: {}", QMetaEnum::fromType<QImage::Format>().valueToKey(fb->format()));

    auto screen_update_func = reinterpret_cast<ScreenUpdateFunc>(screen_update_func_addr);

    compositor_inst = std::make_shared<compositor>(compositor::display_config{fb, epfb_inst, screen_update_func});
    compositor_inst->start();

    hook_passthrough = true;
    return;
}

void bifrost_impl::initialize_hooks()
{
    BWSR_InlineHook(
        create_ep_fb_addr, reinterpret_cast<void*>(create_ep_fb_hook),
        reinterpret_cast<void**>(&create_ep_fb_original), nullptr, nullptr);
    spdlog::info("Hooks initialized");
}
