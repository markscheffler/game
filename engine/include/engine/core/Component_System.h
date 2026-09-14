#pragma once

#include <memory>
#include <print>

#include <engine/Engine.h>
#include <functional>

namespace eng {



//class Scene;
//class Collision;

class Subsystem_Manager {
public:
    using GuiInit = std::function<bool()>;
    using GuiShutdown = std::function<void()>;

    Subsystem_Manager()
        : m_log{std::make_unique<Log>()}, m_fs{std::make_unique<FileSystem>()},
          m_window{std::make_unique<Window>()}, m_renderer{std::make_unique<Renderer>()},
          m_input{std::make_unique<InputMap>()}, m_resources{std::make_unique<ResourceManager>()},
          m_gizmos{std::make_unique<Gizmos>()}, m_msg{std::make_unique<MessageBus>()},
          m_scripts{std::make_unique<ScriptSystem>()}, m_scene{std::make_unique<Scene>()},
          m_collision{std::make_unique<CollisionSystem>()}
 {

        m_log->Init("", eng::LogLevel::Info);
        m_fs->Init();
        m_renderer->Init(*m_window);
        m_resources->Init();
    }

    ~Subsystem_Manager() {

        m_resources->Shutdown();

        if (m_GuiShutdown)
            m_GuiShutdown();

        m_renderer->Shutdown();
        m_fs->Shutdown();
        m_log->Shutdown();
    }

    Log* GetLogger()                    const { return m_log.get(); }
    FileSystem* Getfs()                 const { return m_fs.get(); }
    Window* GetWindow()                 const { return m_window.get(); }
    Renderer* GetRenderer()             const { return m_renderer.get(); }
    InputMap* GetInput()                const { return m_input.get(); }
    ResourceManager* GetResources()     const { return m_resources.get(); }
    Gizmos* GetGizmo()                  const { return m_gizmos.get(); }
    MessageBus* GetMsgBus()             const { return m_msg.get(); }
    ScriptSystem* GetScriptSys()        const { return m_scripts.get(); }
    Scene* GetScene()                   const { return m_scene.get(); }
    CollisionSystem* GetCollisionSys()  const { return m_collision.get(); }

    void SetGuiHooks(GuiInit init, GuiShutdown shutdown) {
        m_GuiInit = std::move(init);
        m_GuiShutdown = std::move(shutdown);

    }

    bool InitGui() {
        if (!m_GuiInit)
            return true;
        return false;
    }

private:

    //friend class Engine;
   
   
    std::function<bool()> m_GuiInit;
    std::function<void()> m_GuiShutdown;

    std::unique_ptr<Log> m_log;
    std::unique_ptr<FileSystem> m_fs;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<InputMap> m_input;
    std::unique_ptr<ResourceManager> m_resources;
    std::unique_ptr<Gizmos> m_gizmos;
    std::unique_ptr<MessageBus> m_msg;
    std::unique_ptr<ScriptSystem> m_scripts;
    std::unique_ptr<Scene> m_scene;
    std::unique_ptr<CollisionSystem> m_collision;
};


} // namespace eng