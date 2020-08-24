#include <stdio.h>
#include <iterator>

#include "frame_iterator.hpp"
#include "error.hpp"

namespace av
{

    FrameIterator::FrameIterator(AVFormatContext *pFormatContext, int streamIndex) : pFormatContext(pFormatContext), streamIndex(streamIndex)
    {
        AVStream *pStream = pFormatContext->streams[streamIndex];

        AVCodecParameters *pCodecParameters = pStream->codecpar;
        AVCodec *pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
        pCodecContext = avcodec_alloc_context3(pCodec);
        if (pCodecContext == NULL)
        {
            throw std::bad_alloc();
        }

        int ret;
        if ((ret = avcodec_parameters_to_context(pCodecContext, pCodecParameters)) < 0)
        {
            avcodec_free_context(&pCodecContext);
            throw av::error(ret);
        }

        if ((ret = avcodec_open2(pCodecContext, pCodec, NULL)) < 0)
        {
            avcodec_free_context(&pCodecContext);
            throw av::error(ret);
        }

        pPacket = av_packet_alloc();
        state = 0;
    }

    FrameIterator::~FrameIterator()
    {
        avcodec_free_context(&pCodecContext);
        av_packet_unref(pPacket);
    }

    enum AVPixelFormat FrameIterator::pix_fmt()
    {
        return pCodecContext->pix_fmt;
    }

    int FrameIterator::width()
    {
        return pCodecContext->width;
    }

    int FrameIterator::height()
    {
        return pCodecContext->height;
    }

    bool FrameIterator::next(AVFrame *pFrame)
    {
        int ret = 0;
        while (!ret)
        {
            switch (state)
            {
            case 0:
                av_packet_unref(pPacket);
                ret = av_read_frame(pFormatContext, pPacket);
                if (pPacket->stream_index == streamIndex)
                {
                    state = 1;
                }
                break;

            case 1:
                ret = avcodec_send_packet(pCodecContext, pPacket);
                state = 2;
                break;

            case 2:
                if ((ret = avcodec_receive_frame(pCodecContext, pFrame)) == AVERROR(EAGAIN))
                {
                    state = 0;
                    ret = 0;
                }
                else if (ret == 0)
                {
                    return true;
                }
                break;

            default:
                printf("invalid frame iterator state: %d\n", state);
                exit(1);
            }
        }

        if (ret == AVERROR_EOF)
        {
            return false;
        }

        throw av::error(ret);
    }

    void FrameIterator::rewind()
    {
        state = 0;
        avcodec_flush_buffers(pCodecContext);
        int ret = av_seek_frame(pFormatContext, streamIndex, 0, AVSEEK_FLAG_BACKWARD);
        if (ret)
        {
            throw av::error(ret);
        }
    }
} // namespace av
