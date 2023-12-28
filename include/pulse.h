#ifndef PULSE_H
#define PULSE_H
#include <pulse/error.h>
#include <pulse/pulseaudio.h>

class pulse {
public:
 pulse();

 int init();
 int run();
 void quit();
 void destroy();
 ~pulse();

private:
};

#endif
