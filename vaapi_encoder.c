#include "vaapi_encoder.h"

static struct timeval beg, end;

typedef struct VaapiEncoderContextPriv{
    int pts;
    int last_frame_counts;
    AVCodecContext *enc_ctx;
    AVBufferRef *hw_device_ctx;
    AVFilterGraph *graph;
    AVFilterContext *buffersrc_ctx, *buffersink_ctx;
    AVFrame *input_frame, *sink_frame;
}VaapiEncoderContextPriv;

void vaapi_encoder_register_all(){
    avcodec_register_all();
    avdevice_register_all();
    avfilter_register_all();
    av_register_all();
}

VaapiEncoderContext *vaapi_encoder_alloc_context(){
    VaapiEncoderContext *ctx = malloc(sizeof(VaapiEncoderContext));

    memset(ctx, 0, sizeof(*ctx));
    ctx->fps = 25;
    ctx->format = AV_PIX_FMT_RGBA;

    return ctx;
}

int vaapi_encoder_init(VaapiEncoderContext *ctx){
    char args[1024];
    int i, ret;
    AVCodec *codec;
    //AVFormatContext *oc = NULL;
    VaapiEncoderContextPriv *priv;
    AVFilterInOut *inputs, *outputs;
    AVFilter *buffersrc, *buffersink;

    static const char *drm_device_paths[] = {
        "/dev/dri/renderD128",
        "/dev/dri/card0",
        NULL
    };
    
    assert(ctx);

    priv = malloc(sizeof(VaapiEncoderContextPriv));

    priv->pts = priv->last_frame_counts = 0;

    for(i = 0; drm_device_paths[i]; i++){
        ret = av_hwdevice_ctx_create(&priv->hw_device_ctx, AV_HWDEVICE_TYPE_VAAPI, drm_device_paths[i], NULL, 0);
        if(ret >= 0) break;
    }

    if(!drm_device_paths[i]){
        free(priv);
        fprintf(stderr, "vaapi_encoder_init: create hw device error!\n");
        return ret;
    }

    priv->graph = avfilter_graph_alloc();
    
    sprintf(args, "hwupload,scale_vaapi=%d:%d:format=nv12", ctx->width, ctx->height);
    ret = avfilter_graph_parse2(priv->graph, args, &inputs, &outputs);
    if(ret < 0){
        fprintf(stderr, "avfilter_graph_parse2 failed\n");
        avfilter_graph_free(&priv->graph);
        free(priv);
        return ret;
    }

    for (i = 0; i < priv->graph->nb_filters; i++) {
        priv->graph->filters[i]->hw_device_ctx = av_buffer_ref(priv->hw_device_ctx);
    }

    buffersrc = avfilter_get_by_name("buffer");
    buffersink = avfilter_get_by_name("buffersink");

    sprintf(args, "video_size=%dx%d:pix_fmt=%d:time_base=1/%d:pixel_aspect=1/1:frame_rate=%d/1", ctx->width, ctx->height, ctx->format, ctx->fps, ctx->fps);
    avfilter_graph_create_filter(&priv->buffersrc_ctx, buffersrc, "in", args, NULL, priv->graph);
    avfilter_link(priv->buffersrc_ctx, 0, inputs->filter_ctx, inputs->pad_idx);

    avfilter_graph_create_filter(&priv->buffersink_ctx, buffersink, "out", NULL, NULL, priv->graph);
    avfilter_link(outputs->filter_ctx, outputs->pad_idx, priv->buffersink_ctx, 0);

    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    avfilter_graph_config(priv->graph, NULL);

    //ret = avformat_alloc_output_context2(&oc, NULL, "h264", NULL);
    codec = avcodec_find_encoder_by_name("h264_vaapi");
    if(!codec){
        fprintf(stderr, "Could not find encoder\n");
        avfilter_graph_free(&priv->graph);
        free(priv);
        return -1;
    }
    //st->codecpar->codec_id = codec->id;

    priv->enc_ctx = avcodec_alloc_context3(codec);
    if (!priv->enc_ctx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        avfilter_graph_free(&priv->graph);
        free(priv);
        return -1;
    }
    
    priv->enc_ctx->width = ctx->width;
    priv->enc_ctx->height = ctx->height;
    priv->enc_ctx->time_base = (AVRational){1,ctx->fps};
    priv->enc_ctx->gop_size = 25;
    priv->enc_ctx->max_b_frames = 1;
    priv->enc_ctx->pix_fmt = AV_PIX_FMT_VAAPI;
    /* open it */

    priv->enc_ctx->hw_frames_ctx = av_buffer_ref(priv->buffersink_ctx->inputs[0]->hw_frames_ctx);
    
    if (avcodec_open2(priv->enc_ctx, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        avcodec_free_context(&priv->enc_ctx);
        avfilter_graph_free(&priv->graph);
        free(priv);
        return -1;
    }

    priv->input_frame = av_frame_alloc();
    priv->sink_frame = av_frame_alloc();

    priv->input_frame->format = ctx->format;
    priv->input_frame->width  = ctx->width;
    priv->input_frame->height = ctx->height;

    ret = av_image_alloc(priv->input_frame->data,
                         priv->input_frame->linesize,
                         priv->input_frame->width,
                         priv->input_frame->height,
                         priv->input_frame->format,
                         32);

    ctx->priv = priv;

    return 0;
}

int vaapi_encoder_encode(VaapiEncoderContext *ctx, uint8_t *dst, int *out_len, uint8_t *src){
    int ret, offset = 0;
    AVPacket pkt;
    AVFrame *filtered_frame;
    VaapiEncoderContextPriv *priv = ctx->priv;
    
    if(!src){
        ret = avfilter_graph_request_oldest(priv->graph);
        if(ret < 0){
            if(ret != AVERROR(EAGAIN)){
                return ret;
            }else{
                if(priv->last_frame_counts){
                    av_init_packet(&pkt);
                    pkt.data = NULL;
                    pkt.size = 0;

                    do{
                        ret = avcodec_receive_packet(priv->enc_ctx, &pkt);
                        if(ret < 0){
                            if(ret != AVERROR(EAGAIN))
                                fprintf(stderr, "avcodec receive error:%d!\n", ret);
                            break;
                        }

                        memcpy(dst + offset, pkt.data, pkt.size);
                        offset += pkt.size;

                        av_packet_unref(&pkt);

                        priv->last_frame_counts--;
                    }while(1);
                }

                *out_len = offset;

                return 0;
            }
        }
    }else{
        priv->input_frame->data[0] = src;
        ret = av_buffersrc_add_frame_flags(priv->buffersrc_ctx, priv->input_frame, AV_BUFFERSRC_FLAG_PUSH);
        if(ret < 0){
            fprintf(stderr, "avbuffersrc error:%d!\n", ret);
        }else{
            priv->last_frame_counts++;
        }
    }

    filtered_frame = priv->sink_frame;

    do{
        ret = av_buffersink_get_frame_flags(priv->buffersink_ctx, filtered_frame, AV_BUFFERSINK_FLAG_NO_REQUEST);
        if(ret < 0){
            if(ret != AVERROR(EAGAIN))
                fprintf(stderr, "avbuffersink error:%d!\n", ret);
            break;
        }

        filtered_frame->pict_type = 0;
        if(!priv->pts)
            filtered_frame->pict_type = AV_PICTURE_TYPE_I;
        filtered_frame->pts = priv->pts++;

        ret = avcodec_send_frame(priv->enc_ctx, filtered_frame);
        if(ret < 0){
            fprintf(stderr, "avcodec send error:%d!\n", ret);
        }

        av_init_packet(&pkt);
        pkt.data = NULL;
        pkt.size = 0;

        do{
            ret = avcodec_receive_packet(priv->enc_ctx, &pkt);
            if(ret < 0){
                if(ret != AVERROR(EAGAIN))
                    fprintf(stderr, "avcodec receive error:%d!\n", ret);
                break;
            }

#if 1
            gettimeofday(&end, NULL);
            int ms = end.tv_sec * 1000 + end.tv_usec / 1000 - beg.tv_sec * 1000 - beg.tv_usec / 1000;
            double dms = ms / 1000.0;
            if(!(priv->pts % 30)){
                printf("\t%.1ffps\n", priv->pts / dms);
            }
#endif

            memcpy(dst + offset, pkt.data, pkt.size);
            offset += pkt.size;

            av_packet_unref(&pkt);

            priv->last_frame_counts++;
        }while(1);
        
        av_frame_unref(filtered_frame);
    }while(1);

    *out_len = offset;

    return 0;
}

void vaapi_encoder_uinit(VaapiEncoderContext *ctx){
    int ret;
    AVPacket pkt;
    VaapiEncoderContextPriv *priv;
    assert(ctx);

    priv = ctx->priv;
    ctx->priv = NULL;

    avcodec_send_frame(priv->enc_ctx, NULL);

    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    do{
        ret = avcodec_receive_packet(priv->enc_ctx, &pkt);
        if(ret < 0){
            if(ret != AVERROR_EOF)
                fprintf(stderr, "avcodec receive error:%d!\n", ret);
            break;
        }
        av_packet_unref(&pkt);
    }while(1);

    av_buffer_unref(&priv->hw_device_ctx);
    avfilter_graph_free(&priv->graph);

    av_frame_free(&priv->input_frame);
    av_frame_free(&priv->sink_frame);

    avcodec_free_context(&priv->enc_ctx);

    free(priv);
}

void vaapi_encoder_free_context(VaapiEncoderContext **ctx){
    if(*ctx){
        free(*ctx);
        *ctx = NULL;
    }
}

int main(int argc, char **argv)
{
    struct itimerspec new_value;

    vaapi_encoder_register_all();
    VaapiEncoderContext *ctx = vaapi_encoder_alloc_context();
    ctx->width = 1366;
    ctx->height = 768;
    //ctx->format = AV_PIX_FMT_YUV420P;
    vaapi_encoder_init(ctx);

    uint8_t *src = malloc(1920 * 1080 * 4);
    uint8_t *dst = malloc(1920 * 1080 * 4);

    int x, y, i = 0;
    for (y = 0; y < ctx->height; y++) {
        for (x = 0; x < ctx->width; x++) {
#if 0//argb
            iframe->data[0][y * iframe->linesize[0] + x * 4 + 1] = x + i + y * 3;
            iframe->data[0][y * iframe->linesize[0] + x * 4 + 2] = x + y + i * 3;
            iframe->data[0][y * iframe->linesize[0] + x * 4 + 3] = x + y + i * 3;
#else//rgba
            src[y * ctx->width * 4 + x * 4 + 0] = x + i + y * 3;
            src[y * ctx->width * 4 + x * 4 + 1] = x + y + i * 3;
            src[y * ctx->width * 4 + x * 4 + 2] = x + y + i * 3;
#endif
        }
    }

    FILE *f = fopen("test.h264", "wb");
    int out_len = 0;
    gettimeofday(&beg, NULL);

    new_value.it_value.tv_sec = 0;
    new_value.it_value.tv_nsec = 1;
    new_value.it_interval.tv_sec = 0;
    new_value.it_interval.tv_nsec = 40000 * 1000;
    int fd = timerfd_create(CLOCK_REALTIME, 0);
    int exp;

    //timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL);
    timerfd_settime(fd, 0, &new_value, NULL);

    for(;;){
        read(fd, &exp, sizeof(uint64_t));
        vaapi_encoder_encode(ctx, dst, &out_len, src);
        fwrite(dst, 1, out_len, f);
    }

    vaapi_encoder_unit(ctx);
    vaapi_encoder_free_context(&ctx);

    return 0;
}
