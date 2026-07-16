#include "PortalFrameGrabber_Linux.h"

#ifdef __linux__

#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <unistd.h>

#include <gio/gio.h>
#include <gio/gunixfdlist.h>
#include <glib.h>

#include <pipewire/pipewire.h>
#include <spa/param/video/format-utils.h>

namespace ucf::utilities::screencapture {

namespace {

// ---------------------------------------------------------------------------
// Restore token persistence (shared semantics with the screen recorder)
// ---------------------------------------------------------------------------

std::string tokenFilePath()
{
    const char* home = std::getenv("HOME");
    if (!home || !*home)
    {
        return {};
    }
    return std::string(home) + "/.cache/portal_screencast_token";
}

std::string loadRestoreToken()
{
    std::string path = tokenFilePath();
    if (path.empty())
    {
        return {};
    }
    std::ifstream ifs(path);
    if (!ifs.is_open())
    {
        return {};
    }
    std::string token;
    std::getline(ifs, token);
    // Trim trailing whitespace/newline
    while (!token.empty() && (token.back() == '\n' || token.back() == '\r' || token.back() == ' '))
    {
        token.pop_back();
    }
    return token;
}

void saveRestoreToken(const std::string& token)
{
    std::string path = tokenFilePath();
    if (path.empty() || token.empty())
    {
        return;
    }
    // Ensure ~/.cache exists
    std::string dir = path.substr(0, path.find_last_of('/'));
    g_mkdir_with_parents(dir.c_str(), 0700);
    std::ofstream ofs(path, std::ios::trunc);
    if (ofs.is_open())
    {
        ofs << token;
    }
}

// ---------------------------------------------------------------------------
// PipeWire single-frame capture
// ---------------------------------------------------------------------------

struct PwGrab
{
    pw_thread_loop* loop = nullptr;
    pw_context* context = nullptr;
    pw_core* core = nullptr;
    pw_stream* stream = nullptr;
    spa_hook streamListener{};

    spa_video_info format{};
    bool haveFormat = false;

