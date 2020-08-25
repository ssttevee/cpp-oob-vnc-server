#include <chrono>
#include <thread>

#include "lazy_screen.hpp"

using namespace std;

typedef chrono::steady_clock Clock;
typedef chrono::nanoseconds nsec;

lazy_screen::screen *lazy_screen::get_screen()
{
    unique_lock<mutex> lock(mu);
    num_clients++;

    if (!screen_info)
    {
        thread t(&lazy_screen::screen_loop, this);
        cv.wait(lock, [this] { return !!screen_info; });
        t.detach();
    }

    return new screen(&mu, &num_clients, screen_info);
}

void lazy_screen::screen_loop()
{
    int width, height;
    nsec time_base_nsec;

    {
        lock_guard<mutex> lock(it_mu);
        width = it.width();
        height = it.height();

        time_base_nsec = nsec(it.time_base_nsec());
    }

    const int linesize = width * 3; // 3 bytes per pixel
    const int bufferSize = linesize * height;

    char buf[bufferSize];

    {
        auto screen = rfbGetScreen(NULL, 0, width, height, 8, 3, 3);
        screen->frameBuffer = buf;

        lock_guard<mutex> lock(mu);
        screen_info = screen;
    }

    cv.notify_all();

    auto start = Clock::now();
    long prev_pts = 0, current_pts;
    do
    {
        {
            lock_guard<mutex> lock(it_mu);
            if (!it.next(buf, linesize, &current_pts))
            {
                it.rewind();
                prev_pts = 0;
                continue;
            }
        }

        this_thread::sleep_for(((current_pts - prev_pts) * time_base_nsec) - (Clock::now() - start));

        {
            lock_guard<mutex> lock(mu);
            rfbMarkRectAsModified(screen_info, 0, 0, width, height);
        }

        prev_pts = current_pts;
        start = Clock::now();
    } while (has_clients());

    lock_guard<mutex> lock(mu);
    rfbScreenCleanup(screen_info);
    screen_info = nullptr;
}
