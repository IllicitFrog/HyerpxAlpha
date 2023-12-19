#include "hyperxFrame.h"
#include "SwitchCtrl.h"
#include "alpha_w.h"
#include <wx/gbsizer.h>
#include <wx/slider.h>

hyperx_frame::hyperx_frame(const wxChar *title, const wxPoint &pos,
                           const wxSize &size)
    : wxFrame(nullptr, wxID_ANY, title, pos, size), m_headset(new headset) {
  t = std::thread(&hyperx_frame::read_loop, this);

  // taskbar icon add menu to quit  and other stuff
  wicon = wxIcon(wxIconLocation(
      wxT("/usr/share/icons/Tokyonight-Moon/devices/64/audio-headset.svg")));
  taskBarIcon = new wxTaskBarIcon();
  taskBarIcon->SetIcon(wicon, _T("Battery: N/A"));
  taskBarIcon->Bind(wxEVT_TASKBAR_LEFT_DCLICK, &hyperx_frame::show, this);

  // layout modifiers
  const auto margin = FromDIP(10);
  auto mainSizer = new wxBoxSizer(wxVERTICAL);
  wxPanel *panel = new wxPanel(this, wxID_ANY);
  auto sizer = new wxGridBagSizer(margin, margin);

  // static text display
  auto manufacturer =
      new wxStaticText(panel, wxID_ANY, m_headset->print_manufacturer());
  auto product = new wxStaticText(panel, wxID_ANY, m_headset->print_product());
  auto serial = new wxStaticText(panel, wxID_ANY, m_headset->print_serial());


  // Pulse Audio Controls
  auto pulseBox = new wxStaticBox(panel, wxID_ANY, _T("Pulse Audio Controls"));
  auto micMute = new wxSwitchCtrl(pulseBox, wxID_ANY, false, _T("Mic Mute"));
  auto mute = new wxSwitchCtrl(pulseBox, wxID_ANY, false, _T("Mute"));
  auto micVolume = new wxSlider(pulseBox, wxID_ANY, 0, 0, 100);
  auto volume = new wxSlider(pulseBox, wxID_ANY, 0, 0, 100);

  // TIMEOUT SELECTOR
  auto featureBox = new wxStaticBox(panel, wxID_ANY, _T("Headphone Features"));
  wxArrayString choices = {_T("30 Minutes"), _T("20 Minutes"),
                           _T("10 Minutes")};
  sleepTimer = new wxComboBox(featureBox, wxID_ANY, choices[0], wxDefaultPosition,
                              wxDefaultSize, choices, wxCB_READONLY,
                              wxDefaultValidator, "Sleep Timer");
  sleepTimer->Bind(wxEVT_COMBOBOX_DROPDOWN, &hyperx_frame::dropdown, this);
  // toggle features
  auto voicePrompt =
      new wxSwitchCtrl(featureBox, wxID_ANY, false, _T("Voice Prompts"));
  auto micMonitor = new wxSwitchCtrl(featureBox, wxID_ANY, false, _T("Mic Monitor"));


  sizer->Add(manufacturer, wxGBPosition(0, 0), wxGBSpan(2, 1), wxALIGN_RIGHT);
  sizer->Add(product, wxGBPosition(0, 1), wxGBSpan(2, 1), wxALIGN_RIGHT);
  sizer->Add(serial, wxGBPosition(0, 2), wxGBSpan(2, 1), wxALIGN_RIGHT);
  sizer->Add(pulseBox,wxGBPosition(0, 3), wxGBSpan(2, 5), wxALIGN_RIGHT);
  sizer->Add(featureBox, wxGBPosition(2, 0), wxGBSpan(1, 4), wxALIGN_RIGHT);

  panel->SetSizer(sizer);
  mainSizer->Add(panel, 1, wxEXPAND | wxALL, margin);
  this->SetSizerAndFit(mainSizer);


  Show(true);
  m_headset->send_command(commands::CONNECTION_STATE);
}