    CaptureImage* out = nullptr;
    bool done = false;
    bool failed = false;
};

/// Convert a single frame of the negotiated raw video format into BGRA.
void convertToBgra(PwGrab* g, const uint8_t* src, int srcStride)
{
    const uint32_t width = g->format.info.raw.size.width;
    const uint32_t height = g->format.info.raw.size.height;
    const uint32_t fmt = g->format.info.raw.format;

    if (width == 0 || height == 0)
    {
        g->failed = true;
        return;
    }

    CaptureImage& img = *g->out;
    img.width = static_cast<int>(width);
    img.height = static_cast<int>(height);
    img.bytesPerRow = static_cast<int>(width) * 4;
    img.scaleFactor = 1;
    img.pixels.resize(static_cast<size_t>(img.bytesPerRow) * height);

    const bool swapRB = (fmt == SPA_VIDEO_FORMAT_RGBA || fmt == SPA_VIDEO_FORMAT_RGBx);
    const bool forceOpaque = (fmt == SPA_VIDEO_FORMAT_BGRx || fmt == SPA_VIDEO_FORMAT_RGBx);

    for (uint32_t y = 0; y < height; ++y)
    {
        const uint8_t* srcRow = src + static_cast<size_t>(y) * srcStride;
        uint8_t* dstRow = img.pixels.data() + static_cast<size_t>(y) * img.bytesPerRow;
        for (uint32_t x = 0; x < width; ++x)
        {
            const uint8_t* sp = srcRow + x * 4;
            uint8_t* dp = dstRow + x * 4;
            if (swapRB)
            {
                dp[0] = sp[2]; // B <- R
                dp[1] = sp[1]; // G
                dp[2] = sp[0]; // R <- B
            }
            else
            {
                dp[0] = sp[0]; // B
                dp[1] = sp[1]; // G
                dp[2] = sp[2]; // R
            }
            dp[3] = forceOpaque ? 0xFF : sp[3];
        }
    }
}

void onStreamParamChanged(void* userData, uint32_t id, const spa_pod* param)
{
    auto* g = static_cast<PwGrab*>(userData);
    if (param == nullptr || id != SPA_PARAM_Format)
    {
        return;
    }

    uint32_t mediaType = 0;
    uint32_t mediaSubtype = 0;
    if (spa_format_parse(param, &mediaType, &mediaSubtype) < 0)
    {
        return;
    }
    if (mediaType != SPA_MEDIA_TYPE_video || mediaSubtype != SPA_MEDIA_SUBTYPE_raw)
    {
        return;
    }

    spa_zero(g->format);
    if (spa_format_video_raw_parse(param, &g->format.info.raw) < 0)
    {
        return;
    }
    g->haveFormat = true;
}

void onStreamProcess(void* userData)
{
    auto* g = static_cast<PwGrab*>(userData);
    if (g->done)
    {
        return;
    }

    pw_buffer* b = pw_stream_dequeue_buffer(g->stream);
    if (b == nullptr)
    {
        return;
    }

    spa_buffer* buf = b->buffer;
    if (g->haveFormat && buf->datas[0].data != nullptr)
    {
        int stride = buf->datas[0].chunk->stride;
        if (stride <= 0)
        {
            stride = static_cast<int>(g->format.info.raw.size.width) * 4;
        }
        convertToBgra(g, static_cast<const uint8_t*>(buf->datas[0].data), stride);
        g->done = true;
    }

    pw_stream_queue_buffer(g->stream, b);
    pw_thread_loop_signal(g->loop, false);
}

void onStreamStateChanged(void* userData, enum pw_stream_state /*old*/,
                          enum pw_stream_state state, const char* /*error*/)
{
    auto* g = static_cast<PwGrab*>(userData);
    if (state == PW_STREAM_STATE_ERROR || state == PW_STREAM_STATE_UNCONNECTED)
    {
        if (!g->done)
        {
            g->failed = true;
            pw_thread_loop_signal(g->loop, false);
        }
    }
}

const pw_stream_events kStreamEvents = {
    .version = PW_VERSION_STREAM_EVENTS,
    .state_changed = onStreamStateChanged,
    .param_changed = onStreamParamChanged,
    .process = onStreamProcess,
};

/// Connect to a PipeWire node via the portal-provided fd and copy one frame.
bool capturePipeWireFrame(int pipewireFd, uint32_t nodeId, CaptureImage& out)
{
    pw_init(nullptr, nullptr);

    PwGrab g;
    g.out = &out;

    g.loop = pw_thread_loop_new("ucf-portal-grab", nullptr);
    if (g.loop == nullptr)
    {
        return false;
    }

    g.context = pw_context_new(pw_thread_loop_get_loop(g.loop), nullptr, 0);
    if (g.context == nullptr)
    {
        pw_thread_loop_destroy(g.loop);
        return false;
    }

    pw_thread_loop_lock(g.loop);
    if (pw_thread_loop_start(g.loop) != 0)
    {
        pw_thread_loop_unlock(g.loop);
        pw_context_destroy(g.context);
        pw_thread_loop_destroy(g.loop);
        return false;
    }

    // Take ownership of the fd; PipeWire closes it on core disconnect.
    g.core = pw_context_connect_fd(g.context, fcntl(pipewireFd, F_DUPFD_CLOEXEC, 0), nullptr, 0);
    if (g.core == nullptr)
    {
        pw_thread_loop_unlock(g.loop);
        pw_thread_loop_stop(g.loop);
        pw_context_destroy(g.context);
        pw_thread_loop_destroy(g.loop);
        return false;
    }

    g.stream = pw_stream_new(g.core, "ucf-screenshot",
                             pw_properties_new(PW_KEY_MEDIA_TYPE, "Video",
                                               PW_KEY_MEDIA_CATEGORY, "Capture",
                                               PW_KEY_MEDIA_ROLE, "Screen",
                                               nullptr));
    if (g.stream == nullptr)
    {
        pw_thread_loop_unlock(g.loop);
        pw_thread_loop_stop(g.loop);
        pw_context_destroy(g.context);
        pw_thread_loop_destroy(g.loop);
        return false;
    }

    pw_stream_add_listener(g.stream, &g.streamListener, &kStreamEvents, &g);

    // Advertise the raw BGRA/RGBA formats we can consume.
    uint8_t podBuffer[1024];
    spa_pod_builder pb = SPA_POD_BUILDER_INIT(podBuffer, sizeof(podBuffer));
    const spa_rectangle sizeDefault = SPA_RECTANGLE(1920, 1080);
    const spa_rectangle sizeMin = SPA_RECTANGLE(1, 1);
    const spa_rectangle sizeMax = SPA_RECTANGLE(8192, 8192);
    const spa_fraction rateDefault = SPA_FRACTION(30, 1);
    const spa_fraction rateMin = SPA_FRACTION(0, 1);
    const spa_fraction rateMax = SPA_FRACTION(1000, 1);

    const spa_pod* params[1];
    params[0] = static_cast<const spa_pod*>(spa_pod_builder_add_object(
        &pb,
        SPA_TYPE_OBJECT_Format, SPA_PARAM_EnumFormat,
        SPA_FORMAT_mediaType, SPA_POD_Id(SPA_MEDIA_TYPE_video),
        SPA_FORMAT_mediaSubtype, SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw),
        SPA_FORMAT_VIDEO_format,
        SPA_POD_CHOICE_ENUM_Id(5,
                               SPA_VIDEO_FORMAT_BGRA, SPA_VIDEO_FORMAT_BGRA,
                               SPA_VIDEO_FORMAT_RGBA, SPA_VIDEO_FORMAT_BGRx,
                               SPA_VIDEO_FORMAT_RGBx),
        SPA_FORMAT_VIDEO_size,
        SPA_POD_CHOICE_RANGE_Rectangle(&sizeDefault, &sizeMin, &sizeMax),
        SPA_FORMAT_VIDEO_framerate,
        SPA_POD_CHOICE_RANGE_Fraction(&rateDefault, &rateMin, &rateMax)));

