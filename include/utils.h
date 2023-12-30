#ifndef _NIU_UTILS_H_
#define _NIU_UTILS_H_

#if __cplusplus >= 201703L
#include <filesystem>
#else
#include <sys/types.h>
#include <sys/stat.h>

#include <cstdlib>
#endif  // C++17+

#include <algorithm>
#include <functional>
#include <string>

namespace niu {
namespace utils {
inline bool
_starts_with(
        std::string const& str,
        std::string const& value)
{
#if __cplusplus >= 202002L
    return str.starts_with(value);
#else
    return str.compare(0, value.size(), value) == 0;
#endif  // C++20+
}

inline bool
_ends_with(
        std::string const& str,
        std::string const& value)
{
#if __cplusplus >= 202002L
    return str.ends_with(value);
#else
    return str.size() >= value.size()
            && 0 == str.compare(str.size() - value.size(), value.size(), value);
#endif  // C++20+
}

inline std::string
_directory_name(
        std::string const& path)
{
#if __cplusplus >= 201703L
    return std::filesystem::path(path.c_str()).parent_path().string();
#else
    auto pos = path.find_last_of("/\\");
    if (pos != std::string::npos) {
        return path.substr(0, pos);
    }
    return std::string();
#endif  // C++17+
}

inline std::string
_file_name(
        std::string const& path)
{
#if __cplusplus >= 201703L
    return std::filesystem::path(path.c_str()).filename().string();
#else
    return path.substr(path.find_last_of("/\\") + 1);
#endif  // C++17+
}

inline bool
_is_directory_exists(
        std::string const& path)
{
#if __cplusplus >= 201703L
    std::filesystem::path p(path.c_str());
    return std::filesystem::is_directory(p);
#else
    struct stat info;
    return stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR);
#endif  // C++17+
}

inline bool
_is_file_exists(
        std::string const& path)
{
#if __cplusplus >= 201703L
    std::filesystem::path p(path.c_str());
    return std::filesystem::exists(p) && !std::filesystem::is_directory(p);
#else
    struct stat info;
    return stat(path.c_str(), &info) == 0 && !(info.st_mode & S_IFDIR);
#endif  // C++17+
}

inline bool
_make_directory(
        std::string const& path)
{
#if __cplusplus >= 201703L
    std::filesystem::path dir(path.c_str());
    return std::filesystem::create_directory(dir);
#else
    std::string command = "mkdir -p " + path;
    return system(command.c_str()) == 0;
#endif  // C++17+
}

inline std::string
_replace(
        std::string str,
        char old,
        std::string const& value)
{
    auto pos = str.find(old);
    while (pos != std::string::npos) {
        str.replace(pos, 1, value);
        pos = str.find(old, pos + value.size());
    }
    return str;
}

inline std::string
_to_upper(
        std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(),
                   [] (unsigned char c)
    { return static_cast<char>(std::toupper(c)); });
    return str;
}

inline std::string
_to_lower(
        std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(),
                   [] (unsigned char c)
    { return static_cast<char>(std::tolower(c)); });
    return str;
}
}  // namespace utils
}  // namespace niu

#endif  // _NIU_UTILS_H_
