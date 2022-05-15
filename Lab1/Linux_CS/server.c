// 传输的服务器端
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <dirent.h>

#define BUFFER_SIZE 1024

// 获得basePath文件夹下所有文件列表字符串，输出到buffer中
int readFileList(char *basePath, char *buffer)
{
    DIR *dir;
    struct dirent *ptr;
    char base[1000];

    if ((dir = opendir(basePath)) == NULL)
    {
        perror("Open dir error...");
        return 1;
    }

    while ((ptr = readdir(dir)) != NULL)
    {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) /// current dir OR parrent dir
            continue;
        else if (ptr->d_type == 8) // file
        {
            strcat(buffer, ptr->d_name);
            strcat(buffer, "\n");
        }
        else if (ptr->d_type == 10) // link file
        {
            strcat(buffer, ptr->d_name);
            strcat(buffer, "\n");
        }
        else if (ptr->d_type == 4) // dir
        {
            memset(base, '\0', sizeof(base));
            strcpy(base, basePath);
            strcat(base, "/");
            strcat(base, ptr->d_name);
            readFileList(base, buffer);
        }
    }
    closedir(dir);
    return 1;
}

// 子线程处理函数参数
struct client_addr
{
    int cfd;
    struct sockaddr_in *clientaddr;
};

// 子线程处理函数
void *client_method(void *arg)
{
    struct client_addr *addr = (struct client_addr *)arg;
    struct sockaddr_in *clientaddr = addr->clientaddr;
    int cfd = addr->cfd;

    // 输出客户端的信息
    char clientIP[16];
    inet_ntop(AF_INET, &clientaddr->sin_addr.s_addr, clientIP, sizeof(clientIP));
    unsigned short clientPort = ntohs(clientaddr->sin_port);
    printf("client ip is %s, port is %d\n", clientIP, clientPort);
    // 5.通信
    char buffer[1024] = {0};
    DIR *dir;
    char basePath[1000];

    // 得到当前绝对文件路径
    memset(basePath, '\0', sizeof(basePath));
    getcwd(basePath, 999);
    printf("the current dir is : %s\n", basePath);

    // 得到当前文件夹下文件列表
    readFileList(basePath, buffer);
    // 向客户端发送文件列表
    send(cfd, buffer, BUFFER_SIZE, 0);

    bzero(buffer, BUFFER_SIZE);
    strcpy(buffer, "请输入要传输的文件的文件名：");
    strcat(buffer, "\n");
    send(cfd, buffer, BUFFER_SIZE, 0);
    bzero(buffer, BUFFER_SIZE);

    // 接收客户端请求文件名
    int length = recv(cfd, buffer, BUFFER_SIZE, 0);
    printf("len = %d\n",length);
    if (length <= 0)
    {
        printf("Recieve path from client %s failed!\n", clientIP);
        close(cfd);
        return NULL;
    }
    printf("\n%s target file is %s\n", clientIP, buffer);

    // 读入文件并传输
    FILE *stream;
    if ((stream = fopen(buffer, "r")) == NULL)
    {
        printf("the file was not opened\n");
        close(cfd);
        return NULL;
    }
    bzero(buffer, 1024);
    printf("正在传输...\n");
    int len = 0;
    // 不断读取并发送数据
    while ((len = fread(buffer, 1, 1024, stream)) > 0)
    {
        printf("len=%d\n", len);
        if (send(cfd, buffer, len, 0) < 0)
        {
            printf("send file error\n");
            break;
        }
        bzero(buffer, 1024);
    }
    if (fclose(stream))
    {
        printf("file close error\n");
    }

    // 关闭文件描述符
    close(cfd);
}

int main()
{
    // 1.创建socket(用于监听的套接字)
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1)
    {
        perror("socket");
        exit(-1);
    }
    // 2.绑定
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    // inet_pton(AF_INET, "192.168.56.103", saddr.sin_addr.s_addr);
    saddr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
    saddr.sin_port = htons(9999);
    int ret = bind(lfd, (struct sockaddr *)&saddr, sizeof(saddr));
    if (ret == -1)
    {
        perror("bind");
        exit(-1);
    }
    // 3.监听
    ret = listen(lfd, 8);
    if (ret == -1)
    {
        perror("listen");
        exit(-1);
    }
    // 4.接收客户端连接
    while (1)
    {
        struct sockaddr_in clientaddr;
        int len = sizeof(clientaddr);
        int cfd = accept(lfd, (struct sockaddr *)&clientaddr, &len);
        if (cfd == -1)
        {
            perror("accept");
            exit(-1);
        }
        struct client_addr addr;
        addr.cfd = cfd;
        addr.clientaddr = &clientaddr;
        pthread_t id;
        // 创建子线程
        ret = pthread_create(&id, NULL, (void *)client_method, (void *)&addr);
        if (ret != 0)
        {
            char *errstr = strerror(ret);
            printf("error : %s\n", errstr);
        }
    }
    return 0;
}