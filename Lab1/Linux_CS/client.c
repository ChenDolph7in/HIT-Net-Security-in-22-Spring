// 传输的客户端
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

int main()
{
    // 1.创建套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("socket");
        exit(-1);
    }
    // 2.连接服务器端
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, "192.168.56.104", &serveraddr.sin_addr.s_addr);
    serveraddr.sin_port = htons(9999);
    int ret = connect(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (ret == -1)
    {
        perror("connect");
        exit(-1);
    }
    // 3. 通信
    int data_len, flag = 2;
    char buffer[BUFFER_SIZE];
    while (flag > 0)
    {
        printf("len=%d\n", data_len);
        data_len = recv(fd, buffer, BUFFER_SIZE, 0);
        if (data_len < 0)
        {
            printf("接收错误\n");
            exit(1);
        }
        printf("%s", buffer);
        bzero(buffer, BUFFER_SIZE);
        flag--;
    }
    // 输入传输文件名
    char name[BUFFER_SIZE];
    scanf("%s", name);
    //printf("%s\n", buffer);
    send(fd, name, BUFFER_SIZE, 0);
    // 定义文件流
    FILE *stream;
    flag = 0;
    bzero(buffer, BUFFER_SIZE);
    // 从服务器接收数据*********************************************
    if ((stream = fopen(name, "w")) == NULL)
    {
        printf("file open error\n");
        exit(1);
    }
    else
    {
        bzero(buffer, BUFFER_SIZE);
    }
    // printf("正在接收来自%s的文件....\n",inet_ntoa(client_addr.sin_addr));
    // 先将数据接收到缓冲区buffer中，再写入到新建的文件中
    while (data_len = recv(fd, buffer, BUFFER_SIZE, 0))
    {
        flag++;
        if (flag == 1)
        {
            printf("正在接收文件....\n");
        }

        if (data_len < 0)
        {
            printf("接收错误\n");
            exit(1);
        }
        // 向文件中写入数据
        int write_len = fwrite(buffer, sizeof(char), data_len, stream);
        if (write_len > data_len)
        {
            printf("file write failed\n");
            exit(1);
        }
        bzero(buffer, BUFFER_SIZE);
    }
    if (flag > 0)
        printf("文件传送完毕\n");
    if (flag == 0)
        printf("文件传输失败\n");
    // 从服务器接收数据完毕****************************************************
    fclose(stream);
    // 临时文件重命名
    // rename("data", name);

    // 关闭连接
    close(fd);
    return 0;
}
