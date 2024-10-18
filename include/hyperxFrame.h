#ifndef __HYPERXFRAME_H
#define __HYPERXFRAME_H

#include <thread>
#include <wx/taskbar.h>
#include <wx/wx.h>
#include <atomic>

#include "SwitchCtrl.h"
#include "alpha_w.h"

// main window
class hyperxFrame : public wxFrame {
public:
  hyperxFrame(const wxChar *title, const wxPoint &pos, const wxSize &size, const wxChar *runDir);

private:
  // Main layout
  wxTaskBarIcon taskBarIcon;
  wxMenu *taskMenu;
  wxIcon wicon;
  wxButton *quitButton;
  wxButton *hideButton;
  wxString m_runDir;
  bool dialogShown = false;

  // features box
  wxStaticText *sleepTimerLabel;
  wxChoice *sleepTimer;
  wxStaticText *voicePromptLabel;
  wxSwitchCtrl *voicePrompt;
  wxStaticText *micMonitorLabel;
  wxSwitchCtrl *micMonitor;

  // headset data
  headset *m_headset;
  sleep_time sleep;
  connection_status status;
  unsigned int battery;
  bool micMuted;
  bool muted;
  bool voice;
  bool mic_monitor;
  unsigned long identifier;
  const wxArrayString choices = {_T("30 Minutes"), _T("20 Minutes"),
                                 _T("10 Minutes"), _T("Never")};


  // callback functions for controls
  void createFrame();
  void setTaskIcon();
  void onConnect();
  void showWindow(wxTaskBarIconEvent &);
  void showMenu(wxTaskBarIconEvent &);
  void sleepChoice(wxCommandEvent &);
  void voiceSwitch(wxCommandEvent &); // hide button
  void micSwitch(wxCommandEvent &);   // hide button
  void quit(wxCommandEvent &);        // quit button

  // timer Event 5 seconds
  wxTimer *dialogTimer;
  wxTimer *timer;
  void on_timer(wxTimerEvent &);

  // read loop for headset
  bool wanted;
  std::atomic<bool> running;
  std::thread t;
  void read_loop();
};
#endif
