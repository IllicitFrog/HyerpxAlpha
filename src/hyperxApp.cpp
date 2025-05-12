#include "hyperxApp.h"
#include "hyperxFrame.h"
#include <sys/param.h>

// App
hyperxApp::hyperxApp(bool systray) : systray(systray) {}
hyperxApp::~hyperxApp() {}

bool hyperxApp::OnInit() {
  wxImage::AddHandler(new wxPNGHandler);
  char resolved_path[256];
  realpath(argv[0], resolved_path);
  wxString c(resolved_path);
  c.erase(c.end() - 6, c.end());
  try {
    hyperxFrame *m_frame = new hyperxFrame(
        _T("HyperX Alpha"), wxDefaultPosition, wxSize(200, 400), c, this, systray);
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  SetTopWindow(m_frame);
  return true;
}

// start the application from here
// wxIMPLEMENT_APP(hyperxApp);
