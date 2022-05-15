#include <pcap.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define ETHERNET_ADDR_LEN 6
#define IP_ADDR_LEN 4

// 回调函数参数
struct userarg
{
    int *id;      //捕获包ID，从1开始计数
    FILE *stream; //保存数据文件句柄
};

// 以太网数据帧头部结构体
struct ethernet
{
    u_char eth_dsthost[ETHERNET_ADDR_LEN]; //以太网MAC目的地址
    u_char eth_srchost[ETHERNET_ADDR_LEN]; //以太网MAC源地址
    u_short eth_type;                      //协议类型
};

// IPv4报文头部结构体
struct ip
{
    u_char ip_hlv;              //版本号+头部长度
    u_char ip_tos;              //区分服务
    u_short ip_len;             // IP数据报长度
    u_short ip_id;              //标识
    u_short ip_foff;            //标志3位+片偏移13位
    u_char ip_ttl;              //生存时间
    u_char ip_pro;              //协议
    u_short ip_checksum;        //首部校验和
    u_char ip_src[IP_ADDR_LEN]; //源IP地址
    u_char ip_dst[IP_ADDR_LEN]; //目的IP地址
};

// TCP报文头
struct tcp
{
    u_short tcp_srcport;  //源端口号
    u_short tcp_dstport;  //目的端口号
    u_int tcp_seq;        //序列号
    u_int tcp_ack;        //确认号
    u_char tcp_headlen;   // 4位头部长度+4位保留
    u_char tcp_flag;      // 2保留位+6位标志位
    u_short tcp_win;      //窗口大小
    u_short tcp_checksum; //校验和
    u_short tcp_urp;      //紧急指针
};

// UDP报文头部结构体
struct udp
{
    u_short udp_srcport;  //源端口号
    u_short udp_dstport;  //目的端口号
    u_short udp_len;      //总长度
    u_short udp_checksum; //校验和
};

char *getTime();
void getPacket(u_char *arg, const struct pcap_pkthdr *pkthdr, const u_char *packet);
void parse_ethernet(struct ethernet *myEth, const u_char *packet);
int parse_ip(struct ip *myIp, const u_char *packet);
void parse_tcp(struct tcp *myTcp, const u_char *packet);
void parse_udp(struct udp *myUdp, const u_char *packet);

int main(int argc, char *argv[])
{
    /* 根据选项判断是否需要过滤，若没有选项，默认不过滤 */
    char *filter_str = NULL;
    if (argc >= 2)
    {
        /* 当调用-f选项时，将-f后字符串参数当作[过滤规则] */
        if (argc == 3 && strcmp(argv[1], "-f") == 0)
        {
            filter_str = argv[2];
        }
        /* 否则打印帮助信息 */
        else
        {
            printf("用法: pcap -f [过滤规则]   (缺省-f默认不过滤)\n");
            exit(0);
        }
    }
    pcap_if_t *alldevs;
    pcap_t *device;
    char errbuf[PCAP_ERRBUF_SIZE];

    /* 获得网卡的列表 */
    if (pcap_findalldevs(&alldevs, errbuf) == -1)
    {
        fprintf(stderr, "pcap_findalldevs error: %s\n", errbuf);
        exit(1);
    }

    pcap_if_t *d;
    int i = 0;
    /* 打印网卡信息 */
    for (d = alldevs; d; d = d->next)
    {
        printf("[%d]%s", ++i, d->name);
        if (d->description)
            printf(" (%s)\n", d->description);
        else
            printf(" (no description)\n");
    }

    if (i == 0)
    {
        printf("\nNo interfaces found!\n");
        return -1;
    }

    int interface_num;
    printf("Enter the number of interface (1-%d) or 0 to exit:", i);
    scanf("%d", &interface_num); //输入要选择打开的网卡号

    if (interface_num == 0)
    {
        printf("\nReady to exit.\n");
        /* 释放网卡列表 */
        pcap_freealldevs(alldevs);
        return -1;
    }
    else if (interface_num < 1 || interface_num > i) //判断号的合法性
    {
        printf("\nNumber not exist!\n");
        /* 释放网卡列表 */
        pcap_freealldevs(alldevs);
        return -1;
    }

    /* 找到要选择的网卡结构 */
    for (d = alldevs, i = 0; i < interface_num - 1; d = d->next, i++)
        ;

    /* 打开选择的网卡 */
    if ((device = pcap_open_live(d->name, // 设备名称
                                 65536,   // portion of the packet to capture.
                                 // 65536 grants that the whole packet will be captured on all the MACs.
                                 0,     // 非混杂模式
                                 1000,  // 读超时为1秒
                                 errbuf // error buffer
                                 )) == NULL)
    {

        fprintf(stderr, "\nUnable to open the adapter. %s is not supported byWinPcap\n", d->name);
        /* 释放网卡列表 */
        pcap_freealldevs(alldevs);
        return -1;
    }

    printf("\nlistening on %s...\n", d->description);

    /* 释放网卡列表 */
    pcap_freealldevs(alldevs);

    /* 创建输出流*/
    FILE *stream;
    char *filename = (char *)malloc(32);
    bzero(filename, 32);
    filename = strcat(filename, "capture_");
    filename = strcat(filename, getTime());
    filename = strcat(filename, ".txt");
    printf("filename:%s\n", filename);
    if ((stream = fopen(filename, "w")) == NULL)
    {
        printf("file open error\n");
        exit(1);
    }
    setbuf(stream,NULL);

    /* construct a filter */
    if (filter_str != NULL)
    {
        struct bpf_program filter;
        pcap_compile(device, &filter, filter_str, 1, 0);
        pcap_setfilter(device, &filter);
        fprintf(stream, "过滤规则: %s\n", filter_str);
    }
    /* wait loop forever */
    int id = 0;
    struct userarg arg;
    arg.id = &id;
    arg.stream = stream;
    pcap_loop(device, -1, getPacket, (u_char *)&arg);
    pcap_close(device);
    fclose(stream);
    return 0;
}

