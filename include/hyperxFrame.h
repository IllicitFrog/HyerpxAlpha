#ifndef __HYPERXFRAME_H
#define __HYPERXFRAME_H

#include <wx/taskbar.h>
#include <wx/wx.h>
#include <wx/combobox.h>
#include <thread>
#include "alpha_w.h"

// main window
class hyperx_frame : public wxFrame {
public:
  hyperx_frame(const wxChar *title, const wxPoint &pos, const wxSize &size);

  ~hyperx_frame();

private:
  wxTaskBarIcon *taskBarIcon;
  wxIcon wicon;
  wxComboBox *sleepTimer;
  wxStatusBar *statusBar;

  std::thread t;
  //headset data
  headset *m_headset;
  sleep_time sleep;
  connection_status status;
  unsigned int battery;
  bool muted;
  bool voice;
  bool mic_monitor;
  unsigned long identifier;

  void show(wxTaskBarIconEvent &event);
  void click(wxCommandEvent &event);
  void dropdown(wxCommandEvent &event);
  void read_loop();
};
#endif
