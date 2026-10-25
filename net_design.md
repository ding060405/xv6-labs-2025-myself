# net_design
## NIC
### background

### to do
    实现网卡收发
### idea
### structure
```cpp
char *tx_buf[]// transimit buffer
char *rx_buf[]//recieve buffer
```
### function
```cpp
// 实现转发，将buf中长度为len的内容交由网卡转发
int
e1000_transmit(char *buf, int len)
//实现接受，将网卡接受到的内容直接写入内存
static void
e1000_recv(void)
```
### mechanism
- 当前可用的buffer位置储存在reg[E100_RDT]
- 
### processor
#### transmit
    lock
    |
    v
    indentify finishing previous transmission
    status&E1000_TXD_STAT_DD!=0
    else return error
    |
    v
    buffer is empty?
    if no,free it
    |
    v
    put address of buf into tx_ring
    set tx_ring length as len
    set cmd 
    and set others to 0
    |
    v
    update descriptor tail int reg[E100_RDT]
#### recieve
    lock
    |
    v
    indentify wether finishing previous recieve
    status&E1000_RXD_STAT_DD!=0
    if no return err
    |
    v
    wait all packet arrive
    status&E1000_RXD_STAT_DD!=0
    |
    v
    allocate buffer
    |
    v
    put packet in buffer
    |
    v
    update discriptor tail
    |
    v
    release
    net_rx()
    lock
    |
    v
    release
### note
- run net_rx() after release lock
- acquire lock again after net_rx() return
## UDP recieve
### to do
    实现udp通信中的send,recieve,bind
### idea
- 创建端口结构
### struct 
buf
|
|-eth
|
|-ip
|
|-udp
|
|-content

```cpp
struct port{
  int port;
  int bound;
  struct packet pac[16];
  int head;
  int tail;
  int cnt;
};
#define PORTSIZE 32
struct port p[PORTSIZE];
```
### function
```cpp
void recv(short dport, int *src, short *sport, char *buf, int maxlen)

void send(short sport, int dst, short dport, char *buf, int len)

void bind(short port)
```
### processor
### ip_rx
    get argument
    |
    v
    is udp?
    yes
    |
    v
    destanition port is bind?
    yes
    |
    v
    put packet into waiting queue of port
    |
    v
    wait rev
### send
    get argument
    |
    v
    add file header to buf like struct
    |
    v
    invoke e1000_transimt()

#### rev
    get argument
    |
    v
    is greater than PGSIZE
    no
    |
    v
    is dport bind?
    port is same?
    yes
    |
    v
    waiting queue of port is empty?
    if yes ,sleep to waiting
    else 
    |
    v
    get one packet
    |
    v
    copyout to user mode
#### bind
    get port
    |
    v
    judege wether port has been init
    no
    |
    v
    init port
### note
- 注意网络储存形式为大端储存，收发均需要进行格式转换
- 收发的内容均来自于当前进程