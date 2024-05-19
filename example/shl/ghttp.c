#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SERVER_PORT 8080
#define LISTEN_BACKLOG 5
#define RECV_BUFFER_SIZE 128

// HTTP响应
const char *http_response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello World!";

// HTTP请求处理函数
void http_request_handler(int client_socket)
{
    char recv_buffer[RECV_BUFFER_SIZE];
    int recv_len;

    // 接收客户端请求
    recv_len = recv(client_socket, recv_buffer, sizeof(recv_buffer) - 1, 0);
    if (recv_len > 0)
    {
        recv_buffer[recv_len] = '\0';

        // TODO: 根据需要处理HTTP请求内容

        // 发送HTTP响应
        send(client_socket, http_response, strlen(http_response), 0);
    }

    // 关闭客户端连接
    close(client_socket);
}

// HTTP服务器任务
void http_server_task(void *pvParameters)
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, http_client_addr;
    socklen_t client_addr_len;

    // 创建服务器套接字
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        printf("HTTP_SERVER: Failed to create server socket");
    }

    // 设置服务器地址和端口
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // 绑定服务器套接字到指定地址和端口
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("HTTP_SERVER: Failed to bind server socket");
        close(server_socket);
    }

    // 监听连接请求
    if (listen(server_socket, LISTEN_BACKLOG) < 0)
    {
        printf("HTTP_SERVER: Failed to listen on server socket");
        close(server_socket);
    }
    printf("ok!!\n");
    printf("HTTP_SERVER: HTTP server listening on port %d", SERVER_PORT);

    while (1)
    {
        // 接受客户端连接请求
        client_addr_len = sizeof(http_client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&http_client_addr, &client_addr_len);
        if (client_socket < 0)
        {
            //printf("HTTP_SERVER: Failed to accept client connection");
            continue;
        }

        // 处理HTTP请求
        http_request_handler(client_socket);
    }
}
