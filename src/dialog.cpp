#include "dialog.h"
#include <wx/event.h>

dialog::dialog(const wxChar *title, const wxPoint &pos, const wxSize &size,
               const wxChar *staticImg, bool modal, int timeout)
    : wxDialog(NULL, -1, title, pos, size), m_modal(modal),
      logo(wxImage(staticImg, wxBITMAP_TYPE_PNG)) {

  if (timeout > 0) {
    dialogTimer = new wxTimer();
    dialogTimer->Bind(wxEVT_TIMER,
                      [this](wxTimerEvent &event) { this->Destroy(); });
    dialogTimer->Start(timeout, true);
  }

  this->Bind(wxEVT_CLOSE_WINDOW,
             [this](wxCloseEvent &event) { this->Destroy(); });
  dialogSizer = new wxBoxSizer(wxVERTICAL);
  buttonBox = new wxBoxSizer(wxHORIZONTAL);
  dialogPanel = new wxPanel(this, wxID_ANY);

  dialogLogo =
      new wxStaticBitmap(dialogPanel, wxID_ANY, logo, wxDefaultPosition,
                         wxSize(size.GetWidth(), size.GetHeight() - 50));
  dialogLogo->SetMinSize(wxSize(400, 150));

  closeButton = new wxButton(this, wxID_ANY, "Close");
  closeButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                    [this](wxCommandEvent &event) {
                      if (dialogTimer->IsRunning()) {
                        dialogTimer->Stop();
                        dialogTimer->UnRef();
                      }
                      this->Destroy();
                    });
  buttonBox->Add(closeButton, 0, wxALIGN_CENTER, 0);

  dialogSizer->Add(dialogPanel, 1);
  dialogSizer->Add(buttonBox, 0, wxALIGN_RIGHT | wxALL, 10);
  SetSizer(dialogSizer);

  (m_modal) ? ShowModal() : Show();
}

dialog::~dialog() { this->Destroy(); }