    int connectRc = pw_stream_connect(
        g.stream, PW_DIRECTION_INPUT, nodeId,
        static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT |
                                     PW_STREAM_FLAG_MAP_BUFFERS),
        params, 1);
    if (connectRc != 0)
    {
        pw_thread_loop_unlock(g.loop);
        pw_stream_destroy(g.stream);
        pw_thread_loop_stop(g.loop);
        pw_context_destroy(g.context);
        pw_thread_loop_destroy(g.loop);
        return false;
    }

    // Wait (bounded) for the first frame or an error.
    constexpr int kMaxWaitIters = 200; // ~10s at 50ms slices
    int iters = 0;
    while (!g.done && !g.failed && iters < kMaxWaitIters)
    {
        // timed wait: release lock, sleep on the loop, re-acquire
        struct timespec abstime;
        pw_thread_loop_get_time(g.loop, &abstime, 50 * SPA_NSEC_PER_MSEC);
        pw_thread_loop_timed_wait_full(g.loop, &abstime);
        ++iters;
    }

    bool ok = g.done && out.isValid();

    pw_stream_disconnect(g.stream);
    pw_thread_loop_unlock(g.loop);

    pw_stream_destroy(g.stream);
    pw_core_disconnect(g.core);
    pw_thread_loop_stop(g.loop);
    pw_context_destroy(g.context);
    pw_thread_loop_destroy(g.loop);

    return ok;
}

// ---------------------------------------------------------------------------
// xdg-desktop-portal ScreenCast negotiation (GDBus)
// ---------------------------------------------------------------------------

struct PortalCtx
{
    GDBusConnection* conn = nullptr;
    GMainLoop* loop = nullptr;

    std::string sessionHandle;
    guint32 nodeId = 0;
    int regionX = 0;
    int regionY = 0;
    int pipewireFd = -1;
    bool started = false;

    int requestCounter = 0;
    bool failed = false;
};

std::string uniqueNameToken(GDBusConnection* conn)
{
    const char* unique = g_dbus_connection_get_unique_name(conn); // ":1.234"
    std::string s = (unique && *unique == ':') ? (unique + 1) : (unique ? unique : "");
    for (char& c : s)
    {
        if (c == '.')
        {
            c = '_';
        }
    }
    return s;
}

std::string makeRequestPath(PortalCtx* ctx, const std::string& token)
{
    return "/org/freedesktop/portal/desktop/request/" + uniqueNameToken(ctx->conn) + "/" + token;
}

std::string newRequestToken(PortalCtx* ctx)
{
    return "ucf" + std::to_string(getpid()) + "_" + std::to_string(++ctx->requestCounter);
}

void failPortal(PortalCtx* ctx)
{
    ctx->failed = true;
    if (ctx->loop)
    {
        g_main_loop_quit(ctx->loop);
    }
}

// Forward declarations for the response chain.
void onStartResponse(GDBusConnection*, const gchar*, const gchar*, const gchar*,
                     const gchar*, GVariant* parameters, gpointer userData);
void onSelectSourcesResponse(GDBusConnection*, const gchar*, const gchar*, const gchar*,
                             const gchar*, GVariant* parameters, gpointer userData);
void onCreateSessionResponse(GDBusConnection*, const gchar*, const gchar*, const gchar*,
                             const gchar*, GVariant* parameters, gpointer userData);

guint subscribeResponse(PortalCtx* ctx, const std::string& path,
                        GDBusSignalCallback cb)
{
    return g_dbus_connection_signal_subscribe(
        ctx->conn, "org.freedesktop.portal.Desktop",
        "org.freedesktop.portal.Request", "Response", path.c_str(),
        nullptr, G_DBUS_SIGNAL_FLAGS_NONE,
        cb, ctx, nullptr);
}

