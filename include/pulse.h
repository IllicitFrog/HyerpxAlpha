#ifndef PULSE_H
#define PULSE_H

#include <functional>
#include <iostream>
#include <pulse/introspect.h>
#include <pulse/pulseaudio.h>

class PulseAudioManager {
public:
  PulseAudioManager(const char *sinkName, const char *sourceName);
  ~PulseAudioManager();

  void start();
  void stop();

  void
  setVolumeSourceChangeCallback(std::function<void(unsigned int, bool)> cb);
  void setVolumeSinkChangeCallback(std::function<void(unsigned int, bool)> cb);

  void muteSink(bool mute);
  void setSinkVolume(uint32_t volume);
  void getSinkVolume();
  void muteSource(bool mute);
  void setSourceVolume(uint32_t volume);
  void getSourceVolume();

  void setOnVolumeChangeSource(pa_source_info_cb_t cb);
  void setOnVolumeChangeSink(pa_sink_info_cb_t cb);

private:
  static void contextStateCallback(pa_context *c, void *userdata) {
    auto manager = static_cast<PulseAudioManager *>(userdata);
    switch (pa_context_get_state(c)) {
    case PA_CONTEXT_READY:
      pa_context_set_subscribe_callback(c, subscribeCallback, userdata);
      pa_context_subscribe(
          c,
          (pa_subscription_mask_t)(PA_SUBSCRIPTION_MASK_SOURCE |
                                   PA_SUBSCRIPTION_MASK_SINK),
          nullptr, nullptr);
      break;
    case PA_CONTEXT_FAILED:
    case PA_CONTEXT_TERMINATED:
      pa_mainloop_quit(manager->mainLoop, 0);
      break;
    default:
      break;
    }
  }

  static void subscribeCallback(pa_context *c, pa_subscription_event_type_t t,
                                uint32_t idx, void *userdata) {
    auto manager = static_cast<PulseAudioManager *>(userdata);
    switch(t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
    case PA_SUBSCRIPTION_EVENT_SINK:
      manager->getSinkVolume();
      break;
    case PA_SUBSCRIPTION_EVENT_SOURCE:
      manager->getSourceVolume();
      break;
    default:
      break;
    }
  }

  static void sinkVolumeCallback(pa_context *c, const pa_sink_info *i, int eol,
                                 void *userdata) {
    auto manager = static_cast<PulseAudioManager *>(userdata);
    if (i) {
      manager->onVolumeChangeSink(
          pa_cvolume_avg(&i->volume) * 100 / PA_VOLUME_NORM, i->mute);
    }
  }

  static void sourceVolumeCallback(pa_context *c, const pa_source_info *i,
                                   int eol, void *userdata) {
    auto manager = static_cast<PulseAudioManager *>(userdata);
    if (i) {
      manager->onVolumeChangeSource(
          pa_cvolume_avg(&i->volume) * 100 / PA_VOLUME_NORM, i->mute);
    }
  }

  pa_mainloop *mainLoop;
  pa_mainloop_api *mainLoopAPI;
  pa_context *context;

  std::string sinkName;
  std::string sourceName;

  std::function<void(unsigned int, bool)> onVolumeChangeSink;
  std::function<void(unsigned int, bool)> onVolumeChangeSource;
};

#endif
