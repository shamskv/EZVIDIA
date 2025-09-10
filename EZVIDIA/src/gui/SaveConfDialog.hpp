#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class WindowsGui;

class SaveConfDialog {
 private:
  static LRESULT CALLBACK callbackProc(HWND, UINT, WPARAM, LPARAM);
  static LRESULT handleCommand(HWND, WPARAM, WindowsGui*);

 public:
  static INT_PTR show(HINSTANCE, HWND, LPARAM);
};