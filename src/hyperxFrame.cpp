#include "hyperxFrame.h"
#include "dialog.h"
#include "hyperxApp.h"
#include <iomanip>

hyperxFrame::hyperxFrame(const wxChar *title, const wxPoint &pos,
                         const wxSize &size, const wxChar *runDir)
    : wxFrame(nullptr, wxID_ANY, title, pos, size), m_headset(new headset),
      wanted(false), m_runDir(runDir), running(true),
      pa_manager("alsa_output.usb-HP__Inc_HyperX_Cloud_Alpha_Wireless_00000001-"
                 "00.iec958-stereo",
                 "alsa_input.usb-HP__Inc_HyperX_Cloud_Alpha_Wireless_00000001-"
                 "00.mono-fallback") {

  if (!m_headset->init()) {
    dialog *error =
        new dialog(_T("HyperX Cloud Alpha Unavailable"), wxDefaultPosition,
                   wxSize(440, 150), m_runDir + _T("img/poweredOff.png"));
    delete m_headset;
    throw std::runtime_error("Failed to initialize headset");
  }

  t = std::thread(&hyperxFrame::read_loop, this);
  // headset polling
  timer = new wxTimer();
  timer->Bind(wxEVT_TIMER, &hyperxFrame::on_timer, this);
  timer->Start(10000);

  taskBarIcon.Bind(wxEVT_TASKBAR_RIGHT_DOWN, &hyperxFrame::showMenu, this);
  setTaskIcon();
  taskBarIcon.Bind(wxEVT_TASKBAR_LEFT_DOWN, &hyperxFrame::showWindow, this);

  createFrame();

  pa_manager.setVolumeSourceChangeCallback(
      [this](unsigned int Tvolume, bool isMute) {
        wxGetApp().CallAfter([this, Tvolume, isMute]() {
          if (isMute) {
            micMute->SetValue(true);
            micMuted = true;
          } else {
            micMute->SetValue(false);
            micMuted = false;
          }
          this->micVolume->SetValue(Tvolume);
        });
      });

  pa_manager.setVolumeSinkChangeCallback(
      [this](unsigned int Tvolume, bool isMute) {
        wxGetApp().CallAfter([this, Tvolume, isMute]() {
          if (isMute) {
            mute->SetValue(true);
            muted = true;
          } else {
            mute->SetValue(false);
            muted = false;
          }
          this->volume->SetValue(Tvolume);
        });
      });

  pt = std::thread([this]() { pa_manager.start(); });
  m_headset->send_command(commands::CONNECTION_STATE);
}

// Status update from headset
void hyperxFrame::on_timer(wxTimerEvent &event) {
  if (status == connection_status::CONNECTED) {
    m_headset->send_command(commands::STATUS_REQUEST);
    m_headset->send_command(commands::PING);
  } else {
    m_headset->send_command(commands::CONNECTION_STATE);
  }
}

// Left Click Taskbar
void hyperxFrame::showWindow(wxTaskBarIconEvent &event) {
  if (status == connection_status::CONNECTED) {
    (IsShown()) ? this->Hide() : this->Show();

  } else {
    if (!dialogShown) {
      dialogShown = true;
      dialog *poweroff =
          new dialog(_T("Headset Off"), wxDefaultPosition, wxSize(440, 150),
                     m_runDir + "img/poweredOff.png");
      wanted = true;
    }
    dialogShown = false;
  }
}

void hyperxFrame::showMenu(wxTaskBarIconEvent &event) {
  enum { hOPEN = 2525, hMUTE, hQUIT };
  taskMenu = new wxMenu();
  if (connection_status::CONNECTED == status) {
    taskMenu->Append(hOPEN, (IsShown()) ? _T("Hide") : _T("Show"), _T(""),
                     wxITEM_NORMAL);
    taskMenu->Append(hMUTE, (micMuted) ? _T("Unmute") : _T("Mute"), _T(""),
                     wxITEM_NORMAL);
  } else {
    taskMenu->Append(wxID_ANY, _T("Power Off"), _T(""), wxITEM_NORMAL);
  }
  taskMenu->AppendSeparator();
  taskMenu->Append(hQUIT, _T("Quit"), _T(""), wxITEM_NORMAL);
  taskMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent &event) {
    switch (event.GetId()) {
    case hQUIT:
      this->quit(event);
      break;
    case hMUTE:
      pa_manager.muteSource(!this->micMuted);
      break;
    case hOPEN:
      (IsShown()) ? this->Hide() : this->Show();
      break;
    default:;
      break;
    }
  });
  taskBarIcon.PopupMenu(taskMenu);
}

