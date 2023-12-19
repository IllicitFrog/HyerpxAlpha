#include <wx/event.h>
#include <wx/icon.h>
#include <wx/taskbar.h>
#include <wx/textctrl.h>
#include "hyperxFrame.h"
#include "hyperxApp.h"
#include "alpha_w.h"

// App
hyperxApp::hyperxApp() {}
hyperxApp::~hyperxApp() {}

bool hyperxApp::OnInit() {
  hyperx_frame *m_frame =
      new hyperx_frame(_T("HyperX"), wxPoint(50, 50), wxSize(400, 600));
  SetTopWindow(m_frame);
  headset *m_headset = new headset;
  return true;
}
// start the application from here
wxIMPLEMENT_APP(hyperxApp);
