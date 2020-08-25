#include <arpa/inet.h>

#include "server.hpp"

using namespace std;

string get_ip_str(const struct sockaddr *sa)
{
    char s[INET6_ADDRSTRLEN];
    uint16_t port;

    switch (sa->sa_family)
    {
    case AF_INET:
        inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), s, INET6_ADDRSTRLEN);
        port = htons(((struct sockaddr_in *)sa)->sin_port);
        break;

    case AF_INET6:
        inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), s, INET6_ADDRSTRLEN);
        port = htons(((struct sockaddr_in6 *)sa)->sin6_port);
        break;

    default:
        throw runtime_error("Unknown AF");
    }

    return string(s) + ":" + to_string(port);
}

struct rfb_client_data
{
    ev::io *watcher;
    ev::idle *idle;
    lazy_screen::screen *screen;

    struct sockaddr *addr;
};

void client_disconnect_handler(rfbClientPtr client)
{
    auto data = (rfb_client_data *)client->clientData;
    printf("client %s disconnected\n", get_ip_str(data->addr).c_str());
    delete data->watcher;
    delete data->idle;
    delete data->screen;
    delete data->addr;
    delete data;
}

void server::update_client(ev::idle &watcher, int revents)
{
    rfbUpdateClient((rfbClientPtr)watcher.data);
}

void server::read_client(ev::io &watcher, int revents)
{
    auto rfb_client = (rfbClientPtr)watcher.data;
    rfbProcessClientMessage(rfb_client);
    if (rfb_client->sock < 0)
    {
        rfbClientConnectionGone(rfb_client);
    }
}

void server::accept_client(ev::io &watcher, int revents)
{
    auto client_addr = new sockaddr();
    socklen_t addr_len = sizeof(sockaddr);

    int sock = accept(watcher.fd, client_addr, &addr_len);
    if (sock < 0)
    {
        throw system_error(errno, generic_category());
    }

    printf("connection from %s\n", get_ip_str(client_addr).c_str());

    auto client_watcher = new ev::io(loop);
    client_watcher->set<server, &server::read_client>(this);
    client_watcher->start(sock, ev::READ);

    auto idle_watcher = new ev::idle(loop);
    idle_watcher->set<server, &server::update_client>(this);
    idle_watcher->start();

    auto *data = new rfb_client_data();
    data->watcher = client_watcher;
    data->idle = idle_watcher;
    data->screen = lazy->get_screen();
    data->addr = client_addr;

    rfbClientPtr rfb_client = rfbNewClient(data->screen->screen_info(), sock);
    rfb_client->clientData = data;
    rfb_client->clientGoneHook = &client_disconnect_handler;

    client_watcher->data = rfb_client;
    idle_watcher->data = rfb_client;
}

server::server(int port, string filename, ev::loop_ref loop) : loop(loop), lazy(new lazy_screen(filename))
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == 0)
    {
        throw system_error(errno, generic_category());
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(fd, (struct sockaddr *)&address, sizeof(address)))
    {
        throw system_error(errno, generic_category());
    }

    if (listen(fd, 4))
    {
        throw system_error(errno, generic_category());
    }

    printf("listening on port %d\n", port);

    watcher = new ev::io(loop);
    watcher->set<server, &server::accept_client>(this);
    watcher->start(fd, ev::READ);
}
