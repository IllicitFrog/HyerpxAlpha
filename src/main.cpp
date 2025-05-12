#include "hyperxApp.h"

int main(int argc, char *argv[]) {
  if (argv[1] == NULL || std::string(argv[1]) == "--systray") {
    wxApp *pApp = new hyperxApp(argv[1]);
    hyperxApp::SetInstance(pApp);
    wxEntry(argc, argv);
    wxEntryCleanup();
  } else {
    std::cout << "HyperX Alpha Help" << std::endl;
    std::cout << "  Use --systray to start gui with legacy systray support!"
              << std::endl;
  }
  return 0;
}
