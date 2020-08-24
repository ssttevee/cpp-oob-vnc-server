#ifndef AV__FRAME_ITERATOR_HPP
#define AV__FRAME_ITERATOR_HPP

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/pixfmt.h>
}

namespace av
{
    class FrameIterator
    {
        AVFormatContext *pFormatContext;
        int streamIndex;

        AVCodecContext *pCodecContext;
        AVPacket *pPacket;

        int state;

    public:
        FrameIterator(AVFormatContext *pFormatContext, int streamIndex);
        ~FrameIterator();

        int width();
        int height();
        enum AVPixelFormat pix_fmt();

        bool next(AVFrame *pFrame);
        void rewind();
    };
} // namespace av

#endif
