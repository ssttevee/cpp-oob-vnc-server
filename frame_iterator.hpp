#ifndef FRAME_ITERATOR_HPP
#define FRAME_ITERATOR_HPP

#include <string>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
}

#include "av/frame_iterator.hpp"

class frame_iterator
{
    AVFormatContext *pFormatContext = nullptr;
    AVFrame *pFrame = nullptr;

    struct SwsContext *pSwsContext = nullptr;

    int streamIndex;

    av::FrameIterator *it = nullptr;

public:
    frame_iterator(std::string filename, enum AVPixelFormat pix_fmt);
    ~frame_iterator();

    long time_base_nsec();
    int width() { return it->width(); }
    int height() { return it->height(); }

    bool next(char *buf, int linesize, long *out_pts);

    void rewind();
};

#endif
