#include "pulse.h"
#include <thread>

PulseAudioManager::PulseAudioManager(const char *sinkName,
                                     const char *sourceName)
    : sinkName(sinkName), sourceName(sourceName) {
  mainLoop = pa_mainloop_new();
  mainLoopAPI = pa_mainloop_get_api(mainLoop);
  context = pa_context_new(mainLoopAPI, "PulseAudioManager");
  if (pa_context_connect(context, nullptr, PA_CONTEXT_NOAUTOSPAWN, nullptr) <
      0) {
    std::cout << "Failed to connect to PulseAudio" << std::endl;
  }
  pa_context_set_state_callback(context, contextStateCallback, this);
}

PulseAudioManager::~PulseAudioManager() {
  pa_context_unref(context);
  pa_mainloop_free(mainLoop);
}

void PulseAudioManager::start() { pa_mainloop_run(mainLoop, nullptr); }

void PulseAudioManager::stop() { pa_mainloop_quit(mainLoop, 0); }

void PulseAudioManager::setVolumeSourceChangeCallback(
    std::function<void(unsigned int, bool)> cb) {
  onVolumeChangeSource = std::move(cb);
}

void PulseAudioManager::setVolumeSinkChangeCallback(
    std::function<void(unsigned int, bool)> cb) {
  onVolumeChangeSink = std::move(cb);
}

// Set volume for sink by name
void PulseAudioManager::setSinkVolume(uint32_t volume) {
  pa_cvolume cvolume;
  pa_cvolume_init(&cvolume);
  pa_cvolume_set(&cvolume, 2, PA_VOLUME_NORM * volume / 100);
  pa_context_set_sink_volume_by_name(context, sinkName.c_str(), &cvolume,
                                     nullptr, nullptr);
}

// Set volume for source by name
void PulseAudioManager::setSourceVolume(uint32_t volume) {
  pa_cvolume cvolume;
  pa_cvolume_init(&cvolume);
  pa_cvolume_set(&cvolume, 1, PA_VOLUME_NORM * volume / 100);
  pa_context_set_source_volume_by_name(context, sourceName.c_str(), &cvolume,
                                       nullptr, nullptr);
}

// Mute and unmute sink by name
void PulseAudioManager::muteSink(bool mute) {
  pa_context_set_sink_mute_by_name(context, sinkName.c_str(), mute, nullptr,
                                   nullptr);
}

void PulseAudioManager::muteSource(bool mute) {
  pa_operation *op = pa_context_set_source_mute_by_name(context, sourceName.c_str(), mute, nullptr,
                                     nullptr);
  pa_operation_unref(op);
}

// Get volume for sink by name
void PulseAudioManager::getSinkVolume() {
  pa_context_get_sink_info_by_name(context, sinkName.c_str(),
                                   sinkVolumeCallback, this);
}

// Get volume for source by name
void PulseAudioManager::getSourceVolume() {
  pa_context_get_source_info_by_name(context, sourceName.c_str(),
                                     sourceVolumeCallback, this);
}
