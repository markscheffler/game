// =============================================================================
//  InputMap.cpp - a skeleton. Every function is here with the right signature
//  and an empty body. InputMap.h is the specification; read it first.
//
//  Game code asks "does the player want to move up?", never "was W pressed?".
//  This file is the only place the two are connected.
// =============================================================================

#include <engine/input/InputMap.h>
#include <engine/core/Log.h>
#include <engine/platform/EventPump.h>

namespace eng {

    namespace {

// Which kind of thing a binding refers to.
enum class Device { None, Key, MouseButton };

struct Binding {
    Device device = Device::None;
    int code = 0; // the key number or the mouse button number
};

struct ActionEntry {
    // One action can have several bindings, which is how "MoveLeft" ends up
    // on both A and the left arrow.
    std::vector<Binding> bindings;

    ActionState state = ActionState::Idle;

    // Pressed/Held/Released are worked out by comparing these two. Keeping
    // last frame's value is the whole trick, and it is why the engine does not
    // depend on the operating system's key-repeat rate - which is a personal
    // setting that differs from machine to machine.
    bool downNow = false;
    bool downLast = false;
};

struct Context {
    std::map<std::string, ActionEntry> actions;
};

std::map<std::string, Context> g_contexts;
std::vector<std::string> g_stack;

// Finds the action that owns a physical key, searching from the top of the
// stack downwards and stopping at the first match. This function IS the
// shadowing rule described in the header.
ActionEntry* FindOwningAction(Device device, int code) {
    // rbegin/rend walk the vector backwards, i.e. from the top of the stack.
    for (auto it = g_stack.rbegin(); it != g_stack.rend(); ++it) {
        const auto contextIt = g_contexts.find(*it);
        if (contextIt == g_contexts.end()) {
            continue;
        }
        for (auto& [name, action] : contextIt->second.actions) {
            for (const Binding& binding : action.bindings) {
                if (binding.device == device && binding.code == code) {
                    return &action;
                }
            }
        }
    }
    return nullptr;
}

// Finds an action by name, again from the top of the stack down.
ActionEntry* FindAction(std::string_view action) {
    const std::string key(action);
    for (auto it = g_stack.rbegin(); it != g_stack.rend(); ++it) {
        const auto contextIt = g_contexts.find(*it);
        if (contextIt == g_contexts.end()) {
            continue;
        }
        const auto actionIt = contextIt->second.actions.find(key);
        if (actionIt != contextIt->second.actions.end()) {
            return &actionIt->second;
        }
    }
    return nullptr;
}

// Turns "Key.Space" or "Mouse.Left" into a Binding.
Binding ParseBinding(std::string_view text, std::string& outWarning) {
    Binding binding;

    const std::size_t dot = text.find('.');
    if (dot == std::string_view::npos) {
        outWarning = "binding '" + std::string(text) +
                     "' is missing its device prefix (expected Key. or Mouse.)";
        return binding;
    }

    const std::string_view device = text.substr(0, dot);
    const std::string name(text.substr(dot + 1));

    if (device == "Key") {
        const int code = EventPump::KeyCodeFromName(name.c_str());
        if (code < 0) {
            outWarning = "there is no key called '" + name + "'";
            return binding;
        }
        binding.device = Device::Key;
        binding.code = code;
        return binding;
    }

    if (device == "Mouse") {
        const int code = EventPump::MouseButtonFromName(name.c_str());
        if (code < 0) {
            outWarning =
                "there is no mouse button called '" + name + "' (try Left, Right or Middle)";
            return binding;
        }
        binding.device = Device::MouseButton;
        binding.code = code;
        return binding;
    }

    outWarning = "unknown device '" + std::string(device) + "' in a binding";
    return binding;
}

} // namespace

// Turns an action's state into a readable name, for the log.
const char* ToString(ActionState state) {
    switch (state) {
    case ActionState::Idle:
        return "Idle";
    case ActionState::Pressed:
        return "Pressed";
    case ActionState::Held:
        return "Held";
    case ActionState::Released:
        return "Released";
    }
    return "?";
}

// Pushes a set of bindings on top of the stack - a pause menu opened over the
// game. A key is looked up from the top down, and the first context that binds
// it wins, so the menu can take Escape while W still reaches the game.
void InputMap::PushContext(std::string_view context) {
    g_stack.emplace_back(context);
}

// Removes the top set of bindings, going back to whatever was underneath.
void InputMap::PopContext() {
    if (g_stack.empty()) {
        ENGINE_LOG_WARN(Channels::kInput, "PopContext called when no context is active");
        return;
    }
    g_stack.pop_back();
}

// Empties the stack entirely.
void InputMap::ClearContexts() {
    g_stack.clear();
}

// The name of the set of bindings currently on top.
std::string InputMap::ActiveContext() {
    return g_stack.empty() ? std::string{} : g_stack.back();
}

// How many sets of bindings are stacked up.
std::size_t InputMap::ContextDepth() {
    return g_stack.size();
}

// Did this action go down THIS step? True for one step only - the right question
// for a jump.
bool InputMap::IsPressed(std::string_view action) {
    const ActionEntry* entry = FindAction(action);
    return entry != nullptr && entry->state == ActionState::Pressed;
}

// Has this action been held since before this step?
bool InputMap::IsHeld(std::string_view action) {
    const ActionEntry* entry = FindAction(action);
    return entry != nullptr && entry->state == ActionState::Held;
}

// Did this action come up THIS step?
bool InputMap::IsReleased(std::string_view action) {
    const ActionEntry* entry = FindAction(action);
    return entry != nullptr && entry->state == ActionState::Released;
}

// Is this action down at all, whether it started this step or earlier? The right
// question for walking.
bool InputMap::IsDown(std::string_view action) {
    const ActionEntry* entry = FindAction(action);
    return entry != nullptr &&
           (entry->state == ActionState::Pressed || entry->state == ActionState::Held);
}

// The full state of an action, for code that needs to tell the four apart.
ActionState InputMap::GetState(std::string_view action) {
    const ActionEntry* entry = FindAction(action);
    return (entry != nullptr) ? entry->state : ActionState::Idle;
}

// An action as a number: 1 when it is down, 0 when it is not.
float InputMap::GetAxis(std::string_view action) {
    return IsDown(action) ? 1.0f : 0.0f;
}

// Four actions as one direction, already normalised - so holding two keys does
// not move a character diagonally about 40% faster than one key does.
Vec2 InputMap::GetAxis2D(std::string_view negX, std::string_view posX,
                         std::string_view negY, std::string_view posY) {

    const Vec2 raw{GetAxis(posX) - GetAxis(negX), GetAxis(posY) - GetAxis(negY)};
    return raw.Normalized();
}

// Reads this frame's raw events and works out what every action is now doing.
// This is the function that makes all the questions above start answering.
void InputMap::Update(const EventPump& pump) {
    // Step 1: this frame's "down" becomes last frame's.
    for (auto& [contextName, context] : g_contexts) {
        for (auto& [actionName, action] : context.actions) {
            action.downLast = action.downNow;
        }
    }

    // Step 2: apply this frame's events.
    for (std::size_t i = 0; i < pump.Count(); ++i) {
        // Anything the editor's GUI claimed never reaches the game.
        if (pump.WasConsumed(i)) {
            continue;
        }

        const RawEvent& event = pump.At(i);
        ActionEntry* entry = nullptr;

        switch (event.kind) {
        case RawEventKind::KeyDown:
            entry = FindOwningAction(Device::Key, event.code);
            if (entry != nullptr) {
                entry->downNow = true;
            }
            break;
        case RawEventKind::KeyUp:
            entry = FindOwningAction(Device::Key, event.code);
            if (entry != nullptr) {
                entry->downNow = false;
            }
            break;
        case RawEventKind::MouseButtonDown:
            entry = FindOwningAction(Device::MouseButton, event.code);
            if (entry != nullptr) {
                entry->downNow = true;
            }
            break;
        case RawEventKind::MouseButtonUp:
            entry = FindOwningAction(Device::MouseButton, event.code);
            if (entry != nullptr) {
                entry->downNow = false;
            }
            break;
        default:
            break;
        }
    }

    // Step 3: turn the two booleans into a state.
    for (auto& [contextName, context] : g_contexts) {
        for (auto& [actionName, action] : context.actions) {
            if (action.downNow && !action.downLast) {
                action.state = ActionState::Pressed;
            } else if (action.downNow) {
                action.state = ActionState::Held;
            } else if (action.downLast) {
                action.state = ActionState::Released;
            } else {
                action.state = ActionState::Idle;
            }
        }
    }
}

// Binds one key to one action inside one context, from code rather than a file.
void InputMap::Bind(std::string_view context, std::string_view action,
                    std::string_view binding) {
    std::string warning;
    const Binding parsed = ParseBinding(binding, warning);
    if (!warning.empty()) {
        ENGINE_LOG_WARN(Channels::kInput, "{}", warning);
        return;
    }
    if (parsed.device == Device::None) {
        return;
    }
    // operator[] on a std::map creates the entry if it is not there yet, which
    // is exactly what is wanted for "add a binding to this action".
    g_contexts[std::string(context)].actions[std::string(action)].bindings.push_back(parsed);
}

// Reads the "input" section of the settings file, so the controls can be
// rebound without recompiling anything. Bad entries are reported in
// outWarnings rather than silently ignored.
void InputMap::LoadBindings(const Json& inputSection, std::string& outWarnings) {
    if (!inputSection.is_object()) {
        ENGINE_LOG_WARN(Channels::kInput,
                        "the settings file has no \"input\" section, so nothing is bound");
        return;
    }

    const auto contextsIt = inputSection.find("contexts");
    if (contextsIt == inputSection.end() || !contextsIt->is_object()) {
        outWarnings += "input section has no \"contexts\"\n";
        return;
    }

    // items() walks a JSON object as name/value pairs, which is how the
    // context names and action names are discovered rather than hardcoded.
    for (const auto& [contextName, actions] : contextsIt->items()) {
        if (!actions.is_object()) {
            outWarnings += "input.contexts." + contextName + " should be a list of actions\n";
            continue;
        }

        Context& context = g_contexts[contextName];

        for (const auto& [actionName, bindings] : actions.items()) {
            ActionEntry& entry = context.actions[actionName];

            if (!bindings.is_array()) {
                outWarnings += "input.contexts." + contextName + "." + actionName +
                               " should be a list like [\"Key.A\"]\n";
                continue;
            }

            for (const Json& item : bindings) {
                if (!item.is_string()) {
                    continue;
                }
                std::string warning;
                const Binding parsed = ParseBinding(item.get<std::string>(), warning);
                if (!warning.empty()) {
                    // Named with the context and the action, so the message
                    // says which line of the file to go and look at.
                    const std::string full = contextName + "." + actionName + ": " + warning;
                    ENGINE_LOG_WARN(Channels::kInput, "{}", full);
                    outWarnings += full + "\n";
                    continue;
                }
                if (parsed.device != Device::None) {
                    entry.bindings.push_back(parsed);
                }
            }
        }

        ENGINE_LOG_INFO(Channels::kInput, "input context '{}': {} action(s)", contextName,
                        context.actions.size());
    }
}

// Forgets every binding.
void InputMap::ClearBindings() {
    g_contexts.clear();
    g_stack.clear();
}

// Pretends an action was pressed or released. This is what lets an automatic
// playthrough drive the game through the same path a keyboard does, rather than
// going round it.
void InputMap::InjectAction(std::string_view action, bool down) {
    if (ActionEntry* entry = FindAction(action); entry != nullptr) {
        entry->downNow = down;
    }
}

// Stops pretending, handing control back to the real keyboard.
void InputMap::ClearInjectedActions() {
    for (auto& [contextName, context] : g_contexts) {
        for (auto& [actionName, entry] : context.actions) {
            entry.downNow = false;
        }
    }
}

// Lists every binding, for anything that wants to show the current controls.
void InputMap::Snapshot(std::vector<BindingInfo>& out) {

     out.clear();
    for (const auto& [contextName, context] : g_contexts) {
        for (const auto& [actionName, action] : context.actions) {
            if (action.bindings.empty()) {
                out.push_back({contextName, actionName, "<not bound>"});
                continue;
            }
            for (const Binding& binding : action.bindings) {
                std::string text;
                switch (binding.device) {
                case Device::Key:
                    text = std::string("Key.") + EventPump::KeyName(binding.code);
                    break;
                case Device::MouseButton:
                    text = "Mouse." + std::to_string(binding.code);
                    break;
                case Device::None:
                    text = "<none>";
                    break;
                }
                out.push_back({contextName, actionName, text});
            }
        }
    }
}

} // namespace eng
