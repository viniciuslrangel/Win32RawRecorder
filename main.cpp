#include <iostream>
#include <string>
#include <string_view>

#include "capture.hpp"

using std::string_literals::operator ""s;

static void helpAndExit() {
  std::cout << "\tHelp: [--title title] [--title @partialTitle] [--hwnd 0xFF12] [--info] [--framerate 60] [--help/-h]" << std::endl;
  exit(1);
}

template<class E, class T, class A>
bool startsWith(const std::basic_string<E, T, A> &s, _In_z_ const E *const prefix) {
  return s.rfind(prefix, 0) == 0;
}

int main(int argc, char *argv[]) {
  unsigned int framerate = 60;
  bool         showInfo  = false;
  HWND         hwnd      = nullptr;
  std::string  title;

  if (argc == 1) {
    helpAndExit();
  }

  for (int i = 1; i < argc; i++) {
    static const auto infoArg  = "--info"s;
    static const auto titleArg = "--title"s;
    static const auto hwndArg  = "--hwnd"s;
    static const auto frArg    = "--framerate"s;
    static const auto helpArg1 = "--help"s;
    static const auto helpArg2 = "-h"s;

    auto arg  = argv[i];
    auto next = argv[i + 1];
    if(i == argc - 1) {
      next = nullptr;
    }

    if (helpArg1 == arg || helpArg2 == arg) {
      helpAndExit();
    } else if (infoArg == arg) {
      showInfo = true;
    } else if (titleArg == arg) {
      title = next;
    } else if (frArg == arg) {
      framerate = std::strtol(next, nullptr, 10);
    } else if (hwndArg == arg) {
      auto               hwndStr = std::string(next);
      unsigned long long value;
      if (hwndStr.rfind("0x", 0) == 0) {
        value = std::stoll(hwndStr.substr(2), nullptr, 16);
      } else {
        value = std::stoll(hwndStr);
      }

      hwnd = reinterpret_cast<HWND>(value);
    }
  }

  if (title.empty() && hwnd == nullptr) {
    helpAndExit();
  }

  if (!title.empty() && (hwnd == nullptr || !IsWindow(hwnd))) {
    hwnd = FindWindowA(nullptr, title.c_str());
    if (hwnd == nullptr && startsWith(title, "@")) {
      struct _Data {
        HWND        *hwnd{};
        std::string title;
      } data;
      data.hwnd = &hwnd;
      data.title = title.substr(1);
      EnumWindows([](HWND hwnd, LPARAM param) -> BOOL {
        auto data = reinterpret_cast<_Data *>(param);

        constexpr auto maxT = 2048;
        char           title[maxT];
        if (GetWindowTextA(hwnd, title, maxT) > 0 && std::string_view(title).find(data->title) != std::string::npos) {
          *data->hwnd = hwnd;
          return false;
        }
        return true;
      }, reinterpret_cast<LPARAM>(&data));
    }
    if (hwnd == nullptr) {
      helpAndExit();
    }
  }

  RECT rect{};
  GetClientRect(hwnd, &rect);

  HDC clientDC = GetDC(hwnd);
  {
    BITMAP  bitmap;
    HGDIOBJ hGDI = GetCurrentObject(clientDC, OBJ_BITMAP);
    if (GetObject(hGDI, sizeof(BITMAP), &bitmap) > 0) {
      rect.right  = bitmap.bmWidth;
      rect.bottom = bitmap.bmHeight;
    }
  }

  if (showInfo) {
    std::cout << rect.right << " " << rect.bottom << std::endl;
    exit(0);
  }

//  capture(rect, clientDC, )
}
