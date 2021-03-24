#include <windows.h>
#include <iostream>
#include <chrono>
#include <tchar.h>

struct Size
{
  short width, height;
};

// Global Vars
CHAR_INFO *screenBuffer;
Size screen;
Size fontSize;
std::string appName;
HANDLE hConsole;
HANDLE hConsoleIn;
SMALL_RECT rectWindow;
bool isRunning = false;

// Methods
void error(std::string error)
{
  std::cout << error;
  exit(1);
}
BOOL closeHandler(DWORD event)
{
  // Note this gets called in a seperate OS thread, so it must
  // only exit when the game has finished cleaning up, or else
  // the process will be killed before OnUserDestroy() has finished
  if (event == CTRL_CLOSE_EVENT)
  {
    isRunning = false;
  }
  return true;
}
bool onUserCreate()
{
  return true;
}
bool onUserUpdate(float &elapsedTime)
{
  return true;
}
void cleanup()
{
  delete[] screenBuffer;
}
// Main
int main()
{
  screen = {206, 190};
  fontSize = {4, 4};
  appName = "Default";

  hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hConsole == INVALID_HANDLE_VALUE)
    error("Bad Handle");
  hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);

  COORD coord = {screen.width, screen.height};
  if (!SetConsoleScreenBufferSize(hConsole, coord))
    throw(L"SetConsoleScreenBufferSize");

  if (!SetConsoleActiveScreenBuffer(hConsole))
    throw(L"SetConsoleActiveScreenBuffer");

  // Set the font size now that the screen buffer has been assigned to the console
  CONSOLE_FONT_INFOEX cfi;
  cfi.cbSize = sizeof(cfi);
  cfi.nFont = 0;
  cfi.dwFontSize.X = fontSize.width;
  cfi.dwFontSize.Y = fontSize.height;
  cfi.FontFamily = FF_DONTCARE;
  cfi.FontWeight = FW_NORMAL;

  wcscpy_s(cfi.FaceName, L"Consolas");
  if (!SetCurrentConsoleFontEx(hConsole, false, &cfi))
    error("SetCurrentConsoleFontEx");

  // Get screen buffer info and check the maximum allowed window size. Return
  // error if exceeded, so user knows their dimensions/fontsize are too large
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
    error("GetConsoleScreenBufferInfo");
  if (screen.height > csbi.dwMaximumWindowSize.Y)
    error("Screen Height / Font Height Too Big");
  if (screen.width > csbi.dwMaximumWindowSize.X)
    error("Screen Width / Font Width Too Big");

  // Set Physical Console Window Size
  rectWindow = {0, 0, (short)(screen.width - 1), (short)(screen.height - 1)};
  if (!SetConsoleWindowInfo(hConsole, TRUE, &rectWindow))
    error("SetConsoleWindowInfo2");

  // Set flags to allow mouse input
  if (!SetConsoleMode(hConsoleIn, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT))
    error("SetConsoleMode");

  // Allocate memory for screen buffer
  screenBuffer = new CHAR_INFO[screen.width * screen.height];
  memset(screenBuffer, 0, sizeof(CHAR_INFO) * screen.width * screen.height);

  SetConsoleCtrlHandler((PHANDLER_ROUTINE)closeHandler, TRUE);

  /* Start
  ---------*/
  isRunning = true;

  if (!onUserCreate())
    isRunning = false;

  auto tp1 = std::chrono::system_clock::now();
  auto tp2 = std::chrono::system_clock::now();

  short consoleTitleLenght = appName.length() + 12;
  char consoleTitle[consoleTitleLenght];

  while (isRunning)
  {
    tp2 = std::chrono::system_clock::now();
    std::chrono::duration<float> elapsedTime = tp2 - tp1;
    tp1 = tp2;
    float fElapsedTime = elapsedTime.count();

    if (!onUserUpdate(fElapsedTime))
      isRunning = false;

    // Update Title & Present Screen Buffer
    snprintf(consoleTitle, consoleTitleLenght, "%s - FPS: %d", appName.c_str(), (int)(1 / fElapsedTime));
    SetConsoleTitle(_T(consoleTitle));
    WriteConsoleOutput(hConsole, screenBuffer, {screen.width, screen.height}, {0, 0}, &rectWindow);
  };

  return 0;
}