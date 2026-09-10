#pragma once

#include <memory>
#include <print>

#include <engine/Engine.h>
#include <functional>

//#include "../../../../editor/src/EditorGui.h"

struct guipointer
{
    std::function<bool()> init;
    std::function<void()> shutdown;
};

    //#include <engine/core/Log.h>
namespace eng {

//class Log;
//class FileSystem;
//class Window;
//class Renderer;
//class EditorGui;
//class Input;
//class Resources;
//class Gizmos;
//class Messaging;
//class Scripts;
//class Scene;
//class Collision;

class Subsytem_Manager {
public:

    Subsytem_Manager()
        : m_log{std::make_unique<Log>()}, 
        m_fs{std::make_unique<FileSystem>()},
          m_window{std::make_unique<Window>()}, 
        m_renderer{std::make_unique<Renderer>()},
          m_gui{std::make_unique<guipointer>()},
          m_input{std::make_unique<InputMap>()} 
    {

        m_log->Init("", eng::LogLevel::Info);
        m_fs->Init();
        m_renderer->Init(*m_window);
        m_gui->init();
    }

    ~Subsytem_Manager() {
        // destroy subsystems in reverse order

        

        //m_collision->
       // m_scene->
        //m_scripts->
        //m_msg->
       // m_gizmos->
       // m_resources->
        //m_input->
        m_gui->shutdown();
        m_renderer->Shutdown();
        //m_window
        m_fs->Shutdown();
        m_log->Shutdown();
        
        
    }

 

    Log* getlogger() { return m_log.get(); }
    FileSystem* getfs() { return m_fs.get(); }
    Window* GetWindow() { return m_window.get(); }
    Renderer* getrenderer() { return m_renderer.get(); }
    guipointer* geteditor() { return m_gui.get(); }
    InputMap* getinput() { return m_input.get(); }
 



private:
    friend class Engine;
   
    guipointer gui;
    std::function<bool()> GuiInit;
    std::function<void()> GuiShutdown;
    std::unique_ptr<Log> m_log;
    std::unique_ptr<FileSystem> m_fs;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr <guipointer> m_gui;
    std::unique_ptr<InputMap> m_input;
    //std::unique_ptr<Resources> m_resources;
    //std::unique_ptr<Gizmos> m_gizmos;
    //std::unique_ptr<Messaging> m_msg;
    //std::unique_ptr<Scripts> m_scripts;
    //std::unique_ptr<Scene> m_scene;
    //std::unique_ptr<Collision> m_collision;
};


} // namespace eng