#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

struct Url
{
    std::string protocol;
    std::string server;
    short port;
    std::string path;
    std::string file;
};

class Request
{
    public:
        Request();
        virtual ~Request();
        static void request(const std::string& url);
        static void request(const Url& url);
        static Url parse(const std::string& url);
        static bool setSocket(const Url& url, SOCKET& conn);

    protected:

};

#endif // REQUEST_H