/*
    获得作为输出文件名后半部分的当前时间格式串
*/
char *getTime()
{
    char *timeStr = (char *)malloc(20);
    bzero(timeStr, 20);
    time_t iNowTime = 0;
    struct tm *pstNowTransfer = NULL;

    iNowTime = time(NULL);
    pstNowTransfer = localtime(&iNowTime);

    // 按格式输出时间
    strftime(timeStr, 20, "%Y_%m_%d_%H_%M_%S", pstNowTransfer);

    return timeStr;
}

/*
    捕获包回调函数；
*/
void getPacket(u_char *arg, const struct pcap_pkthdr *pkthdr, const u_char *packet)
{
    struct userarg *myArg = (struct userarg *)arg;
    int *id = myArg->id;
    FILE *stream = myArg->stream;
    *id = 1 + (*id);
    fprintf(stream, "id: %d\n", *id);
    fprintf(stream, "Recieved time: %s", ctime((const time_t *)&pkthdr->ts.tv_sec));
    /* 终端打印 */
    // printf("\nid: %d\n", ++(*id));
    // printf("Packet length: %d\n", pkthdr->len);
    // printf("Number of bytes: %d\n", pkthdr->caplen);
    // printf("Recieved time: %s", ctime((const time_t *)&pkthdr->ts.tv_sec));
    // int i;
    // for (i = 0; i < pkthdr->len; ++i)
    // {
    //     printf(" %02x", packet[i]);
    //     if ((i + 1) % 16 == 0)
    //     {
    //         printf("\n");
    //     }
    // }
    struct ethernet myEth;
    struct ip myIp;
    // 1.分析以太网数据帧头
    parse_ethernet(&myEth, packet);
    fprintf(stream, "src_mac: ");
    for (int i = 0; i < 6; i++)
    {
        fprintf(stream, " %02x", myEth.eth_srchost[i]);
    }
    fprintf(stream, "\ndst_mac: ");
    for (int i = 0; i < 6; i++)
    {
        fprintf(stream, " %02x", myEth.eth_dsthost[i]);
    }
    // 2.分析IPv4数据报头，并解析得到传输层协议
    int trans_pro = parse_ip(&myIp, packet);
    char ipv4_str[64] = {'\0'};
    fprintf(stream, "\nsrc_ip:");
    for (int i = 0; i < 4; i++)
    {
        fprintf(stream, " %02x", myIp.ip_src[i]);
    }
    // 得到IP地址对应点分十进制表示串
    inet_ntop(AF_INET, &myIp.ip_src, ipv4_str, 64);
    fprintf(stream, "(%s)\ndst_ip:", ipv4_str);
    for (int i = 0; i < 4; i++)
    {
        fprintf(stream, " %02x", myIp.ip_dst[i]);
    }
    // 得到IP地址对应点分十进制表示串
    inet_ntop(AF_INET, &myIp.ip_dst, ipv4_str, 64);
    fprintf(stream, "(%s)", ipv4_str);
    // 3.根据传输层协议选择分析TCP报文头或UDP报文头
    if (trans_pro == 1)
    {
        struct tcp myTcp;
        parse_tcp(&myTcp, packet);
        fprintf(stream, "\nsrc_port: %u", myTcp.tcp_srcport);
        fprintf(stream, "\ndst_port: %u\n\n", myTcp.tcp_dstport);
    }
    else
    {
        struct udp myUdp;
        parse_udp(&myUdp, packet);
        fprintf(stream, "\nsrc_port: %u", myUdp.udp_srcport);
        fprintf(stream, "\ndst_port: %u\n\n", myUdp.udp_dstport);
    }

}

