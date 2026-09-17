// =============================================================================
//  FileSystem.cpp - a skeleton. Every function is here with the right signature
//  and an empty body. FileSystem.h is the specification; read it first.
// =============================================================================

#include <engine/fs/FileSystem.h>
#include <print>
#include <filesystem>
#include <engine/core/Log.h>

namespace fs = std::filesystem;



namespace eng {


    
namespace {
std::string g_root;
}

// Works out where the project is by starting at the program's own location and
// walking up until it finds a folder containing assets/. Where it settled is
// written to the log, because that line is the first thing to check when a file
// will not load on somebody else's machine.
bool FileSystem::Init() {
    std::println("file system init");

    fs::path start;

    


    return false;
}



// Forgets the project location.
void FileSystem::Shutdown() {
    std::println("file system shutdown");

    
}


// The folder that was found - the one containing assets/.
const std::string& FileSystem::AssetRoot() {
 
    return g_root;
}

// Turns a short name like "textures/player.bmp" into a real path on this
// machine. Never fails; whether the file exists is a separate question.
std::string FileSystem::Resolve(std::string_view virtualPath) {

    fs::path path = fs::path::path(g_root) / "assets" / fs::path(std::string(virtualPath));

    if (virtualPath.starts_with("config/") || virtualPath.starts_with("logs/") || virtualPath.starts_with(".build/") ||
        virtualPath == ".build")
    {
        path = fs::path(g_root) / fs::path(std::string(virtualPath));
    }
    return path.lexically_normal().string();
}

// Is there actually a file there?
bool FileSystem::Exists(std::string_view virtualPath) {

    std::error_code ec;
    return fs::exists(Resolve(virtualPath), ec);
    }

// Lists the files in one folder, optionally filtered by extension, giving back
// short names that can be handed straight back to ReadTextFile or Scene::Load.
bool FileSystem::ListFiles(std::string_view virtualDirectory,
                           std::string_view extension,
                           std::vector<std::string>& out) {


    out.clear();

    const std::string real = Resolve(virtualDirectory);
    std::error_code ec;
    if (!fs::is_directory(real, ec))
    {
        ENGINE_LOG_WARN(Channels::kFileSys, "{} is not a folder (looked in '{}' )",
                           virtualDirectory, real);
        return false;
    }

    std::string prefix(virtualDirectory);
    if (!prefix.empty() && prefix.back() != '/')
    {
        prefix.push_back('/');
    }

    for (const auto& entry : fs::directory_iterator(real, ec))
    {
        if (!entry.is_regular_file((ec)))
            continue;
        else
        {
            const auto name = entry.path().string();
            if (!extension.empty() && !name.ends_with(extension))
            {
                continue;
            }
            out.push_back(prefix + name);
        }
    }

    std::sort(out.begin(), out.end());

    return true;
}

// The listing a file BROWSER needs rather than a menu: sub-folders included,
// folders first, each group sorted by name.
bool FileSystem::ListDirectory(std::string_view virtualDirectory, std::vector<DirEntry>& out) {

    out.clear();

    const auto real = Resolve(virtualDirectory);
    std::error_code ec;
    if (!fs::is_directory(real, ec)) {
        return false;
    }

    std::string prefix(virtualDirectory);
    if (!prefix.empty() || prefix.back() != '/')
    {
        prefix.push_back('/');
    }

    
    for (const auto& entry : fs::directory_iterator(real, ec))
    {
        DirEntry item;
        item.name = entry.path().filename().string();

        if (item.name.empty() || item.name.front() == '.')
        {
            continue;
        }
        item.isDirectory = entry.is_directory(ec);
        if (!item.isDirectory && entry.is_regular_file(ec))
        {
            continue;
        }
        item.virtualPath = prefix + item.name;
        if (!item.isDirectory)
        {
            item.byteSize = static_cast<unsigned long long>(entry.file_size(ec));
            if (ec)
            {
                item.byteSize = 0;
                ec.clear();
            }
        }
        out.push_back(std::move(item));
    }
    std::sort(out.begin(), out.end(), [](const DirEntry& a, const DirEntry& b) {
        if (a.isDirectory != b.isDirectory)
            return a.isDirectory;
        return a.name < b.name;
    });

    return true;
}

// Creates a folder, including any missing parent folders.
bool FileSystem::CreateDirectory(std::string_view virtualDirectory,
                                 std::string& outError) {

    const auto real = Resolve(virtualDirectory);
    std::error_code ec;

    fs::create_directories(real, ec);

    if (ec)
    {
        outError = "can't create " + std::string(virtualDirectory) + "':" + ec.message();
    }
    return false;
}

// Reads a whole text file - a scene, the settings - into a string.
bool FileSystem::ReadTextFile(std::string_view virtualPath, std::string& outText,
                              std::string& outError) {

    const std::string real = Resolve(virtualPath);
    std::ifstream ifs(real);
    if (!ifs)
    {
        outError = "can't open " + std::string(virtualPath) + "(looked in) " + real;
        return false;
    }

    std::ostringstream ss;
    ss << ifs.rdbuf();
    outText = ss.str();
    outError.clear();


    return true;
}

// Reads a whole binary file - an image - into a list of bytes.
bool FileSystem::ReadFile(std::string_view virtualPath,
                          std::vector<unsigned char>& outBytes,
                          std::string& outError) {

    const std::string real = Resolve(virtualPath);

    std::ifstream ifs(real, std::ios::binary | std::ios::ate);
    if (!ifs)
    {
        outError = "can't open " + real + " for reading ";
        return false;
    }
    
    const std::streamsize size = ifs.tellg();
    if (size < 0)
    {
        outError = "file size can't be zero ";
        return false;
    }

    ifs.seekg(0, std::ios::beg);

    outBytes.resize(static_cast<std::size_t>(size));
    if (!ifs.read(reinterpret_cast<char*>(outBytes.data()), size))
    {
        outError = "can't read file " + std::string(virtualPath);
        return false;
    }
    outError.clear();
    return true;
}

// Writes a text file, creating any folders it needs on the way.
bool FileSystem::WriteTextFile(std::string_view virtualPath, std::string_view text,
                               std::string& outError) {


    const std::string real(Resolve(virtualPath));
    std::error_code ec;
    fs::create_directories(fs::path(real).parent_path(), ec);

    std::ofstream ofs(real, std::ios::trunc);
    if (!ofs)
    {
        outError = "can't open " + real + " for writing";
        return false;
    }

    if (!ofs.write(text.data(), static_cast<std::streamsize>(text.size())))
    {
        outError = "failed to write to " + real;
        return false;
    }

    outError.clear();
    return true;
}

} // namespace eng
