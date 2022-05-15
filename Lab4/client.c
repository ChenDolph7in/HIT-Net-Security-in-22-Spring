#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <openssl/md5.h>
#include <openssl/des.h>

#define BUFFER_SIZE 1024

char *genRandomString(int length);
void md5hexToString(unsigned char *md, char *result);

int main()
{
	char username[18], passwd[18];
	printf("请输入用户名(最大18位):\n");
	scanf("%s", username);
	printf("请输入密码(最大18位):\n");
	scanf("%s", passwd);
	// 获得认证码
	char *cert_code = genRandomString(1024);
	printf("cert_code :%s\n", cert_code);

	// 计算散列值1
	unsigned char hash1[16] = {0};
	MD5_CTX md5;
	MD5_Init(&md5);
	char username_passwd[36] = {0};
	sprintf(username_passwd, "%s%s", username, passwd);
	MD5(username_passwd, sizeof(username_passwd), hash1);
	int i;

	char hash1_str[BUFFER_SIZE] = {0};
	md5hexToString(hash1, hash1_str);
	printf("加密前:%s %s\nhash1:%s\n", username, passwd, hash1_str);

	// 计算散列值2
	unsigned char hash2[16] = {0};
	MD5_Init(&md5);
	char temp_cert[BUFFER_SIZE * 2] = {0};
	sprintf(temp_cert, "%s%s", hash1_str, cert_code);
	MD5(temp_cert, sizeof(temp_cert), hash2);

	char hash2_str[BUFFER_SIZE] = {0};
	md5hexToString(hash2, hash2_str);
	printf("hash2:%s\n", hash2_str);
	// 连接服务器
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
	inet_pton(AF_INET, "192.168.56.103", &serveraddr.sin_addr.s_addr);
	serveraddr.sin_port = htons(9999);
	int ret = connect(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (ret == -1)
	{
		perror("connect");
		exit(-1);
	}
	// 3. 通信
	int data_len, flag = 3;
	char buffer[BUFFER_SIZE];
	send(fd, username, 18, 0);
	bzero(buffer, BUFFER_SIZE);
	data_len = recv(fd, buffer, BUFFER_SIZE, 0);
	if (data_len < 0)
	{
		printf("接收错误\n");
		exit(1);
	}
	if (strcmp(buffer, "GET SEQ1") == 0)
	{
		send(fd, cert_code, BUFFER_SIZE, 0);
		bzero(buffer, BUFFER_SIZE);
		data_len = recv(fd, buffer, BUFFER_SIZE, 0);
		if (data_len < 0)
		{
			printf("接收错误\n");
			exit(1);
		}
		if (strcmp(buffer, "GET SEQ2") == 0)
		{
			send(fd, hash2_str, BUFFER_SIZE + 36, 0);
			bzero(buffer, BUFFER_SIZE);
			data_len = recv(fd, buffer, BUFFER_SIZE + 36, 0);
			if (data_len < 0)
			{
				printf("接收错误\n");
				exit(1);
			}
			if (strcmp(buffer, "GET SEQ3") == 0)
			{
				unsigned char *keystring = hash1_str;
				DES_cblock key;
				DES_key_schedule key_schedule;

				//生成一个 key
				DES_string_to_key(keystring, &key);
				if (DES_set_key_checked(&key, &key_schedule) != 0)
				{
					printf("convert to key_schedule failed.\n");
					return -1;
				}

				//需要加密的字符串
				unsigned char input[BUFFER_SIZE] = {0};
				size_t len = (BUFFER_SIZE + 7) / 8 * 8;
				unsigned char *output = malloc(len + 1);
				bzero(output, len + 1);
				data_len = recv(fd, output, len + 1, 0);
				printf("output:%s\n", output);
				if (data_len < 0)
				{
					printf("接收错误\n");
					exit(1);
				}
				else
				{
					// IV
					DES_cblock ivec;

					// IV设置为0x0000000000000000
					memset((char *)&ivec, 0, sizeof(ivec));

					//解密
					//printf("len:%ld\n", len);
					DES_ncbc_encrypt(output, input, len, &key_schedule, &ivec, 0);

					printf("%s\n", input);
					if (strcmp(cert_code, input) == 0)
					{
						printf("server certification success\n");
						FILE *f;
						if ((f = fopen("client_result.txt", "w")) == NULL)
						{
							printf("file open error\n");
							exit(1);
						}
						setbuf(f,NULL);
						fprintf(f,"%s",cert_code);
					}
					else
					{
						printf("server certification fail\n");
					}

					free(output);
				}
			}
		}
	}

	// 关闭连接
	close(fd);
	return 0;
}

//产生长度为length的随机字符串
char *genRandomString(int length)
{
	int flag, i;
	char *string;
	srand((unsigned)time(NULL));
	if ((string = (char *)malloc(length)) == NULL)
	{
		perror("Malloc failed!flag:14\n");
		return NULL;
	}

	for (i = 0; i < length - 1; i++)
	{
		flag = rand() % 3;
		switch (flag)
		{
		case 0:
			string[i] = 'A' + rand() % 26;
			break;
		case 1:
			string[i] = 'a' + rand() % 26;
			break;
		case 2:
			string[i] = '0' + rand() % 10;
			break;
		default:
			string[i] = 'x';
			break;
		}
	}
	string[length - 1] = '\0';
	return string;
}

void md5hexToString(unsigned char *md, char *result)
{
	// char tmp[3];
	for (size_t i = 0; i <= 15; i++)
	{
		sprintf(result + i * 2, "%02x", md[i]);
	}
	return;
}
