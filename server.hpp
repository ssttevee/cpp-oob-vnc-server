#ifndef SERVER_HPP
#define SERVER_HPP

#include <rfb/rfb.h>
#include <ev++.h>

#include "lazy_screen.hpp"

class server
{
    ev::io *watcher;
    ev::loop_ref loop;

    lazy_screen *lazy;

    void update_client(ev::idle &watcher, int revents);
    void read_client(ev::io &watcher, int revents);
    void accept_client(ev::io &watcher, int revents);

public:
    server(int port, std::string filename, ev::loop_ref loop = ev::get_default_loop());

    ~server()
    {
        delete watcher;
        delete lazy;
    }

    void serve() { loop.run(); }
};

#endif