void hyperxFrame::micSwitch(wxCommandEvent &event) {
  (micMonitor->GetValue())
      ? m_headset->send_command(commands::MICROPHONE_MONITOR)
      : m_headset->send_command(commands::MICROPHONE_MONITOR_OFF);
}

void hyperxFrame::voiceSwitch(wxCommandEvent &event) {
  (voicePrompt->GetValue())
      ? m_headset->send_command(commands::VOICE_PROMPTS)
      : m_headset->send_command(commands::VOICE_PROMPTS_OFF);
}

void hyperxFrame::quit(wxCommandEvent &event) {
  pa_manager.stop();
  running = false;
  m_headset->send_command(commands::PING);
  if (timer->IsRunning()) {
    timer->Stop();
  }
  t.join();
  pt.join();
  taskBarIcon.RemoveIcon();
  delete m_headset;
  this->Destroy();
}

void hyperxFrame::on_micMute(wxCommandEvent &event) {
  if (event.GetEventType() == 10321) {
    pa_manager.muteSource(micMute->GetValue());
  }
}

void hyperxFrame::on_mute(wxCommandEvent &event) {
  if (event.GetEventType() == 10321) {
    pa_manager.muteSink(mute->GetValue());
  }
}

void hyperxFrame::on_volume(wxCommandEvent &event) {
  pa_manager.setSinkVolume(volume->GetValue());
}

void hyperxFrame::on_micVolume(wxCommandEvent &event) {
  pa_manager.setSourceVolume(micVolume->GetValue());
}

void hyperxFrame::sleepChoice(wxCommandEvent &event) {
  switch (sleepTimer->GetSelection()) {
  case 0:
    m_headset->send_command(commands::SLEEP_TIMER_30);
    break;
  case 1:
    m_headset->send_command(commands::SLEEP_TIMER_20);
    break;
  case 2:
    m_headset->send_command(commands::SLEEP_TIMER_10);
    break;
  }
}

void hyperxFrame::setTaskIcon() {
  if (status == connection_status::CONNECTED) {
    switch (battery) {
    case 0 ... 10:
      wicon = wxIcon(wxIconLocation(m_runDir + _T("img/tray0.png")));
      break;
    case 11 ... 30:
      wicon = wxIcon(wxIconLocation(m_runDir + _T("img/tray20.png")));
      break;
    case 31 ... 50:
      wicon = wxIcon(wxIconLocation(m_runDir + _T("img/tray40.png")));
      break;
    case 51 ... 70:
      wicon = wxIcon(wxIconLocation(m_runDir + _T("img/tray60.png")));
      break;
    case 71 ... 90:
      wicon = wxIcon(wxIconLocation(m_runDir + _T("img/tray80.png")));
      break;
    case 91 ... 100:
      wicon = wxIcon(wxIconLocation(m_runDir + _T("img/tray100.png")));
      break;
    }
    taskBarIcon.SetIcon(wicon, "Battery: " + std::to_string(battery) + "%");
  } else {
    wicon = wxIcon(wxIconLocation(m_runDir + _T("img/traydc.png")));
    taskBarIcon.SetIcon(wicon, "Power Off");
  }
}

