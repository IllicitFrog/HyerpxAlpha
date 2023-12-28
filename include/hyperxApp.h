#ifndef HYPERXAPP_H
#define HYPERXAPP_H

#include <wx/wx.h>
#include "hyperxFrame.h"

class hyperxApp : public wxApp {
public:
  hyperxApp();
  ~hyperxApp();

  virtual bool OnInit();

private:
  hyperxFrame *m_frame;
};
DECLARE_APP(hyperxApp)

#endif
