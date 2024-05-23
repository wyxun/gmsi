#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/ipc_socket"
#define BUFFER_SIZE 256

int main() {
    int sockfd;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    // 创建 socket
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error opening socket");
        exit(1);
    }

    // 连接服务器
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to socket");
        exit(1);
    }

    printf("Connected to server. Enter a message (press Enter to send):\n");

    while (1) {
        // 从键盘读取输入
        memset(buffer, 0, sizeof(buffer));
        if (fgets(buffer, sizeof(buffer) - 1, stdin) == NULL) {
            perror("Error reading from stdin");
            exit(1);
        }

        // 去掉末尾的换行符
        buffer[strcspn(buffer, "\n")] = '\0';

        // 发送消息到服务器
        if (write(sockfd, buffer, strlen(buffer)) < 0) {
            perror("Error writing to socket");
            exit(1);
        }

        // 读取服务器的响应
        memset(buffer, 0, sizeof(buffer));
        ssize_t num_bytes = read(sockfd, buffer, sizeof(buffer) - 1);
        if (num_bytes < 0) {
            perror("Error reading from socket");
            exit(1);
        } else if (num_bytes == 0) {
            printf("Server disconnected\n");
            break;
        }

        printf("Received response from server: %s\n", buffer);
    }

    // 关闭连接
    close(sockfd);

    return 0;
}