void hyperxFrame::createFrame() {
  // main Layout

  this->Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent &event) { this->Hide(); });
  this->Bind(wxEVT_ICONIZE, [this](wxIconizeEvent &event) { this->Hide(); });
  const auto margin = FromDIP(5);
  auto mainSizer = new wxBoxSizer(wxHORIZONTAL);
  wxPanel *panel = new wxPanel(this, wxID_ANY);
  auto sizer = new wxBoxSizer(wxVERTICAL);

  // logo
  wxBitmapBundle logoImg(
      wxImage(m_runDir + "img/hyperx.png", wxBITMAP_TYPE_PNG));
  auto logo = new wxStaticBitmap(panel, wxID_ANY, logoImg, wxDefaultPosition,
                                 wxSize(400, 150));
  auto mainControls = new wxBoxSizer(wxHORIZONTAL);

  statusLabel = new wxStaticText(panel, wxID_ANY, _T("Battery Life Remaining"));
  // mic audio
  auto audioBox = new wxStaticBoxSizer(wxVERTICAL, panel, _T("Audio Controls"));
  micMuteLabel = new wxStaticText(panel, wxID_ANY, _T("Microphone Mute"));
  micMute = new wxSwitchCtrl(panel, wxID_ANY, false);
  micMute->SetToolTip(_T("Mute Microphone"));
  micMute->Bind(wxEVT_SWITCH, &hyperxFrame::on_micMute, this);
  micVolumeLabel = new wxStaticText(panel, wxID_ANY, _T("Microphone Volume"));
  micVolume = new wxSlider(panel, wxID_ANY, 0, 0, 100, wxDefaultPosition,
                           wxDefaultSize, wxSL_MIN_MAX_LABELS);
  micVolume->SetToolTip(_T("Set Microphone Volume"));
  micVolume->Bind(wxEVT_SCROLL_THUMBRELEASE, &hyperxFrame::on_micVolume, this);
  // headphone audio
  muteLabel = new wxStaticText(panel, wxID_ANY, _T("Headphone Mute"));
  mute = new wxSwitchCtrl(panel, (int)wxID_ANY, false);
  mute->SetToolTip(_T("Mute Headphone"));
  mute->Bind(wxEVT_SWITCH, &hyperxFrame::on_mute, this);
  volumeLabel = new wxStaticText(panel, wxID_ANY, _T("Headphone Volume"));
  volume = new wxSlider(panel, wxID_ANY, 0, 0, 100, wxDefaultPosition,
                        wxDefaultSize, wxSL_MIN_MAX_LABELS);
  volume->SetToolTip(_T("Set Headphone Volume"));
  volume->Bind(wxEVT_SCROLL_THUMBRELEASE, &hyperxFrame::on_volume, this);

  // add to audio box
  audioBox->Add(micMuteLabel, 0, wxALIGN_RIGHT | wxALL, margin);
  audioBox->Add(micMute, 0, wxALL | wxALIGN_RIGHT, margin);
  audioBox->Add(micVolumeLabel, 0, wxALIGN_RIGHT | wxALL, margin);
  audioBox->Add(micVolume, 1, wxEXPAND | wxALL, margin);
  audioBox->Add(muteLabel, 0, wxALIGN_RIGHT | wxALL, margin);
  audioBox->Add(mute, 0, wxALL | wxALIGN_RIGHT, margin);
  audioBox->Add(volumeLabel, 0, wxALIGN_RIGHT | wxALL, margin);
  audioBox->Add(volume, 1, wxEXPAND | wxALL, margin);

  // Feature
  auto featureBox = new wxStaticBoxSizer(wxVERTICAL, panel, _T("Features"));
  // sleepTimer
  sleepTimerLabel = new wxStaticText(panel, wxID_ANY, _T("Sleep Timer"));
  sleepTimer =
      new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices);
  sleepTimer->SetSelection(0);
  sleepTimer->SetToolTip(_T("Set Sleep Timer"));
  sleepTimer->Bind(wxEVT_CHOICE, &hyperxFrame::sleepChoice, this);
  // voicePrompt
  voicePromptLabel = new wxStaticText(panel, wxID_ANY, _T("Voice Prompt"));
  voicePrompt = new wxSwitchCtrl(panel, wxID_ANY, false);
  voicePrompt->SetToolTip(_T("Enable Voice Prompt"));
  voicePrompt->Bind(wxEVT_SWITCH, &hyperxFrame::voiceSwitch, this);

  // micMonitor
  micMonitorLabel = new wxStaticText(panel, wxID_ANY, _T("Mic Monitor"));
  micMonitor = new wxSwitchCtrl(panel, wxID_ANY, false);
  micMonitor->SetToolTip(_T("Enable Mic Monitor"));
  micMonitor->Bind(wxEVT_SWITCH, &hyperxFrame::micSwitch, this);

  // add to feature box
  featureBox->Add(sleepTimerLabel, 0, wxEXPAND | wxALL, margin);
  featureBox->Add(sleepTimer, 0, wxEXPAND | wxALL, margin);
  featureBox->Add(voicePromptLabel, 0, wxALIGN_RIGHT | wxALL, margin);
  featureBox->Add(voicePrompt, 0, wxALIGN_RIGHT | wxALL, margin);
  featureBox->Add(micMonitorLabel, 0, wxALIGN_RIGHT | wxALL, margin);
  featureBox->Add(micMonitor, 0, wxALIGN_RIGHT | wxALL, margin);

  auto buttonBox = new wxBoxSizer(wxVERTICAL);
  quitButton = new wxButton(panel, wxID_EXIT, _T("Quit"));
  quitButton->Bind(wxEVT_BUTTON, &hyperxFrame::quit, this);
  hideButton = new wxButton(panel, wxID_ANY, _T("Minimize"));
  hideButton->Bind(wxEVT_BUTTON,
                   [this](wxCommandEvent &event) { this->Hide(); });

  buttonBox->Add(featureBox, 0, wxEXPAND | wxALL, margin);
  buttonBox->Add(quitButton, 0, wxEXPAND | wxALL, margin);
  buttonBox->Add(hideButton, 0, wxEXPAND | wxALL, margin);

  mainControls->Add(audioBox, 1, wxEXPAND | wxALL, margin);
  mainControls->Add(buttonBox, 1, wxALL, margin);

  sizer->Add(logo, 0, wxLEFT | wxRIGHT, margin);
  sizer->Add(statusLabel, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, margin);
  sizer->Add(mainControls, 0, wxEXPAND | wxALL, margin);

  panel->SetSizer(sizer);
  mainSizer->Add(panel, 1, wxALL, 20);
  this->SetSizerAndFit(mainSizer);
}

