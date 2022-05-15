// 传输的服务器端
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "mysql.h"
#include <openssl/md5.h>
#include <openssl/des.h>

#define BUFFER_SIZE 1024

// 子线程处理函数参数
struct client_addr
{
    int cfd;
    struct sockaddr_in *clientaddr;
};

void md5hexToString(unsigned char *md, char *result)
{
    // char tmp[3];
    for (size_t i = 0; i <= 15; i++)
    {
        sprintf(result + i * 2, "%02x", md[i]);
    }
    return;
}

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
    char buffer[BUFFER_SIZE] = {0};
    int length = recv(cfd, buffer, BUFFER_SIZE, 0);
    char username[18] = {0}, hash2[BUFFER_SIZE] = {0}, cert_code[BUFFER_SIZE] = {0};
    if (length <= 0)
    {
        close(cfd);
        return NULL;
    }
    else
    {
        strcpy(username, buffer);
        printf("recv username: %s\n", username);
        strcpy(buffer, "GET SEQ1");
        send(cfd, buffer, BUFFER_SIZE, 0);
        bzero(buffer, BUFFER_SIZE);
        int length = recv(cfd, buffer, BUFFER_SIZE, 0);
        if (length <= 0)
        {
            close(cfd);
            return NULL;
        }
        else
        {
            strcpy(cert_code, buffer);
            printf("cert_code :%s\n", cert_code);
            strcpy(buffer, "GET SEQ2");
            send(cfd, buffer, BUFFER_SIZE, 0);
            bzero(buffer, BUFFER_SIZE);
            int length = recv(cfd, buffer, BUFFER_SIZE, 0);
            if (length <= 0)
            {
                close(cfd);
                return NULL;
            }
            else
            {
                strcpy(hash2, buffer);
                printf("recv hash2: %s\n", hash2);
                strcpy(buffer, "GET SEQ3");
                send(cfd, buffer, BUFFER_SIZE, 0);
                bzero(buffer, BUFFER_SIZE);
                int length = recv(cfd, buffer, BUFFER_SIZE, 0);
                if (length <= 0)
                {
                    close(cfd);
                    return NULL;
                }
                else
                {
                    MYSQL m_conn;
                    int ret;

                    mysql_init(&m_conn);
                    if (mysql_real_connect(&m_conn, "localhost", "root", "root", "NSLAB4", 0, NULL, 0))
                    {
                        MYSQL_ROW row;
                        MYSQL_RES *res;
                        int t;
                        char *allData = (char *)malloc(sizeof(char) * 62);
                        sprintf(allData, "Select hash1 From Users Where username = '%s'", username);
                        //printf("select str : %s\n", allData);
                        mysql_real_query(&m_conn, allData, (unsigned int)strlen(allData));

                        res = mysql_store_result(&m_conn);
                        row = mysql_fetch_row(res);
                        if (row != NULL)
                        {
                            char hash1_str[BUFFER_SIZE] = {0};
                            unsigned char hash3[16] = {0};
                            strcpy(hash1_str, row[0]);
                            printf("hash1:%s\n", hash1_str);
                            MD5_CTX md5;
                            MD5_Init(&md5);
                            char temp_cert[BUFFER_SIZE * 2] = {0};
                            sprintf(temp_cert, "%s%s", hash1_str, cert_code);
                            MD5(temp_cert, sizeof(temp_cert), hash3);
                            char hash3_str[BUFFER_SIZE] = {0};
                            md5hexToString(hash3, hash3_str);
                            if (strcmp(hash3_str, hash2) == 0)
                            {
                                printf("client certification success\n");
                                unsigned char *keystring = hash1_str;
                                DES_cblock key;
                                DES_key_schedule key_schedule;

                                //生成一个 key
                                DES_string_to_key(keystring, &key);
                                if (DES_set_key_checked(&key, &key_schedule) != 0)
                                {
                                    printf("convert to key_schedule failed.\n");
                                    return NULL;
                                }

                                //需要加密的字符串
                                unsigned char input[BUFFER_SIZE] = {0};
                                strcpy((char *)input, cert_code);
                                size_t len = (BUFFER_SIZE + 7) / 8 * 8;
                                unsigned char *output = malloc(len + 1);
                                bzero(output, len + 1);
                                // IV
                                DES_cblock ivec;

                                // IV设置为0x0000000000000000
                                memset((char *)&ivec, 0, sizeof(ivec));

                                //加密
                                DES_ncbc_encrypt(input, output, BUFFER_SIZE, &key_schedule, &ivec, DES_ENCRYPT);
                                printf("output:%s\n", output);
                                printf("len:%ld\n",len);
                                send(cfd, (char *)output, len + 1, 0);

                                printf("%s\n", input);
                                free(output);
                            }
                            else
                            {
                                printf("client certification fail\n");
                                bzero(buffer, BUFFER_SIZE);
                                strcpy(buffer, "passwd wrong");
                                send(cfd, buffer, BUFFER_SIZE, 0);
                            }
                        }

                        mysql_close(&m_conn);
                    }
                }
            }
        }

        // 关闭文件描述符
        close(cfd);
    }
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