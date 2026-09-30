// =============================================================================
//  Window.cpp - a skeleton. Every function is here with the right signature and
//  an empty body. Window.h is the specification; read it before filling one in.
// =============================================================================

#include <engine/platform/Window.h>
#include <engine/core/Log.h>
#include <SDL3/SDL.h>


namespace eng {

// Opens an operating-system window of the given size, and the object that draws
// into it. If anything fails the object is left INVALID rather than half-built,
// and no exception is thrown - a display that will not open is a problem with
// the machine, not a bug, and the caller should be able to exit tidily.
//Window::Window(const char* title, int width, int height) {
//}

// Closes the window. The renderer has to go first, which is the order the
// members are declared in - see Window.h.
Window::~Window() {
    shutdown();
}


void Window::Init(const BootConfig& config)
{
    const auto width = config.windowWidth;
    const auto height = config.windowHeight;

    SDL_InitSubSystem(SDL_INIT_VIDEO);
    auto win = SDL_CreateWindow(config.windowTitle.c_str(), config.windowWidth, config.windowHeight,
                                SDL_WINDOW_RESIZABLE);

    m_window.reset(win);
    //_sleep(5000);

    auto ren = SDL_CreateRenderer(m_window.get(), nullptr);
    m_renderer.reset(ren);

        if (!SDL_SetRenderVSync(m_renderer.get(), 1)) {
        ENGINE_LOG_WARN(Channels::kPlatform, "vsync is not available: {}", SDL_GetError());
    }

    ENGINE_LOG_INFO(Channels::kPlatform, "window created: {}x{} \"{}\" (drawing with {})", width,
                    height, m_title, SDL_GetRendererName(m_renderer.get()));
}


void Window::shutdown()
{
    if (m_window == nullptr && m_renderer == nullptr && !m_videoInitialised)
        return;
    ENGINE_LOG_INFO(Channels::kPlatform, "window closed");

    
    m_renderer.reset();
    m_window.reset();

    if (m_videoInitialised) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        m_videoInitialised = false;
    }
}

// Did the window actually open? Start-up stops here if it did not.
bool Window::IsValid() const {
    return m_window != nullptr && m_renderer != nullptr;
}

// How wide the window is, in pixels.
int Window::Width() const {
    int w = 0;
    int h = 0;
    if (m_window != nullptr) {
        SDL_GetWindowSize(m_window.get(), &w, &h);
    }
    return w;
}

// How tall the window is, in pixels.
int Window::Height() const {
    int w = 0;
    int h = 0;
    if (m_window != nullptr) {
        SDL_GetWindowSize(m_window.get(), &w, &h);
    }
    return h;
}

// Changes the text in the window's title bar.
void Window::SetTitle(const char* title) {
    if (m_window == nullptr || title == nullptr) {
        return;
    }
    m_title = title;
    SDL_SetWindowTitle(m_window.get(), m_title.c_str());
}

// Fills the whole window with one colour, wiping last frame's picture.
void Window::Clear(unsigned char r, unsigned char g, unsigned char b) {
    if (m_renderer == nullptr) {
        return;
    }
    SDL_SetRenderDrawColor(m_renderer.get(), r, g, b, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(m_renderer.get());
}

// Shows whatever has been drawn since the last Clear.
void Window::Present() {
    if (m_renderer == nullptr) {
        return;
    }
    SDL_RenderPresent(m_renderer.get());
}

// The underlying SDL window, as a plain pointer. Only the editor needs this, to
// attach its interface - which is why it is handed out without naming SDL.
void* Window::NativeWindowHandle() const {
    return m_window.get();
}

// The underlying SDL renderer, handed out for the same reason.
void* Window::NativeRendererHandle() const {
    return m_renderer.get();
}

} // namespace eng
