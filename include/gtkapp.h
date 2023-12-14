#include "alpha_w.h"
#include <gtkmm.h>

class HyperxApp : public Gtk::Window {
public:
  HyperxApp() {
    headset.run();
    auto conn = Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &HyperxApp::getStatus), 5000);
    set_title("HyperX Control");
    set_default_size(200, 200);
  }

private:
  alpha_w::headset headset;

  bool getStatus() {
    headset.send_command(alpha_w::commands::STATUS_REQUEST);
    return true;
  }
  bool UpdateInterface() {}
};
