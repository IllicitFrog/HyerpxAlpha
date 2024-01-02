#include "dialog.h"
#include <wx/event.h>

dialog::dialog(const wxChar *title, const wxPoint &pos, const wxSize &size,
               const wxChar *staticImg)
    : wxDialog(NULL, -1, title, pos, size),
      logo(wxImage(staticImg, wxBITMAP_TYPE_PNG)) {

  dialogTimer = new wxTimer();
  dialogTimer->Bind(wxEVT_TIMER,
                    [this](wxTimerEvent &event) { this->Destroy(); });
  dialogTimer->Start(3000, true);

  this->Bind(wxEVT_CLOSE_WINDOW,
             [this](wxCloseEvent &event) { this->Destroy(); });
  dialogSizer = new wxBoxSizer(wxVERTICAL);
  dialogPanel = new wxPanel(this, wxID_ANY);

  dialogLogo =
      new wxStaticBitmap(dialogPanel, wxID_ANY, logo, wxDefaultPosition,
                         wxSize(size.GetWidth(), size.GetHeight() - 50));
  dialogLogo->SetMinSize(wxSize(400, 150));

  dialogSizer->Add(dialogPanel, 1, wxALL | wxEXPAND, 20);
  SetSizer(dialogSizer);

  ShowModal();
}

dialog::~dialog() { this->Destroy(); }
