// =============================================================================
//  Config.cpp - a skeleton. Every function is here with the right signature and
//  an empty body. Config.h is the specification; read it before filling one in.
// =============================================================================

#include <engine/core/Config.h>
#include <engine/core/Component_System.h>
#include <engine/core/Json.h>
#include <engine/core/Log.h>
namespace eng {

// Reads config/engine.json into a BootConfig - window size, log level, fixed
// timestep, starting scene. Also hands back the parsed document, because the
// input bindings are read out of it later. Returns false if the file is
// missing or malformed, which is the one thing that stops the engine starting.


    namespace
    {

        const Json& section(const Json& doc, const char* name)
        {
        static const Json kEmpty = Json::object();
            if (!doc.is_object())
                return kEmpty;
            const auto it = doc.find(name);
            return (it != doc.end() && it->is_object() ? *it : kEmpty);
        }
    }


bool LoadBootConfig(std::string_view virtualPath, BootConfig& outConfig,
                    Json& outDocument, std::string& outError, Subsystem_Manager& sm) {

    std::string txt, read_err;
    
    if (!sm.Getfs()->ReadTextFile(virtualPath, txt, read_err))
    {
        outError = "no setting file at " + std::string(virtualPath) + "using built in defaults";
        ENGINE_LOG_WARN(Channels::kConfig, "{}", outError);
        return true;
    }

    std::string parseError;
    Json doc = ParseJson(txt, parseError);

    if (!parseError.empty())
    {
        outError = std::string(virtualPath) + ":" + parseError;
        ENGINE_LOG_WARN(Channels::kConfig, "{}", outError);
    }

    
        const Json& window = section(doc, "window");
    outConfig.windowWidth = ReadInt(window, "width", outConfig.windowWidth, "window");
    outConfig.windowHeight = ReadInt(window, "height", outConfig.windowHeight, "window");
    outConfig.windowTitle = ReadString(window, "title", outConfig.windowTitle, "window");

    const Json& log = section(doc, "logging");
    outConfig.logFile = ReadString(log, "logging", outConfig.logFile, "logging");
    const auto threashold = ReadString(log, "threashold", ToString(outConfig.logThreshold), "logging");

    if (!ParseLogLevel(threashold, outConfig.logThreshold))
    {
        ENGINE_LOG_WARN(
            Channels::kConfig,
            "logging threashold is {}, which is not one of Info, Warning, Error; using {}",
            threashold, ToString(outConfig.logThreshold));
    }

    const auto tunables = section(doc, "tunables");

    outConfig.logBufferCapacity = ReadInt(tunables, "logBufferCapacity", outConfig.logBufferCapacity, "tunables");
    outConfig.gizmoCircleSegments = ReadInt(tunables, "gizmoCircleSegments", outConfig.gizmoCircleSegments, "tunables");
    outConfig.fixedTimestepSeconds =
        ReadFloat(tunables, "fixedTimeStepSeconds", outConfig.fixedTimestepSeconds, "tunables");
    outConfig.maxStepsPerFrame =
        ReadInt(tunables, "maxStepsPerFrame", outConfig.maxStepsPerFrame, "tunables");
    outConfig.startupScene = ReadString(doc, "startup", outConfig.startupScene, "startup");

    outDocument = std::move(doc);

    outError.clear();
    ENGINE_LOG_INFO(Channels::kConfig, "settings loaded from {}", virtualPath);


    return true;
}

} // namespace eng
