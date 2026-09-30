#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

int main() {
    const char *socket_path = "/tmp/demo.sock";
    unlink(socket_path);

    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) == -1) { perror("bind"); return 1; }
    if (listen(server_fd, 5) == -1) { perror("listen"); return 1; }

    std::cout << "IPC server listening on " << socket_path << std::endl;

    while (true) {
        int client = accept(server_fd, nullptr, nullptr);
        if (client == -1) { perror("accept"); break; }
        const char *msg = "Hello from ipc_server\n";
        write(client, msg, strlen(msg));
        close(client);
    }

    close(server_fd);
    unlink(socket_path);
    return 0;
}
