#include <stdexcept>
#include <thread>
#include <chrono>

extern "C"
{
#include <rfb/rfb.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}

#include "av/error.hpp"
#include "av/frame_iterator.hpp"
#include "frame_iterator.hpp"

using namespace std;

typedef chrono::steady_clock Clock;
typedef chrono::nanoseconds nsec;

int main()
{
    av_register_all();
    avcodec_register_all();

    frame_iterator it("small_bunny_1080p_60fps.mp4", AV_PIX_FMT_RGB24);

    const int width = it.width();
    const int height = it.height();
    const int linesize = width * 3; // 3 bytes per pixel
    const int bufferSize = linesize * height;

    const nsec time_base_nsec = nsec(it.time_base_nsec());

    rfbScreenInfoPtr server = rfbGetScreen(NULL, 0, width, height, 8, 3, 3);
    server->frameBuffer = (char *)malloc(bufferSize);
    rfbInitServer(server);
    rfbRunEventLoop(server, -1, true);

    Clock::time_point start = Clock::now();
    while (1)
    {
        long prev_pts = 0;
        while (it.next(server->frameBuffer, linesize))
        {
            this_thread::sleep_for(((it.pts() - prev_pts) * time_base_nsec) - (Clock::now() - start));

            rfbMarkRectAsModified(server, 0, 0, width, height);

            prev_pts = it.pts();
            start = Clock::now();
        }

        it.rewind();
    }
}
