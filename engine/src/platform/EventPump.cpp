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
    m_quitRequested = false;
    m_focusGained = false;
    m_focusLost = false;

    if (m_events.capacity() == 0) {
        m_events.reserve(64);
        m_consumed.reserve(64);
    }

    // The tool layer, if one is attached. In the editor these three point at
    // ImGui; in the standalone game they are all null and every event reaches
    // the game untouched. See tools/GuiHooks.h.
    const GuiHooks& gui = GetGuiHooks();

    SDL_Event sdlEvent;

    // Keep going until SDL_PollEvent reports there is nothing left. Reading
    // only one event per frame would make input fall further and further
    // behind whenever several things happened at once.
    while (SDL_PollEvent(&sdlEvent)) {
        // The editor's interface sees every event first, so that a text box
        // with focus can claim the keyboard.
        const bool guiHandled = (gui.ProcessEvent != nullptr) && gui.ProcessEvent(&sdlEvent);

        RawEvent event;
        bool recognised = true;

        switch (sdlEvent.type) {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            event.kind = RawEventKind::Quit;
            m_quitRequested = true;
            break;

        case SDL_EVENT_KEY_DOWN:
            // The operating system sends repeated KeyDown events while a
            // key is held. Those are thrown away here: how long a key has
            // been held is worked out by InputMap from one frame to the
            // next, which behaves the same on every machine. The OS repeat
            // rate is a personal setting and differs from user to user.
            if (sdlEvent.key.repeat) {
                recognised = false;
                break;
            }
            event.kind = RawEventKind::KeyDown;
            event.code = static_cast<int>(sdlEvent.key.scancode);
            break;

        case SDL_EVENT_KEY_UP:
            event.kind = RawEventKind::KeyUp;
            event.code = static_cast<int>(sdlEvent.key.scancode);
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            event.kind = RawEventKind::MouseButtonDown;
            event.code = static_cast<int>(sdlEvent.button.button);
            event.mouseX = sdlEvent.button.x;
            event.mouseY = sdlEvent.button.y;
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            event.kind = RawEventKind::MouseButtonUp;
            event.code = static_cast<int>(sdlEvent.button.button);
            event.mouseX = sdlEvent.button.x;
            event.mouseY = sdlEvent.button.y;
            break;

        case SDL_EVENT_MOUSE_MOTION:
            event.kind = RawEventKind::MouseMove;
            event.mouseX = sdlEvent.motion.x;
            event.mouseY = sdlEvent.motion.y;
            break;

        case SDL_EVENT_MOUSE_WHEEL:
            event.kind = RawEventKind::MouseWheel;
            event.wheelY = sdlEvent.wheel.y;
            break;

        case SDL_EVENT_WINDOW_RESIZED:
            event.kind = RawEventKind::WindowResized;
            event.mouseX = static_cast<float>(sdlEvent.window.data1); // new width
            event.mouseY = static_cast<float>(sdlEvent.window.data2); // new height
            break;

        // "Keyboard focus" is the operating system's idea of which window
        // the user is typing into - which is what changes when somebody
        // alt-tabs away and back. The editor watches for the "gained" one
        // to notice that scripts may have been edited while it was in the
        // background.
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            event.kind = RawEventKind::WindowFocusGained;
            m_focusGained = true;
            break;

        case SDL_EVENT_WINDOW_FOCUS_LOST:
            event.kind = RawEventKind::WindowFocusLost;
            m_focusLost = true;
            break;

        default:
            // Plenty of SDL event types are of no interest here. Ignoring
            // them keeps the list short and meaningful.
            recognised = false;
            break;
        }

        if (!recognised) {
            continue;
        }

        // Whether the GUI claimed this event is decided per DEVICE: a key
        // press is claimed only when the GUI wants the keyboard, and a click
        // only when it wants the mouse. A Quit is never claimed - the window's
        // close button has to work no matter what has focus.
        bool consumed = false;
        switch (event.kind) {
        case RawEventKind::KeyDown:
        case RawEventKind::KeyUp:
            consumed = guiHandled && gui.WantsKeyboard != nullptr && gui.WantsKeyboard();
            break;
        case RawEventKind::MouseButtonDown:
        case RawEventKind::MouseButtonUp:
        case RawEventKind::MouseMove:
        case RawEventKind::MouseWheel:
            consumed = guiHandled && gui.WantsMouse != nullptr && gui.WantsMouse();
            break;
        default:
            consumed = false;
            break;
        }

        m_events.push_back(event);
        m_consumed.push_back(consumed ? char{1} : char{0});
    }

    // Ask SDL where the cursor is even if it did not move this frame, so that
    // anything reading MouseX/MouseY always gets a current answer.
    float x = 0.0f;
    float y = 0.0f;
    SDL_GetMouseState(&x, &y);
    m_mouseX = x;
    m_mouseY = y;

}

// How many events arrived this frame.
std::size_t EventPump::Count() const {
    return m_events.size();
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
int EventPump::KeyCodeFromName(const char* name) {
    if (name == nullptr) {
        return -1;
    }
    const SDL_Scancode code = SDL_GetScancodeFromName(name);
    return (code == SDL_SCANCODE_UNKNOWN) ? -1 : static_cast<int>(code);
}

// The same, for mouse buttons: Left, Right or Middle.
int EventPump::MouseButtonFromName(const char* name) {
    if (name == nullptr) {
        return -1;
    }
    // SDL_strcasecmp compares ignoring capitalisation, so "left" and "Left"
    // both work in a config file.
    if (SDL_strcasecmp(name, "Left") == 0) {
        return SDL_BUTTON_LEFT;
    }
    if (SDL_strcasecmp(name, "Right") == 0) {
        return SDL_BUTTON_RIGHT;
    }
    if (SDL_strcasecmp(name, "Middle") == 0) {
        return SDL_BUTTON_MIDDLE;
    }
    return -1;
}

} // namespace eng
