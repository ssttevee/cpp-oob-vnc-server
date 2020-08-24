#ifndef AV__ERROR_HPP
#define AV__ERROR_HPP

#include <stdlib.h>
#include <exception>

extern "C"
{
#include <libavutil/error.h>
}

namespace av
{
    class error : public std::exception
    {
        char msg[AV_ERROR_MAX_STRING_SIZE];

    public:
        error(int errnum)
        {
            av_strerror(errnum, msg, AV_ERROR_MAX_STRING_SIZE);
        }
        const char *what()
        {
            return msg;
        }
    };
} // namespace av

#endif