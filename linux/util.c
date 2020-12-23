#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

typedef struct
{
    long int msgType;
    char msgContent[256];
}MSG;

void sys_err(const char *str) {
    perror(str);
    exit(1);
}

char *loaduPath(char *pwd, char *filename) {
    char* path;
    path = (char*)malloc(1024 * sizeof(char));
    strcpy(path,pwd);
    strcat(path,"/");
    strcat(path,filename);
    return path;
}

/* 消息队列 */
void send_msg(char *content, char *APath) {
    key_t Key;
    int MsgId;
    MSG msg;

    // 生成键对值
    // 建立IPC通讯，必须指定一个Key
    // 将消息队列和一个键值关联，文件的索引节点和id关联
    if((Key = ftok (loaduPath(APath, "profile"), 2)) == -1) {
        sys_err("ftok error");
    }
    
    // 创建消息队列并获取IPC内核对象
    // IPC_CREAT 不存在key值的消息队列，则创建消息队列n，并返回消息队列ID
    // IPC_EXCL 存在返回-1
    // 权限为0666，所有用户可读可写
    if((MsgId = msgget(Key, IPC_CREAT|IPC_EXCL|0666)) == -1) {
        // 消息队列创建失败
        if(errno != EEXIST) {
            sys_err("fail to create a message queue");
        }
        // 存在消息队列
        if((MsgId = msgget(Key, 0)) == -1) {
            sys_err("message queue exists");
        }
    }

    // 初始化消息
    memset(&msg,0x00, sizeof(MSG));  
    // 对应消息队列子ID
    msg.msgType = 2;
    // 将linux命令输入消息中
    memcpy(msg.msgContent, content, strlen(content));
    // 发送消息到队列
    // 队列满时不阻塞，直接返回EAGAIN
    if(msgsnd(MsgId, (const void *)&msg, strlen(msg.msgContent), IPC_NOWAIT) <0) {
        perror("msgsnd");
    }
}

void receive_msg(char *rmsg, char *APath)
{
    key_t Key;
    int n, MsgId;
    MSG msg;

    if((Key = ftok (loaduPath(APath, "profile"), 2)) == -1) {
        sys_err("ftok error");
    }

    if((MsgId = msgget(Key, 0)) == -1) {
        sys_err("message exists error");
    }

    // 初始化消息
    memset(&msg, 0x00, sizeof(MSG));  
    // 阻塞的接受消息
    if((n = msgrcv(MsgId, (const void *)&msg, sizeof(msg.msgContent), 2, 0)) < 0) {
        perror("msgrcv error");
    }
    else {   
        strcpy(rmsg, msg.msgContent);
    }
}

/*  pipe */
// 命名管道通信不需要进程之间是父子关系
void create_pipe(char *pipeName) {
    if(access(pipeName, R_OK|W_OK) != -1) {
        return;
    }

    // 创建命名管道
    // 所有者读写权限，其它读权限
    if(mkfifo(pipeName, 0644) < 0) {
        sys_err("make fifo error");
    }
}

// 读取命名管道
void read_pipe(char *pipeName,char *echo)
{
    int fd; // file descriptor 文件描述符

    if((fd = open(pipeName, O_RDONLY, 0)) < 0) {
        // 管道打开失败
        sys_err("open error");
    }

    if(fd != -1) {
        // 阻塞的读取管道
        if(read(fd, echo, 1000) != -1) {
            return;
        }
    }

    close(fd);
    return;
}


void write_pipe(char *pipeName,char *content) {
    int fd;
    // 阻塞写
    if((fd = open(pipeName, O_WRONLY, 0)) < 0) {
        sys_err("open error");
    }

    if(fd != -1) {
        if(write(fd, content, 1000) == -1)
            return;
    }

    close(fd);
    return;
}