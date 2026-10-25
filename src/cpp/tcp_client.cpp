#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <netdb.h>

int main() {
    const char *server_ip = "tcp-server";
    int port = 9090;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return 1; }


    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo *result = nullptr;
    int status = getaddrinfo(server_ip, "9090", &hints, &result);
    if (status != 0) {
        std::cerr << "Erro ao resolver servidor: "
                  << gai_strerror(status) << '\n';
        close(fd);
        return 1;
    }
    if (connect(fd, result->ai_addr, result->ai_addrlen) < 0) {
        perror("connect");
        freeaddrinfo(result);
        close(fd);
        return 1;
    }
    freeaddrinfo(result);
    char buf[256];
    ssize_t n = read(fd, buf, sizeof(buf)-1);
    if (n > 0) {
        buf[n] = '\0';
        std::cout << "TCP client received: " << buf;
    }
    close(fd);
    return 0;
}
