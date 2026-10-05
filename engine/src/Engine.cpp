// =============================================================================
//  Engine.cpp - a skeleton. Every function is here with the right signature and
//  an empty body. Engine.h is the specification; read it before filling one in.
// =============================================================================

#include <engine/Engine.h>
#include <print>
#include <SDL3/SDL.h>
namespace eng {

// Returns the one and only engine. Created the first time it is asked for, so
// it is guaranteed to exist before anything tries to use it.
//static Engine instance;

Engine& Engine::Get() {
    static Engine instance;
    return instance;
}

// Hands back the game window, so the editor can attach its interface to it.
//Window& Engine::GetWindow() {
//    return *sm.GetWindow();
//    //return *m_window;
//}

void Engine::EditorGuiHooks(std::function<bool()> init, std::function<void()> shutdown) {
  
  sm.SetGuiHooks(std::move(init), std::move(shutdown));
}

// Builds the ordered list of subsystems. Registration order IS dependency
// order, and shutdown runs it in reverse: Log, FileSystem, Window, Renderer,
// EditorGui, Input, Resources, Gizmos, Messaging, Scripts, Scene, Collision.
void Engine::RegisterBuiltinSubsystems(const Options& ) {

    /*subsystems_manager.add<SubsystemId::LOGGER, eng::Log>()
        .add<SubsystemId::FILESYSTEM, eng::FileSystem>()
        .add<SubsystemId::WINDOW, eng::Window>()
        .add<SubsystemId::RENDERER, eng::Renderer>();

    subsystems_manager.start();*/



}

// Starts everything: reads the settings file, brings the subsystems up in
// order, sets the clock, and loads the starting scene. Returns false if the
// engine cannot run at all.
bool Engine::Init(const Options& options) {
    sm.m_started = true;
    bool inited = false;
    // The file system first: the settings live in a file, and Log::Init needs
    // those settings. Without this, Log::Init was handed a BootConfig that had
    // never been read, so it could only ever see the defaults.
    sm.Getfs()->Init(this->Config());

    std::string configError;
    
    if (!LoadBootConfig(options.configPath, m_config, m_configDocument, configError))
    {
        std::println("{}", configError.c_str());
    }

    sm.GetLogger()->Init(this->Config());

    // The window has to be OPEN before the renderer can draw into it. The
    // manager starts out holding an empty Window, and a Window only opens in
    // its constructor, so it is replaced here with one built from the settings.
    // (Engine can reach m_window because Subsystem_Manager names it a friend.)
    //sm.m_window = std::make_unique<Window>(m_config.windowTitle.c_str(),
    //                                       m_config.windowWidth, m_config.windowHeight);

    sm.GetWindow()->Init(m_config);
    sm.GetRenderer()->Init(*sm.GetWindow());

    // The editor's interface needs the window and renderer, and comes down
    // before them - which is why Shutdown stops it before the renderer.
    EditorGuiHooks(options.guiInit, options.guiShutdown);
    if (!sm.InitGui())
    {
        inited = false;
    }
    
    sm.GetResources()->Init();
    sm.GetGizmo()->init(this->Config());

    m_clock.Init();
    m_clock.SetFixedStepSeconds(m_config.fixedTimestepSeconds);
    m_clock.SetMaxStepsPerFrame(m_config.maxStepsPerFrame);

    const std::string scene =
        options.sceneOverride.empty() ? m_config.startupScene : options.sceneOverride;

    std::string err;
    LoadScene(scene, err);

    m_lastFrameTicks = static_cast<double>(SDL_GetPerformanceCounter());


    inited = true;
    return inited;
}

// Stops everything, in the exact reverse of the order it was started in.
void Engine::Shutdown() {
    sm.Shutdown();
    SDL_Quit();
}

// Replaces the current scene with the one in the named file, and moves the
// camera to wherever that file says it should be.
bool Engine::LoadScene(std::string_view virtualPath, std::string& outError) {
    if (subsystems().GetScene() == nullptr) {
        outError = "the scene subsystem is not running";
        return false;
    }
    if (!subsystems().GetScene()->Load(virtualPath, outError)) {
        return false;
    }
    m_camera.SetPosition(subsystems().GetScene()->InitialCameraPosition());
    m_camera.SetZoom(subsystems().GetScene()->InitialCameraZoom());
    return true;
}

// Writes the current scene back out to a file, including where the camera is.
bool Engine::SaveScene(std::string_view virtualPath, std::string& outError) {
    if (subsystems().GetScene() == nullptr) {
        outError = "the scene subsystem is not running";
        return false;
    }

    const std::string target =
        virtualPath.empty() ? subsystems().GetScene()->SourcePath() : std::string(virtualPath);
    if (target.empty()) {
        outError = "this scene has never been saved anywhere; use Save Scene As";
        return false;
    }

    // The live camera goes in FIRST, so that framing a shot in the editor and
    // pressing save keeps the framing. Doing it here rather than inside
    // Scene::Save means the scene does not have to know a camera exists.
    subsystems().GetScene()->SetCameraState(m_camera.Position(), m_camera.Zoom());

    return subsystems().GetScene()->Save(target, outError);
}

// Takes a snapshot of the scene and starts running it. The snapshot is what
// makes pressing Play safe on a level you have been building.
bool Engine::EnterPlayMode(std::string& outError) {
    if (m_inPlayMode || subsystems().GetScene() == nullptr) {
        return m_inPlayMode;
    }
    if (!subsystems().GetScene()->SaveToString(m_playModeSnapshot, outError)) {
        // Refuse rather than play unsafely. Entering play mode without a
        // snapshot means Stop cannot put the scene back, and silently turning
        // a safe action into a destructive one is the worst possible failure
        // for this feature.
        ENGINE_LOG_ERROR(Channels::kEditor,
                         "cannot enter play mode, because the scene could not be "
                         "snapshotted: {}",
                         outError);
        return false;
    }
    m_inPlayMode = true;
    m_clock.SetPaused(false);
    ENGINE_LOG_INFO(Channels::kEditor, "play mode started");
    return true;
}

// Stops play mode and puts the snapshot back, undoing everything the running
// game did to the scene.
void Engine::ExitPlayMode() {
    if (!m_inPlayMode) {
        return;
    }
    m_inPlayMode = false;
    m_clock.SetPaused(true);

    // Anything still queued belongs to the play session and must not be
    // applied to the restored scene - a destroy queued on the last frame of
    // play would otherwise delete an entity in the freshly restored one.
    DeferredOps::Clear();
    MessageBus::Clear();

    if (subsystems().GetScene() != nullptr && !m_playModeSnapshot.empty()) {
        std::string error;
        if (!subsystems().GetScene()->LoadFromString(m_playModeSnapshot, error)) {
            ENGINE_LOG_ERROR(Channels::kEditor,
                             "play mode ended but the scene could not be restored: {}", error);
        } else {
            ENGINE_LOG_INFO(Channels::kEditor, "play mode stopped; scene restored");
        }
        m_camera.SetPosition(subsystems().GetScene()->InitialCameraPosition());
        m_camera.SetZoom(subsystems().GetScene()->InitialCameraZoom());
    }
    m_playModeSnapshot.clear();
}

// Starts one frame: measures real time, reads input, and works out how many
// fixed simulation steps this frame owes. Returns false when it is time to quit.
bool Engine::BeginFrame() {
    
    const double now = static_cast<double>(SDL_GetPerformanceCounter());
    const double freq = static_cast<double>(SDL_GetPerformanceFrequency());

    double delta = (now - m_lastFrameTicks) / freq;
    m_lastFrameTicks = now;

    delta = std::min(delta, 0.25);
    ResourceManager::PruneCache();
    m_events.Poll();
    InputMap::Update(m_events);

    if (m_events.QuitRequested())
    {
        m_quitRequested = true;
    }

    m_camera.SetViewportSize(sm.GetRenderer()->OutputSize());
    m_stepsThisFrame - m_clock.BeginFrame(delta);
    return !m_quitRequested;
}

// Runs the simulation steps this frame owes, in system order: gameplay,
// movement, collision, messages, create/destroy, camera.
void Engine::Simulate() {
}

// Draws the world through any camera into whatever is currently being drawn
// into. The editor calls this twice - once per view.
void Engine::RenderWorld(Camera& /*camera*/, bool /*includeGizmos*/) {
}

// Draws one frame for the standalone game, gizmos included.
void Engine::RenderFrame() {
}

// Shows the frame that was just drawn.
void Engine::PresentFrame() {
    Renderer::Present();
}

// The standalone game's whole loop: begin, simulate, render, present, repeat.
void Engine::Run() {
    while (BeginFrame())
    {
        Simulate();
        RenderFrame();
        PresentFrame();
       
        
    }
}

} // namespace eng
