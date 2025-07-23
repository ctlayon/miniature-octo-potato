#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// Struct to hold connection information
struct ConnectionInfo {
    int socket_fd;
    struct sockaddr_in address;
    socklen_t addr_len;
};

int main() {
    int server_fd, client_fd;
    struct ConnectionInfo server_info, client_info;

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Setup server_info
    server_info.socket_fd = server_fd;
    server_info.address.sin_family = AF_INET;
    server_info.address.sin_port = htons(8888);
    server_info.address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_info.addr_len = sizeof(server_info.address);

    // Bind
    if (bind(server_fd, (struct sockaddr*)&server_info.address, server_info.addr_len) == -1) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 1) == -1) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Listening on 127.0.0.1:8888...\n");

    // Accept
    client_fd = accept(server_fd, (struct sockaddr*)&client_info.address, &client_info.addr_len);
    if (client_fd == -1) {
        perror("accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Send "hello"
    const char *message = "hello\n";
    send(client_fd, message, strlen(message), 0);
    printf("Sent message to client.\n");

    // Clean up
    close(client_fd);
    close(server_fd);

    return 0;
}