void onCreateSessionResponse(GDBusConnection*, const gchar*, const gchar*, const gchar*,
                             const gchar*, GVariant* parameters, gpointer userData)
{
    auto* ctx = static_cast<PortalCtx*>(userData);

    guint32 response = 1;
    GVariant* results = nullptr;
    g_variant_get(parameters, "(u@a{sv})", &response, &results);
    if (response != 0 || results == nullptr)
    {
        if (results) g_variant_unref(results);
        failPortal(ctx);
        return;
    }

    const gchar* sessionHandle = nullptr;
    g_variant_lookup(results, "session_handle", "&s", &sessionHandle);
    if (sessionHandle == nullptr)
    {
        g_variant_unref(results);
        failPortal(ctx);
        return;
    }
    ctx->sessionHandle = sessionHandle;
    g_variant_unref(results);

    // SelectSources
    std::string token = newRequestToken(ctx);
    subscribeResponse(ctx, makeRequestPath(ctx, token), onSelectSourcesResponse);

    GVariantBuilder opts;
    g_variant_builder_init(&opts, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&opts, "{sv}", "handle_token", g_variant_new_string(token.c_str()));
    g_variant_builder_add(&opts, "{sv}", "types", g_variant_new_uint32(1));        // MONITOR
    g_variant_builder_add(&opts, "{sv}", "cursor_mode", g_variant_new_uint32(2));  // embedded
    g_variant_builder_add(&opts, "{sv}", "persist_mode", g_variant_new_uint32(2)); // persistent
    std::string saved = loadRestoreToken();
    if (!saved.empty())
    {
        g_variant_builder_add(&opts, "{sv}", "restore_token",
                              g_variant_new_string(saved.c_str()));
    }

    g_dbus_connection_call(
        ctx->conn, "org.freedesktop.portal.Desktop",
        "/org/freedesktop/portal/desktop", "org.freedesktop.portal.ScreenCast",
        "SelectSources",
        g_variant_new("(oa{sv})", ctx->sessionHandle.c_str(), &opts),
        nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, nullptr, nullptr);
}

void onSelectSourcesResponse(GDBusConnection*, const gchar*, const gchar*, const gchar*,
                             const gchar*, GVariant* parameters, gpointer userData)
{
    auto* ctx = static_cast<PortalCtx*>(userData);

    guint32 response = 1;
    GVariant* results = nullptr;
    g_variant_get(parameters, "(u@a{sv})", &response, &results);
    if (results) g_variant_unref(results);
    if (response != 0)
    {
        failPortal(ctx);
        return;
    }

    // Start
    std::string token = newRequestToken(ctx);
    subscribeResponse(ctx, makeRequestPath(ctx, token), onStartResponse);

    GVariantBuilder opts;
    g_variant_builder_init(&opts, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&opts, "{sv}", "handle_token", g_variant_new_string(token.c_str()));

    g_dbus_connection_call(
        ctx->conn, "org.freedesktop.portal.Desktop",
        "/org/freedesktop/portal/desktop", "org.freedesktop.portal.ScreenCast",
        "Start",
        g_variant_new("(osa{sv})", ctx->sessionHandle.c_str(), "", &opts),
        nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, nullptr, nullptr);
}

