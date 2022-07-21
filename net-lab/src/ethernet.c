#include "ethernet.h"
#include "utils.h"
#include "driver.h"
#include "arp.h"
#include "ip.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief 处理一个收到的数据包
 *        你需要判断以太网数据帧的协议类型，注意大小端转换
 *        如果是ARP协议数据包，则去掉以太网包头，发送到arp层处理arp_in()
 *        如果是IP协议数据包，则去掉以太网包头，发送到IP层处理ip_in()
 * 
 * @param buf 要处理的数据包
 */
void ethernet_in(buf_t *buf)
{
    // get header
    ether_hdr_t *header = (ether_hdr_t *)buf->data;
    uint16_t temp_protocal = swap16(header->protocol);
    buf_remove_header(buf, sizeof(ether_hdr_t));
    if (temp_protocal == NET_PROTOCOL_ARP){
        arp_in(buf);
    } else if (temp_protocal == NET_PROTOCOL_IP){
        ip_in(buf);
    } else {
        printf("fail\n");
    }
}

/**
 * @brief 处理一个要发送的数据包
 *        你需添加以太网包头，填写目的MAC地址、源MAC地址、协议类型
 *        添加完成后将以太网数据帧发送到驱动层
 * 
 * @param buf 要处理的数据包
 * @param mac 目标ip地址
 * @param protocol 上层协议
 */
void ethernet_out(buf_t *buf, const uint8_t *mac, net_protocol_t protocol)
{
    // add a length to the head
    buf_add_header(buf, sizeof(ether_hdr_t));
    // get header
    ether_hdr_t *headr = (ether_hdr_t *) buf->data;
    memcpy(headr->dest, mac, NET_MAC_LEN);
    net_protocol_t temp = swap16(protocol);
    headr->protocol = temp;
    memcpy(headr->src, net_if_mac, NET_MAC_LEN);
    if(driver_send(buf) == -1){
        printf("send fail\n");
    }
}

/**
 * @brief 初始化以太网协议
 * 
 * @return int 成功为0，失败为-1
 */
int ethernet_init()
{
    buf_init(&rxbuf, ETHERNET_MTU + sizeof(ether_hdr_t));
    return driver_open();
}

/**
 * @brief 一次以太网轮询
 * 
 */
void ethernet_poll()
{
    if (driver_recv(&rxbuf) > 0)
        ethernet_in(&rxbuf);
}
