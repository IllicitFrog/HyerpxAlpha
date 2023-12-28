#ifndef DIALOG_H
#define DIALOG_H

#include <wx/wx.h>

class dialog : public wxDialog {
public:
  dialog(const wxChar *title, const wxPoint &pos, const wxSize &size,
         const wxChar *staticImg, bool modal = false, int timeout = 0);

  virtual ~dialog();

private:
  wxPanel *dialogPanel;
  wxStaticBitmap *dialogLogo;
  wxBoxSizer *dialogSizer;
  wxBoxSizer *buttonBox;
  wxButton *closeButton;
  wxTimer *dialogTimer;
  wxBitmapBundle logo;
  bool m_modal;
};

#endif