void onStartResponse(GDBusConnection*, const gchar*, const gchar*, const gchar*,
                     const gchar*, GVariant* parameters, gpointer userData)
{
    auto* ctx = static_cast<PortalCtx*>(userData);

    guint32 response = 1;
    GVariant* results = nullptr;
    g_variant_get(parameters, "(u@a{sv})", &response, &results);
    if (response != 0 || results == nullptr)
    {
        if (results) g_variant_unref(results);
        failPortal(ctx);
        return;
    }

    // Persist restore_token for silent future grabs.
    const gchar* restoreToken = nullptr;
    if (g_variant_lookup(results, "restore_token", "&s", &restoreToken) && restoreToken)
    {
        saveRestoreToken(restoreToken);
    }

    // streams: a(ua{sv}) — take the first node id and its position.
    GVariant* streams = g_variant_lookup_value(results, "streams", G_VARIANT_TYPE("a(ua{sv})"));
    g_variant_unref(results);
    if (streams == nullptr)
    {
        failPortal(ctx);
        return;
    }

    GVariantIter iter;
    g_variant_iter_init(&iter, streams);
    guint32 nodeId = 0;
    GVariant* props = nullptr;
    if (g_variant_iter_next(&iter, "(u@a{sv})", &nodeId, &props))
    {
        ctx->nodeId = nodeId;
        if (props)
        {
            GVariant* pos = g_variant_lookup_value(props, "position", G_VARIANT_TYPE("(ii)"));
            if (pos)
            {
                gint32 px = 0, py = 0;
                g_variant_get(pos, "(ii)", &px, &py);
                ctx->regionX = px;
                ctx->regionY = py;
                g_variant_unref(pos);
            }
            g_variant_unref(props);
        }
    }
    g_variant_unref(streams);

    if (ctx->nodeId == 0)
    {
        failPortal(ctx);
        return;
    }

    // OpenPipeWireRemote returns the fd directly (not via Request/Response).
    GError* err = nullptr;
    GUnixFDList* fdList = nullptr;
    GVariantBuilder opts;
    g_variant_builder_init(&opts, G_VARIANT_TYPE_VARDICT);
    GVariant* reply = g_dbus_connection_call_with_unix_fd_list_sync(
        ctx->conn, "org.freedesktop.portal.Desktop",
        "/org/freedesktop/portal/desktop", "org.freedesktop.portal.ScreenCast",
        "OpenPipeWireRemote",
        g_variant_new("(oa{sv})", ctx->sessionHandle.c_str(), &opts),
        G_VARIANT_TYPE("(h)"), G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &fdList,
        nullptr, &err);

    if (reply == nullptr || err != nullptr)
    {
        if (err) g_error_free(err);
        if (fdList) g_object_unref(fdList);
        failPortal(ctx);
        return;
    }

    gint32 fdIndex = -1;
    g_variant_get(reply, "(h)", &fdIndex);
    g_variant_unref(reply);

    if (fdList && fdIndex >= 0)
    {
        ctx->pipewireFd = g_unix_fd_list_get(fdList, fdIndex, nullptr);
    }
    if (fdList) g_object_unref(fdList);

    if (ctx->pipewireFd < 0)
    {
        failPortal(ctx);
        return;
    }

    ctx->started = true;
    g_main_loop_quit(ctx->loop);
}

} // namespace

// ---------------------------------------------------------------------------
// Public entry points
// ---------------------------------------------------------------------------

bool portalHasStoredToken()
{
    return !loadRestoreToken().empty();
}

PortalFrame grabPortalFrameBGRA()
{
    PortalFrame frame;

    PortalCtx ctx;
    GError* err = nullptr;
    ctx.conn = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &err);
    if (ctx.conn == nullptr)
    {
        if (err) g_error_free(err);
        return frame;
    }

    ctx.loop = g_main_loop_new(nullptr, FALSE);

    // Kick off CreateSession.
    std::string token = newRequestToken(&ctx);
    std::string sessionToken = "ucf" + std::to_string(getpid()) + "_session";
    subscribeResponse(&ctx, makeRequestPath(&ctx, token), onCreateSessionResponse);

    GVariantBuilder opts;
    g_variant_builder_init(&opts, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&opts, "{sv}", "handle_token", g_variant_new_string(token.c_str()));
    g_variant_builder_add(&opts, "{sv}", "session_handle_token",
                          g_variant_new_string(sessionToken.c_str()));

    g_dbus_connection_call(
        ctx.conn, "org.freedesktop.portal.Desktop",
        "/org/freedesktop/portal/desktop", "org.freedesktop.portal.ScreenCast",
        "CreateSession", g_variant_new("(a{sv})", &opts), nullptr,
        G_DBUS_CALL_FLAGS_NONE, -1, nullptr, nullptr, nullptr);

    g_main_loop_run(ctx.loop);

    if (ctx.started && !ctx.failed && ctx.pipewireFd >= 0)
    {
        if (capturePipeWireFrame(ctx.pipewireFd, ctx.nodeId, frame.image))
        {
            frame.regionX = ctx.regionX;
            frame.regionY = ctx.regionY;
        }
        close(ctx.pipewireFd);
    }

    // Close the portal session if we created one.
    if (!ctx.sessionHandle.empty())
    {
        g_dbus_connection_call(
            ctx.conn, "org.freedesktop.portal.Desktop", ctx.sessionHandle.c_str(),
            "org.freedesktop.portal.Session", "Close",
            nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, nullptr, nullptr);
    }

    g_main_loop_unref(ctx.loop);
    g_object_unref(ctx.conn);

    return frame;
}

} // namespace ucf::utilities::screencapture

#endif // __linux__
