#include "uhepch.h"
#include "UHE/Utils/PlatfromUtils.h"
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

namespace UHE {
    std::string FileDialogs::OpenFile(const char* filter) {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("zenity --file-selection --title=\"Open File\"", "r"), pclose);
        if (!pipe) return "";
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        if (!result.empty() && result.back() == '\n') result.pop_back();
        return result;
    }

    std::string FileDialogs::SaveFile(const char* filter) {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("zenity --file-selection --save --title=\"Save File\"", "r"), pclose);
        if (!pipe) return "";
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        if (!result.empty() && result.back() == '\n') result.pop_back();
        return result;
    }
}
