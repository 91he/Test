#ifndef VAAPI_ENCODER_H
#define VAAPI_ENCODER_H

#include <math.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libavformat/avformat.h>
#include <libavutil/version.h>
#include <libavutil/pixfmt.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavutil/hwcontext.h>

typedef struct VaapiEncoderContextPriv VaapiEncoderContextPriv;

typedef struct VaapiEncoderContext{
    int fps;
    int width;
    int height;
    enum AVPixelFormat format;
    VaapiEncoderContextPriv *priv;
}VaapiEncoderContext;

void vaapi_encoder_register_all();
VaapiEncoderContext *vaapi_encoder_alloc_context();
int vaapi_encoder_init(VaapiEncoderContext *ctx);
int vaapi_encoder_encode(VaapiEncoderContext *ctx, uint8_t *dst, int *out_len, uint8_t *src);
void vaapi_encoder_uinit(VaapiEncoderContext *ctx);
void vaapi_encoder_free_context(VaapiEncoderContext **ctx);

#endif
