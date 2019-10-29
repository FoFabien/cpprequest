#include "request.hpp"
#include <sstream>
#include <iostream>

#define BUFSIZE 512
static bool initialized = false;

Request::Request()
{
    //ctor
}

Request::~Request()
{
    //dtor
}

void Request::request(const std::string& url)
{
    request(parse(url));
}

void Request::request(const Url& url)
{
    SOCKET conn;
    if(!setSocket(url, conn)) return;
    std::stringstream buffer;
    buffer << "GET " << url.path << " HTTP/1.0\r\n";
    buffer << "Host: " << url.server << "\r\n";
    buffer << "\r\n";
    send(conn, buffer.str().c_str(), buffer.str().size(), 0);
    buffer.clear();
    char recbuf[BUFSIZE+1];
    long readsize, totalread = 0;
    while(true)
    {
        readsize = recv(conn, recbuf, BUFSIZE, 0);
        recbuf[readsize] = '\0';
        if(readsize <= 0) break;
        totalread += readsize;
        buffer << recbuf;
    }
    closesocket(conn);
    std::cout << "[SIZE READ]: " << totalread << std::endl;
    std::string general;
    std::string headers;
    std::string content;

    auto p = buffer.str().find("\r\n\r\n");
    if(p != std::string::npos)
    {
        general = buffer.str().substr(0, p);
        headers = buffer.str().substr(p+4);
    }
    else
    {
        p = buffer.str().find("\n\r\n\r");
        if(p != std::string::npos)
        {
            general = buffer.str().substr(0, p);
            headers = buffer.str().substr(p+4);
        }
        else return; // ?
    }
    p = headers.find("\r\n\r\n");
    if(p != std::string::npos)
    {
        content = headers.substr(p+4);
        headers = headers.substr(0, p);
        std::cout << "\n[GENERAL]\n" << general << std::endl;
        std::cout << "\n[HEADERS]\n" << headers << std::endl;
        std::cout << "\n[CONTENT]\n" << content << std::endl;
    }
    else
    {
        std::cout << "\n[GENERAL]\n" << general << std::endl;
        std::cout << "\n[RESPONSE]\n" << headers << std::endl;
    }
}

Url Request::parse(const std::string& url)
{
    std::string buf = url;
    Url out;
    if(buf.substr(0, 7) == "http://")
    {
        buf.erase(0, 7);
        out.protocol = "http";
    }
    else if(buf.substr(0, 8) == "https://")
    {
        buf.erase(0, 8);
        out.protocol = "https";
    }
    else out.protocol = "http";
    auto p = buf.find('/');
    if(p != std::string::npos)
    {
        out.server = buf.substr(0, p);
        out.path = buf.substr(p);
        p = out.path.rfind('/');
        out.file = out.path.substr(p+1);
    }
    else
    {
        out.server = buf;
        out.path = "/";
        out.file = "";
    }
    p = out.server.find(':');
    if(p != std::string::npos)
    {
        std::stringstream ss;
        ss << out.server.substr(p);
        ss >> out.port;
        out.server = out.server.substr(0, p);
    }
    else out.port = 80;
    std::cout << "[URL]\n[Protocol]: " << out.protocol << " \n[Server]: " << out.server << " \n[Port]: " << out.port << " \n[Path]: " << out.path << " \n[File]: " << out.file << std::endl;
    return out;
}

bool Request::setSocket(const Url& url, SOCKET& conn)
{
    struct hostent *hp;
    unsigned int addr;
    struct sockaddr_in server;

    if(!initialized)
    {
        WSADATA wsaData;
        if(WSAStartup(0x101, &wsaData) != 0)
            return false;
        initialized = true;
    }

    conn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (conn == INVALID_SOCKET)
    {
        std::cout << WSAGetLastError() << std::endl;
        return false;
    }

    if(inet_addr(url.server.c_str())==INADDR_NONE)
    {
        hp=gethostbyname(url.server.c_str());
    }
    else
    {
        addr=inet_addr(url.server.c_str());
        hp=gethostbyaddr((char*)&addr,sizeof(addr),AF_INET);
    }

    if(hp==NULL)
    {
        closesocket(conn);
        return false;
    }

    server.sin_addr.s_addr=*((unsigned long*)hp->h_addr);
    server.sin_family=AF_INET;
    server.sin_port=htons(url.port);
    if(connect(conn,(struct sockaddr*)&server,sizeof(server)))
    {
        closesocket(conn);
        return false;
    }
    return true;
}