/* 分析以太网帧头 */
void parse_ethernet(struct ethernet *myEth, const u_char *packet)
{
    // 没有六字节类型，单个字节传递MAC地址
    int i = 0;
    for (i = 0; i < 6; i++)
    {
        myEth->eth_dsthost[i] = packet[i];
    }
    for (; i < 12; i++)
    {
        myEth->eth_srchost[i - 6] = packet[i];
    }
    myEth->eth_type = ntohs(*(u_short *)(packet + 12));
}

/* 分析IP数据报头 */
int parse_ip(struct ip *myIp, const u_char *packet)
{
    myIp->ip_hlv = packet[14];
    myIp->ip_tos = packet[15];
    myIp->ip_len = ntohs(*(u_short *)(packet + 16));
    myIp->ip_id = ntohs(*(u_short *)(packet + 18));
    myIp->ip_foff = ntohs(*(u_short *)(packet + 20));
    myIp->ip_ttl = packet[22];
    myIp->ip_pro = packet[23];
    myIp->ip_checksum = ntohs(*(u_short *)(packet + 24));
    // inet_ntop自带转换字节序，这里暂时不需要转换字节序
    *(unsigned int *)myIp->ip_src = *(unsigned int *)(packet + 26);
    *(unsigned int *)myIp->ip_dst = *(unsigned int *)(packet + 30);
    // 判断协议类型
    if (myIp->ip_pro == 0x08)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/* 分析TCP报文头 */
void parse_tcp(struct tcp *myTcp, const u_char *packet)
{
    int tcp_start = 14 + 4 * (packet[15] & 0x0f);
    myTcp->tcp_srcport = ntohs(*(u_short *)(packet + tcp_start));
    myTcp->tcp_dstport = ntohs(*(u_short *)(packet + tcp_start + 2));
    myTcp->tcp_seq = ntohl(*(u_int *)(packet + tcp_start + 4));
    myTcp->tcp_ack = ntohl(*(u_int *)(packet + tcp_start + 8));
    myTcp->tcp_headlen = packet[tcp_start + 12];
    myTcp->tcp_flag = packet[tcp_start + 13];
    myTcp->tcp_win = ntohs(*(u_short *)(packet + tcp_start + 14));
    myTcp->tcp_checksum = ntohs(*(u_short *)(packet + tcp_start + 16));
    myTcp->tcp_urp = ntohs(*(u_short *)(packet + tcp_start + 18));
}

/* 分析UDP报文头 */
void parse_udp(struct udp *myUdp, const u_char *packet)
{
    int udp_start = 14 + 4 * (packet[15] & 0x0f);
    myUdp->udp_srcport = ntohs(*(u_short *)(packet + udp_start));
    myUdp->udp_dstport = ntohs(*(u_short *)(packet + udp_start + 2));
    myUdp->udp_len = ntohs(*(u_short *)(packet + udp_start + 4));
    myUdp->udp_checksum = ntohs(*(u_short *)(packet + udp_start + 6));
}