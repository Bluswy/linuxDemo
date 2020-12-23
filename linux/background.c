#include <stdio.h>
#include <string.h>
#include <unistd.h> 

// 执行shell命令
void executeCmd(const char *cmd, char *echo) {

    int lineNum = 0;
    char path[1024];
    // 清空
    strcpy(echo,"");
    // 执行cd和pwd命令
    if(strstr(cmd, "cd") != 0) {
        sscanf(cmd, "%*s%s", path);
        int error = chdir(path);
        if(error == -1) {
            strcpy(echo, "couldn't find the filepath");
        }
        return;
    }

    // 创建管道，调用fork产生子进程，执行shell运行command
    // 返回值连接标准输出
    FILE *pp = popen(cmd, "r");
    if (!pp) {
        return;
    }

    // 存储结果
    char tmp[1024]; 
    // 格式化输出
    while (fgets(tmp, sizeof(tmp), pp) != NULL) {
        // 去除换行符
        if (tmp[strlen(tmp) - 1] == '\n') {
            tmp[strlen(tmp) - 1] = '\0'; 
        }
        // 左对齐
        if(strlen(tmp) > 12) {
            sprintf(tmp, "%-24s", tmp);
            lineNum++;
        } else {
            sprintf(tmp, "%-12s", tmp); 
        }

        strcat(echo, tmp);

        if(++lineNum % 6 == 0) {
            strcat(echo, "\n");
        }
    }

    //关闭管道
    pclose(pp); 
    return;
}

char *concatPath(char *pwd, char *filename)
{
    char* path;
    path = (char*) malloc(1024 * sizeof(char));
    strcpy(path, pwd);
    strcat(path, "/");
    strcat(path, filename);
    return path;
}

void main()
{
    char echo[1000];
    char msgFromQueue[256];
    char syn[20];
    char absolutePath[1024];

    // 系统调用
    getcwd(absolutePath,1024); 

    while(1) {
        // 执行pwd命令返回工作路径
        executeCmd("pwd", &echo);
        // pwd结果写入管道
        write_pipe(concatPath(absolutePath,"pwd_pipe"), echo);
        // 同步
        read_pipe(concatPath(absolutePath,"syn_pipe"), &syn);
        // 接受消息队列中的命令
        receive_msg(&msgFromQueue, absolutePath);
        // 消息队列为空，无效消息
        if(strlen(msgFromQueue) == 0) {
            continue;
        }
        // 执行linux命令
        executeCmd(msgFromQueue, &echo);
        // 写入执行结果
        write_pipe(concatPath(absolutePath, "lx_pipe"), echo);
    }
}