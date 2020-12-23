#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h> // linux/unix系统调用
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

static int cmdNum = 6;
// windoes和linux指令对应关系
char **winCmds[6] = {"dir","rename","move","del","cd","exit"};
char **linuxCmds[6] = {"ls","mv","mv","rm","cd","exit"};

char *winInLinux(char *winCmd) {
    int i;
    char *temp;
    char *cmd;  // 命令
    char *args; // 命令参数

    temp = (char*) malloc(20 * sizeof(char));
    cmd = (char*) malloc(20 * sizeof(char));
    args = (char*) malloc(20 * sizeof(char));

    // 格式化读取命令及其参数
    sscanf(winCmd, "%s%[^'\n']", cmd, args);
    // 比较字符串，如果相等返回0
    // 命令为cd命令且无参数，直接返回pwd命令，即显示当前工作目录的绝对路径
    if(!strcmp(cmd, "cd") && strlen(args) == 0) {
        // 避免内存泄漏
        free(temp);
        free(cmd);
        free(args);
        return "pwd";
    }

    else if(!strcmp(cmd, "cd") && strlen(args) != 0) {
        // 带参数的cd命令
        // 改变当前工作目录
    }
    for(i = 0; i < cmdNum; i++) {
        if(strstr(cmd, winCmds[i]) != 0) {
            // 找到windows命令对应的linux命令
            strcpy(temp, linuxCmds[i]);
            strcat(temp, args); // concatenate 命令和参数
            free(cmd);
            free(args);
            return temp;
        }
    }
    return "";
}

// 根据当前目录和filename生成系统路径
char *concatAPath(char *workPath, char *filename) {
    char* path;
    path = (char*) malloc(1024 * sizeof(char));
    strcpy(path, workPath);
    strcat(path, "/");
    strcat(path, filename);
    return path;
}

void main() {
    pid_t pid;
    char *winCmd;
    char *linuxCmd;

    winCmd = (char*) malloc(20 * sizeof(char));
    linuxCmd = (char*) malloc(20 * sizeof(char));

    char echo[1024];
    char workPath[1024];      // pwd缓冲区
    char absolutePath[1024]; // 保存当前目录的缓冲区

    getcwd(absolutePath,1024); // 系统调用，获得当前工作目录
    // 根据filename创建管道
    create_pipe(concatAPath(absolutePath, "lx_pipe"));
    create_pipe(concatAPath(absolutePath, "syn_pipe"));
    create_pipe(concatAPath(absolutePath, "pwd_pipe"));

    // 忽略SIGCLD，当子进程快结束，发出SIGCLD信号通知父进程调用wait回收子进程，但wait为阻塞调用，
    // 影响性能。忽略SIGCLD信号，由init进程回收子进程。
    // 系统调用 忽略SIGCLD信号
    signal(SIGCLD, SIG_IGN);

    // fork进程 
    switch(pid=fork()) {
        // 进程fork出错
        case -1:
            perror("fork error");
            break;
        // 子进程 
        case 0: {      
            // 执行后台文件，执行后台的代码逻辑
            if(execl(concatAPath(absolutePath, "background"), NULL)== -1) {
                // 执行出错
                sys_err("execl error");
            }
            break;
        }
        // 父进程
        default: { 
            while(1) {
                // 读取管道, 输出当前工作目录
                read_pipe(concatAPath(absolutePath, "pwd_pipe"), &workPath);
                // 读取格式化字符串，读取到空格为止
                sscanf(workPath,"%[^ ]", workPath);
                printf("%s:", workPath);

                // 从标准输入读取一行
                gets(winCmd);

                // 读取到exit命令，父进程退出
                if(strcmp(winCmd,"exit") == 0) {
                    // 杀死子进程，交由init进程wait
                    // 避免内存泄漏
                    free(winCmd);
                    free(linuxCmd);
                    kill(pid, SIGKILL);
                    exit(0);
                }

                // 获得windoes命令对应的linux命令
                linuxCmd = winInLinux(winCmd);
                // 管道同步，获取到命令后子进程才能执行
                write_pipe(concatAPath(absolutePath, "syn_pipe"), "Start Send");

                if(strlen(linuxCmd) == 0) {
                  printf("Invalid command, please check your command list.\n");
                  // 无效命令，通过消息队列同步通知后台进程
                  send_msg(linuxCmd, absolutePath);
                  continue;
                }

                // 发送linux命令到消息队列中
                send_msg(linuxCmd, absolutePath);
                // 读取管道，输出命令执行结果
                read_pipe(concatAPath(absolutePath, "lx_pipe"), &echo);
                printf("%s\n", echo);
            }
            break;
        }
    }
}