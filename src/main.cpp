#include "hyperxApp.h"

int main(int argc, char *argv[]) {
  wxApp *pApp = new hyperxApp();
  hyperxApp::SetInstance(pApp);
  wxEntry(argc, argv);
  wxEntryCleanup();
}
