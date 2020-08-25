#include <stdexcept>

#include "frame_iterator.hpp"

#include "av/error.hpp"

using namespace std;

frame_iterator::frame_iterator(string filename, enum AVPixelFormat out_fmt)
{
    int ret;
    if ((ret = avformat_open_input(&pFormatContext, filename.c_str(), NULL, NULL)) < 0)
    {
        throw av::error(ret);
    }

    if ((ret = avformat_find_stream_info(pFormatContext, NULL)) < 0)
    {
        avformat_free_context(pFormatContext);
        throw av::error(ret);
    }

    int streamIndex = -1;
    for (int i = 0; i < pFormatContext->nb_streams; i++)
    {
        if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            streamIndex = i;
            break;
        }
    }

    if (streamIndex == -1)
    {
        avformat_free_context(pFormatContext);
        throw runtime_error("failed to find video stream");
    }

    try
    {
        it = new av::FrameIterator(pFormatContext, streamIndex);
    }
    catch (const av::error &e)
    {
        avformat_free_context(pFormatContext);
        throw_with_nested(runtime_error("failed to initialize frame iterator"));
    }

    const enum AVPixelFormat in_fmt = it->pix_fmt();
    if (in_fmt != out_fmt)
    {
        pSwsContext = sws_getContext(width(), height(), in_fmt, width(), height(), out_fmt, 0, NULL, NULL, NULL);
    }

    pFrame = av_frame_alloc();
}

frame_iterator::~frame_iterator()
{
    av_frame_free(&pFrame);
    delete it;
    sws_freeContext(pSwsContext);
    avformat_free_context(pFormatContext);
}

long frame_iterator::time_base_nsec()
{
    AVRational time_base = pFormatContext->streams[streamIndex]->time_base;
    return 1000000000 * (long)time_base.num / (long)time_base.den;
}

bool frame_iterator::next(char *buf, int linesize, long *out_pts)
{
    if (!it->next(pFrame))
    {
        return false;
    }

    if (pSwsContext)
    {
        sws_scale(pSwsContext, pFrame->data, pFrame->linesize, 0, height(), (uint8_t *const *)&buf, &linesize);
    }
    else
    {
        // TODO fix copying non-contiguous data
        memcpy(buf, pFrame->data[0], linesize * height());
    }

    if (out_pts) {
        *out_pts = pFrame->pts;
    }

    av_frame_unref(pFrame);

    return true;
}

void frame_iterator::rewind()
{
    return it->rewind();
}
