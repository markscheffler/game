// =============================================================================
//  EventPump.cpp - a skeleton. Every function is here with the right signature
//  and an empty body. EventPump.h is the specification; read it first.
// =============================================================================

#include <engine/platform/EventPump.h>
#include <engine/tools/GuiHooks.h>
#include <engine/core/log.h>
#include <SDL3/sdl.h>



namespace eng {

// Turns an event kind into a readable name, for the log.
const char* ToString(RawEventKind kind) {

    switch (kind) {
    case eng::RawEventKind::None:
        return "none";

    case eng::RawEventKind::Quit:
        return "Quit";
    case eng::RawEventKind::KeyDown:
        return "key down";
    case eng::RawEventKind::KeyUp:
        return "keyup";
    case eng::RawEventKind::MouseButtonDown:
        return "mouseButtonDown";
    case eng::RawEventKind::MouseButtonUp:
        return "mouseButtonUp";
    case eng::RawEventKind::MouseMove:
        return "mouseMove";
    case eng::RawEventKind::MouseWheel:
        return "mouseWheel";
    case eng::RawEventKind::WindowResized:
        return "windowResized";
    case eng::RawEventKind::WindowFocusGained:
        return "WindowFocusGained";
    case eng::RawEventKind::WindowFocusLost:
        return "WindowFocusLost";
    default:
        return "unknown";
    }
}

// Empties the operating system's event queue into this object's list, once per
// frame. When a tool is attached it gets first refusal on each event, so typing
// in a text box does not also drive the game.
void EventPump::Poll() {

    m_events.clear();
    m_consumed.clear();
    m_quitRequested, m_focusGained, m_focusLost = false;

    if (m_events.capacity() == 0)
    {
        m_events.reserve(64);
        m_consumed.reserve(64);

    }

    const GuiHooks gui = GetGuiHooks();
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        const bool guihandled = gui.ProcessEvent != nullptr && gui.ProcessEvent(&event);
        RawEvent revent;
        bool recongnized = true;

        switch (event.type) {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            revent.kind = RawEventKind::Quit;
            m_quitRequested = true;
            break;

            case SDL_EVENT_KEY_DOWN:
                if (event.key.repeat)
                {
                    recongnized = false;
                    break;
                }
                
                revent.kind = RawEventKind::KeyDown;
                revent.code = static_cast<int>(event.key.scancode);
                break;

            case SDL_EVENT_KEY_UP:
                revent.kind = RawEventKind::KeyUp;
                revent.code = static_cast<int>(event.key.scancode);
                break;


            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                revent.kind = RawEventKind::MouseButtonDown;

                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                break;

            case SDL_EVENT_MOUSE_MOTION:
                break;

            case SDL_EVENT_MOUSE_WHEEL:
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                break;
            case SDL_EVENT_WINDOW_FOCUS_GAINED:
                break;
            case SDL_EVENT_WINDOW_FOCUS_LOST:
                break;
        default:
                recongnized = false;
        }

        if (!recongnized)
            continue;

        bool consumed = false;

        switch (revent.kind) {
        using enum RawEventKind;
        case KeyDown:
        case KeyUp:
            consumed = guihandled && gui.WantsKeyboard != nullptr && gui.WantsKeyboard();
            break;
        case MouseButtonDown:
        case MouseButtonUp:
        case MouseMove:
        case MouseWheel:
            consumed = guihandled && gui.WantsMouse != nullptr && gui.WantsMouse();
            break;

        default:
            consumed = false;
            break;
        }
        m_events.push_back(revent);
        m_consumed.push_back(consumed ? char{1} : char{0});

    }

    float x, y = 0.0f;
    SDL_GetMouseState(&x, &y);
    m_mouseX = x;
    m_mouseY = y;

}

// How many events arrived this frame.
std::size_t EventPump::Count() const {
    return 0;
}

// One event from this frame's list.
const RawEvent& EventPump::At(std::size_t index) const {


    if (index >= m_events.size()) {
        ENGINE_LOG_WARN(Channels::kInput, "event pump at:({}) is past the end of {} events", index,
                        m_events.size());

        static const RawEvent none{};
        return none;
    }
    return m_events[index];
}

// Did the user ask to close the window this frame?
bool EventPump::QuitRequested() const {
    return m_quitRequested;
}

// Was this event already claimed by a tool? Game code checks this before acting
// on it.
bool EventPump::WasConsumed(std::size_t index) const {
    return index < m_consumed.size() && m_consumed[index] != 0;
}

// The readable name of a key code, so the settings file can say "Key.Space"
// rather than a number.
const char* EventPump::KeyName(int code) {

    const char* name = SDL_GetScancodeName(static_cast<SDL_Scancode>(code));
    return (name != nullptr && name[0] != '\0') ? name : "?";
}

// The reverse: turns a name from the settings file into a key code. Returns a
// negative number when the name is not a key.
int EventPump::KeyCodeFromName(const char* /*name*/) {
    return -1;
}

// The same, for mouse buttons: Left, Right or Middle.
int EventPump::MouseButtonFromName(const char* /*name*/) {
    return -1;
}

} // namespace eng
