#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

int main()
{
    //创建socket对象
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    //创建网络通信对象
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);
    addr.sin_addr.s_addr = inet_addr("192.168.56.103");

    //绑定socket对象
    int ret = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0)
    {
        printf("bind\n");
        return -1;
    }
    struct sockaddr_in cli;
    socklen_t len = sizeof(cli);

    //接收数据
    char buf[1024] = {0}, compare_str[1024]="";
    recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&cli, &len);
    printf("recv %s from %s\n", buf, inet_ntoa(cli.sin_addr));
    sprintf(compare_str, "%s", "From Function createPac");

    //比较数据内容
    if (strcmp(compare_str, buf) == 0)
    {
        printf("Recv data from function createPac\n");
    }
    else
    {
        printf("Recv data not from function createPac\n");
    }
    close(sockfd);
}