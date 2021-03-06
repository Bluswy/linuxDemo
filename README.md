# linuxDemo

## 主要内容

1．	Linux上的bash和Windows中的命令行有很大的不同。但是两者都有完成相似任务的命令，比如Linux上bash的ls命令的功能，类似于Windows命令行中的dir命令的功能。用C语言写一个简单的Linux终端软件，接收用户发出的类似于Windows命令行中的命令，转换成对应的Linux命令加以执行，并将执行的结果回显给用户。比如，用户输入“dir”，程序实际返回“ls”的内容。
2．	软件包含前、后台两个程序，用户启动前台程序时，前台程序自行启动后台程序。前台程序提供界面，负责接收用户输入，对输入进行转换，并向后台程序发出实际要执行的指令，后台负责执行实际的指令。

## 基本要求
1．	前台程序通过fork和execl系统调用启动后台程序。
2．	前台程序创建消息队列和命名管道，通过消息队列向后台程序发送经过转换的用户命令；通过命名管道从后台程序获取命令执行的结果，并显示在终端。后台程序可以通过popen来执行转换后的命令。
3．	至少实现如下Windows——Linux对照命令：dir——ls，rename——mv，move——mv，del——rm，cd——cd（pwd），exit——exit。
4．	当用户输入exit时，前台程序指示后台程序结束，在后台程序结束后，前台程序退出；在此之前，用户的输入都被作为一条命令进行处理。
