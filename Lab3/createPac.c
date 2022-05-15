#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libnet.h>

int main(int argc, char *argv[])
{
    char send_msg[1000] = "";
    char err_buf[100] = "";
    libnet_t *lib_net = NULL;
    int lens = 0;
    libnet_ptag_t lib_t = 0;
    unsigned char src_mac[6] = {0x08, 0x00, 0x27, 0x18, 0xb1, 0x43}; //发送者网卡地址08:00:27:18:b1:43
    unsigned char dst_mac[6] = {0x08, 0x00, 0x27, 0xad, 0xdb, 0x8b}; //接收者网卡地址08-00-27-ad-db-8b
    char *src_ip_str = "192.168.56.103";                             //源主机IP地址
    char *dst_ip_str = "192.168.56.104";                             //目的主机IP地址
    unsigned long src_ip, dst_ip = 0;

    lens = sprintf(send_msg, "%s", "From Function createPac"); // udp负载内容

    lib_net = libnet_init(LIBNET_LINK_ADV, "192.168.56.103", err_buf); //初始化
    if (NULL == lib_net)
    {
        perror("libnet_init");
        exit(-1);
    }

    src_ip = libnet_name2addr4(lib_net, src_ip_str, LIBNET_RESOLVE); //将字符串类型的ip转换为顺序网络字节流
    dst_ip = libnet_name2addr4(lib_net, dst_ip_str, LIBNET_RESOLVE); //将字符串类型的ip转换为顺序网络字节流

    //构造udp数据包
    lib_t = libnet_build_udp(
        9999,     //源端口号
        9999,     //目的端口号
        8 + lens, // udp数据包总长度=头部长度+负载长度
        0,        //校验和，设为 0，libnet 自动填充
        send_msg, //负载
        lens,     //负载长度
        lib_net,  // libnet 句柄
        0         //协议标记，第一次组新的发送包时，这里写 0，同一个应用程序，下一次再组包时，这个位置的值写此函数的返回值
    );

    //构造ip数据包
    lib_t = libnet_build_ipv4(
        20 + 8 + lens, // ip 包总长
        0,             //服务类型
        500,           // ip 标识
        0,             //片偏移
        10,            //生存时间
        17,            //上层协议
        0,             //校验和，设为 0，libnet 自动填充
        src_ip,        //源 ip 地址
        dst_ip,        //目的ip地址
        NULL,          //负载
        0,             //负载长度
        lib_net,       // libnet 句柄
        0              //协议标记，第一次组新的发送包时，这里写 0，同一个应用程序，下一次再组包时，这个位置的值写此函数的返回值。
    );

    //构造以太网数据包
    lib_t = libnet_build_ethernet(
        (u_int8_t *)dst_mac, //目的 mac
        (u_int8_t *)src_mac, //源 mac
        0x800,               // 上层协议类型,或者ETHERTYPE_IP
        NULL,                //负载
        0,                   //负载长度
        lib_net,             // libnet 句柄
        0                    //协议标记，第一次组新的发送包时，这里写 0，同一个应用程序，下一次再组包时，这个位置的值写此函数的返回值。
    );
    int res = 0;
    res = libnet_write(lib_net); //发送数据包
    if (-1 == res)
    {
        perror("libnet_write");
        exit(-1);
    }

    libnet_destroy(lib_net); //销毁资源

    printf("----send finished-----\n");
    return 0;
}