void hyperxFrame::onConnect() {
  m_headset->send_command(commands::VOICE_STATE);
  m_headset->send_command(commands::MIC_MONITOR_STATE);
  m_headset->send_command(commands::SLEEP_STATE);
  m_headset->send_command(commands::MICROPHONE_STATE);
  m_headset->send_command(commands::STATUS_REQUEST);
  status = connection_status::CONNECTED;
  setTaskIcon();
  pa_manager.getSinkVolume();
  pa_manager.getSourceVolume();
  if (wanted) {
    Show();
    wanted = false;
  }
  timer->Start(30000);
}

void hyperxFrame::read_loop() {
  unsigned char buffer[32];
  while (running) {
    m_headset->read(buffer);
    wxGetApp().CallAfter([this, &buffer]() {
      if (buffer[0] == 0x21 && buffer[1] == 0xbb) {
        switch (buffer[2]) {
        case 0x03:
          if (buffer[3] == 0x01) {
            status = connection_status::DISCONNECTED;
            setTaskIcon();
            if (IsShown()) {
              Hide();
            }
          } else if (buffer[3] == 0x02) {
            onConnect();
          }
          break;

        // STILL DONT KNOW
        case 0x05:
          break;

        // READ SLEEP STATE SETTTING
        case 0x07:
          switch (buffer[3]) {
          case 0x0a:
            sleep = sleep_time::S30;
            sleepTimer->SetSelection(0);
            break;
          case 0x14:
            sleep = sleep_time::S20;
            sleepTimer->SetSelection(1);
            break;
          case 0x1e:
            sleep = sleep_time::S10;
            sleepTimer->SetSelection(2);
            break;
          }
          break;

        // VOICE PROMPTS
        case 0x09:
          if (buffer[3] == 0x01) {
            voice = true;
            voicePrompt->SetValue(true);
          } else if (buffer[3] == 0x00) {
            voice = false;
            voicePrompt->SetValue(false);
          }
          break;

        // STILL DONT KNOW
        case 0x0a:
          break;

        // Battery Check
        case 0x0b:
          battery = (unsigned int)buffer[3];
          statusLabel->SetLabel("Battery: " + std::to_string(battery) +
                                "% -- " + std::to_string(battery * 3) +
                                "Hours Remaining");
          setTaskIcon();
          break;

        // PING IM GUESSING?
        case 0x0c:
          break;

        case 0x0d:
          identifier =
              (unsigned long)buffer[3] << 40 | (unsigned long)buffer[4] << 32 |
              (unsigned long)buffer[5] << 24 | (unsigned long)buffer[6] << 16 |
              (unsigned long)buffer[7] << 8 | (unsigned long)buffer[8];
          break;

        // RESPONSE TO SLEEP TIMER SET
        case 0x12:
          switch (buffer[3]) {
          case 0x0a:
            sleep = S10;
            break;
          case 0x14:
            sleep = S20;
            break;
          case 0x1e:
            sleep = S30;
            break;
          }
          break;

        // VOICE PROMPT RESPONSE
        case 0x13:
          if (buffer[3] == 0x00) {
            voice = false;
          } else if (buffer[3] == 0x01) {
            voice = true;
          }
          break;

        // MICMONIITOR RESPONSE
        case 0x22:
          if (buffer[3] == 0x00) {
            mic_monitor = false;
            micMonitor->SetValue(false);
          } else if (buffer[3] == 0x01) {
            mic_monitor = true;
            micMonitor->SetValue(true);
          }
          break;

        // MIC mute
        case 0x23:
          if (buffer[3] == 0x00) {
            muted = false;
            micMute->SetValue(false);
          } else if (buffer[3] == 0x01) {
            muted = true;
            micMute->SetValue(true);
          }
          break;

        // POWER OFF
        case 0x24:
          if (buffer[3] == 0x01) {
            status = connection_status::DISCONNECTED;
            setTaskIcon();
          } else if (buffer[3] == 0x02) {
            onConnect();
          }
          break;
        }
      }
    });
  }
}
