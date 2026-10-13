#ifndef NET_H
#define NET_H

#ifdef _WIN32
    #include <windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")   
    typedef int socklen_t;               
    typedef SOCKET socket_t;
    #define SOCK_CLOSE    closesocket
    #define SOCK_ERR      SOCKET_ERROR
    #define SOCK_INVALID  INVALID_SOCKET
    #define sock_last_error() WSAGetLastError()
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <errno.h>
    typedef int socket_t;
    #define SOCK_CLOSE    close
    #define SOCK_ERR      (-1)
    #define SOCK_INVALID  (-1)
    #define sock_last_error() errno
#endif

static inline void console_setup_utf8(void) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

static inline int net_init(void) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return -1;
#endif
    return 0;
}

static inline void net_cleanup(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

#endif