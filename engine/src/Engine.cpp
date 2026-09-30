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
bool Engine::LoadScene(std::string_view /*virtualPath*/, std::string& /*outError*/) {
    return false;
}

// Writes the current scene back out to a file, including where the camera is.
bool Engine::SaveScene(std::string_view /*virtualPath*/, std::string& /*outError*/) {
    return false;
}

// Takes a snapshot of the scene and starts running it. The snapshot is what
// makes pressing Play safe on a level you have been building.
bool Engine::EnterPlayMode(std::string& /*outError*/) {
    return false;
}

// Stops play mode and puts the snapshot back, undoing everything the running
// game did to the scene.
void Engine::ExitPlayMode() {
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
