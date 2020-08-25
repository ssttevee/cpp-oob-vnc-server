#ifndef LAZY_SCREEN_HPP
#define LAZY_SCREEN_HPP

#include <string>
#include <mutex>
#include <atomic>
#include <condition_variable>

extern "C"
{
#include <rfb/rfb.h>
}

#include "frame_iterator.hpp"

class lazy_screen
{
    std::mutex mu;
    std::condition_variable cv;
    uint num_clients = 0;
    rfbScreenInfoPtr screen_info = nullptr;

    std::mutex it_mu;
    frame_iterator it;

    void screen_loop();
    bool has_clients()
    {
        std::lock_guard<std::mutex> lock(mu);
        return num_clients > 0;
    };

public:
    class screen;

    lazy_screen(std::string filename) : it(frame_iterator(filename, AV_PIX_FMT_RGB24)){};
    lazy_screen() : lazy_screen(nullptr){};
    screen *get_screen();

    void set_file(std::string filename)
    {
        std::lock_guard<std::mutex> lock(it_mu);

        it = frame_iterator(filename, AV_PIX_FMT_RGB24);
    }
};

class lazy_screen::screen
{
    friend class lazy_screen;

    std::mutex *mu;
    uint *counter;
    rfbScreenInfoPtr info;

private:
    screen(std::mutex *mu, uint *counter, rfbScreenInfoPtr info) : mu(mu), counter(counter), info(info){};

public:
    ~screen()
    {
        std::lock_guard<std::mutex> lock(*mu);
        (*counter)--;
    }

    rfbScreenInfoPtr screen_info() { return info; }
};

#endif