hyperx_frame::~hyperx_frame() { t.join(); }

void hyperx_frame::show(wxTaskBarIconEvent &event) {
  if (this->IsShown()) {
    this->Hide();
  } else {
    this->Show();
  }
}

void hyperx_frame::click(wxCommandEvent &event) {
  m_headset->send_command(commands::STATUS_REQUEST);
}

void hyperx_frame::dropdown(wxCommandEvent &event) {
  if (sleepTimer->GetValue() == _T("30 Minutes")) {
    sleep = sleep_time::S30;
    m_headset->send_command(commands::SLEEP_TIMER_30);
  } else if (sleepTimer->GetValue() == _T("20 Minutes")) {
    sleep = sleep_time::S20;
    m_headset->send_command(commands::SLEEP_TIMER_20);
  } else if (sleepTimer->GetValue() == _T("10 Minutes")) {
    sleep = sleep_time::S10;
    m_headset->send_command(commands::SLEEP_TIMER_10);
  }
}

void hyperx_frame::read_loop() {
  unsigned char buffer[32];
  while (true) {
    m_headset->read(buffer);
    if (buffer[0] == 0x21 && buffer[1] == 0xbb) {
      switch (buffer[2]) {
        // status update
      case 0x03:
        if (buffer[3] == 0x01) {
          status = connection_status::DISCONNECTED;
          statusBar->SetStatusText(_T("Disconnected"));
        } else if (buffer[3] == 0x02) {
          status = connection_status::CONNECTED;
          statusBar->SetStatusText(_T("Connected"));
        }
        break;
      case 0x05:
        // same dont know
        break;
      case 0x07:
        if (buffer[3] == 0x14) {
          // dont know what this does
        }
        break;
      case 0x09:
        // don't know
        break;
      case 0x0a:
        // again no idea
        break;
      case 0x0b:
        battery = (unsigned int)buffer[3];
        taskBarIcon->SetIcon(wicon,
                             _T("Battery: ") + std::to_string(battery) + "%");
        break;
      case 0x0c:
        // response to 0c
        break;
      case 0x0d:
        identifier =
            (unsigned long)buffer[3] << 40 | (unsigned long)buffer[4] << 32 |
            (unsigned long)buffer[5] << 24 | (unsigned long)buffer[6] << 16 |
            (unsigned long)buffer[7] << 8 | (unsigned long)buffer[8];
        std::cout << "Identifier: " << identifier << std::endl;
        break;
      case 0x10:
        if (buffer[3] == 0x00 && buffer[4] == 0x01) {
          voice = false;
        } else if (buffer[3] == 0x01 && buffer[4] == 0x01) {
          voice = true;
        }
        break;
      case 0x12:
        if (buffer[3] == 0x0a && buffer[4] == 0x01) {
          sleep = S10;
          sleepTimer->SetValue(_T("10 Minutes"));
        } else if (buffer[3] == 0x14 && buffer[4] == 0x01) {
          sleep = S20;
          sleepTimer->SetValue(_T("20 Minutes"));
        } else if (buffer[3] == 0x1e && buffer[4] == 0x01) {
          sleep = S30;
          sleepTimer->SetValue(_T("30 Minutes"));
        }
        break;
      case 0x13:
        if (buffer[3] == 0x00) {
          voice = false;
        } else if (buffer[3] == 0x01) {
          voice = true;
        }
        break;
      case 0x22:
        if (buffer[3] == 0x00) {
          mic_monitor = false;
        } else if (buffer[3] == 0x01) {
          mic_monitor = true;
        }
        break;
      case 0x23:
        if (buffer[3] == 0x00) {
          muted = false;
        } else if (buffer[3] == 0x01) {
          muted = true;
        }
        break;
      case 0x24:
        if (buffer[3] == 0x01) {
          status = DISCONNECTED;
        } else if (buffer[3] == 0x02) {
          status = CONNECTED;
        }
        break;
      }
    }
    memset(buffer, 0, 32);
  }
}
