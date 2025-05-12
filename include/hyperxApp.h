#ifndef HYPERXAPP_H
#define HYPERXAPP_H

#include "hyperxFrame.h"
#include <wx/wx.h>

class hyperxApp : public wxApp {
public:
  hyperxApp(bool);
  ~hyperxApp();

  virtual bool OnInit();

private:
  hyperxFrame *m_frame;
  bool systray;
};

#endif
