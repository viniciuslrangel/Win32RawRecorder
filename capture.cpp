#include "capture.hpp"

#include <iostream>

static volatile bool running = true;

BOOL WINAPI ConsoleCtrlHandler(_In_ DWORD dwCtrlType) {
  if (dwCtrlType == CTRL_C_EVENT) {
    running = false;
    return true;
  }
  return false;
}

void capture(HWND hwnd, HDC clientDC, POINT size, unsigned int framerate) {
  SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
  double interval = 1000.0 / framerate;

  HDC hdc = CreateCompatibleDC(clientDC);
  HBITMAP bitmap = CreateCompatibleBitmap(clientDC, size.x, size.y);
  SelectObject(hdc, bitmap);

  auto *hdcData = new RGBQUAD[size.x * size.y];
  auto dataSize = sizeof(RGBQUAD) * size.x * size.y;

  BITMAPINFO bmi{};
  bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
  bmi.bmiHeader.biWidth = size.x;
  bmi.bmiHeader.biHeight = size.y;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  LARGE_INTEGER start, end, freq;
  QueryPerformanceFrequency(&freq);
  while (running && IsWindow(hwnd)) {
    QueryPerformanceCounter(&start);

    PrintWindow(hwnd, hdc, PW_RENDERFULLCONTENT);
    GetDIBits(
        hdc,
        bitmap,
        0,
        size.y,
        hdcData,
        &bmi,
        DIB_RGB_COLORS
    );

    std::cout.write(reinterpret_cast<const char *>(hdcData), dataSize);

    QueryPerformanceCounter(&end);
    double delta = interval - (static_cast<double>(end.QuadPart - start.QuadPart) * 1000.0) / freq.QuadPart;
    if(delta > 0) {
      Sleep(static_cast<DWORD>(delta));
    } else {
      std::cerr << "Slow performance" << std::endl;
    }
  }

  std::cout.flush();

  ReleaseDC(hwnd, clientDC);
  DeleteDC(hdc);
  DeleteObject(bitmap);
}
