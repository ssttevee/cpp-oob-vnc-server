extern "C"
{
#include <libavformat/avformat.h>
}

#include "server.hpp"

int main()
{
    av_register_all();
    avcodec_register_all();

    server s(5900, "small_bunny_1080p_60fps.mp4");
    s.serve();

    return 0;
}
