#include "gtkapp.h"

int main(int argc, char **argv) {
  auto app = Gtk::Application::create("org.hyperx.control");
  return app->make_window_and_run<HyperxApp>(argc, argv);
}
