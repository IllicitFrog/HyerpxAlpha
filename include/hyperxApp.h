#ifndef HYPERXAPP_H
#define HYPERXAPP_H

#include <wx/wx.h>
#include <wx/taskbar.h>
#include <wx/textctrl.h>
#include "hyperxFrame.h"

class hyperxApp : public wxApp {
public:
  hyperxApp();
  ~hyperxApp();

  virtual bool OnInit();

  hyperx_frame *m_frame;
};

#endif
