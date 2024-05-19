#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/ipc_socket"

int main() {
    int sockfd;
    struct sockaddr_un server_addr;
    char buffer[256] = "Hello, server!";

    // 创建 socket
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error opening socket");
        exit(1);
    }

    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // 连接到服务器
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        exit(1);
    }

    printf("Connected to server\n");

    // 发送消息到服务器
    if (write(sockfd, buffer, strlen(buffer)) < 0) {
        perror("Error writing to socket");
        exit(1);
    }

    printf("Message sent to server: %s\n", buffer);

    // 关闭连接
    close(sockfd);

    return 0;
}
