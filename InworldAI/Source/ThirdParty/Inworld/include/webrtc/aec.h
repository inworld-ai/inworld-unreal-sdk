#ifndef __AEC_H_
#define __AEC_H_

#include <stdint.h>

#if defined(WEBRTC_WIN)
#define WEBRTC_PLUGIN_API __declspec(dllexport)
#elif defined(WEBRTC_ANDROID)
#define WEBRTC_PLUGIN_API __attribute__((visibility("default")))
#else
#define WEBRTC_PLUGIN_API
#endif


#ifdef __cplusplus
extern "C" {
#endif

struct Metrics {
    double echo_return_loss;
    double echo_return_loss_enhancement;
    int32_t delay_ms;
};

WEBRTC_PLUGIN_API void* WebRtcAec3_Create(int32_t sample_rate_hz);
WEBRTC_PLUGIN_API void WebRtcAec3_Free(void *handle);
// Receives 10ms of samples
WEBRTC_PLUGIN_API int32_t WebRtcAec3_BufferFarend(void* handle, const int16_t* farend);
WEBRTC_PLUGIN_API int32_t WebRtcAec3_Process(
    void* handle,
    const int16_t* nearend,
    int16_t* out);

WEBRTC_PLUGIN_API void WebRtcAec3_GetMetrics(void* handle, Metrics* metrics);

#ifdef __cplusplus
}
#endif


#endif //__AEC_H_