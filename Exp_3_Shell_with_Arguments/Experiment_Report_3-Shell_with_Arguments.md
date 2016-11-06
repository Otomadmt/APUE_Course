## UNIX系统环境编程课程实验报告

> 实验 3 - 简单 Shell 的设计与实现
>
> 报告完成时间: 5th Nov, 2016



##### 1) 实验内容

在课本 (APUE.2e) 程序 1-5 的基础上编写程序 myshell，要求实现允许输入带多个参数命令的简单 Shell. 其中 exec() 函数只允许使用系统调用 execve().

参数限制在一行内，且不考虑通配符处理 (*, ?, -, etc.). 命令出错时作出相应提示并进入下一轮接受状态，直到输入 Ctrl+C 或 Ctrl + \ 结束运行。

##### 2) 实现分析

本次实验基于课本程序 1-5 的基础，作出以下修改与补充：

1. exec() 函数使用系统调用 execve(), 其原型为 int execve(const char *path, const char *argv[], const char *envp[]).

   其中 path 为程序路径名， argv[] 为参数表，envp[] 为环境变量表。execve() 函数在运行出错时将设置 errno 变量并返回，否则 execve() 将使用 path 所指向的新程序替换当前程序正文段、数据段、堆栈与栈段。

2. 支持带参命令。通过向 execve() 函数传递 argv[] 可以实现参数传递。因实验不要求处理通配符，argv[] 可从程序读入的命令中按空格拆分即可得出。

3. 支持环境变量，以便用户直接输入系统环境变量中的程序名来执行命令，无需完全写出路径。通过外部参数 extern char **environ 可取得系统的环境变量，同时 execve() 函数原型支持传入环境变量表 envp[], 并以此传入的变量表作为所调用的程序的环境变量，这样便实现环境变量支持。


具体实现与注释见源代码 myshell.c .

##### 3) 实现 fork()、带参命令与环境变量支持的注意事项

+ fork() 将创建一份与父程序完全相同且共享内存子段的子程序，故应在源程序中利用 fork() 返回的 pid 值来区分父／子程序以实现不同的行为。此外，在创建子程序后父程序应该调用 waitpid() 以接受子程序返回的信号，在实现同步调度的同时避免子程序成为僵尸进程。


+ 从输入的命令字符串 buf 中分割出参数：

  使用 string.h 中的 strsep() 函数可以方便地指定分隔符 (delimiter) 并实现对字符串的分割。但需要注意：

  1. strsep() 函数将把所有分隔符替换为 '\0'，改变原字符串。故应该使用 strdup() 复制一个原字符串供 strsep() 使用，此外在使用完 strdup() 复制的字符串后应 free() 之。
  2. 在遇到多个分隔符连续的情况下，strsep() 将返回空串。未避免空串被作为参数传入 argv[], 应使用 strcmp() 函数检查 strsep() 的返回结果。

+ 环境变量的支持：

  子程序可以通过 getenv("PATH") 来获取运行环境的环境变量。当用户输入的命令给出了带路径的文件地址时，程序可直接将该地址当作 path 变量传给 execve() 函数；否则，当输入的命令只给定了文件名时，应该使用 ':' 为分隔符分割环境变量，并将输入文件名附在各个环境变量中的路径后当作 path 传给 execve() 函数尝试执行，直到在某一路径下找到该文件成功执行或者找不到出错返回为止。 

##### 4) 实验结果

在 CentOS Release 5.1.1 环境下采用 gcc 4.1.2 进行编译，工作目录为 /home/cs14/csunix/homework/3 .

交互输入与输出内容：

```Bash
% ls
\  apue.h  error.c  myshell  myshell.c
% /bin/ls -al
total 72
drwxr-xr-x 2 cs14 teacher  4096 Nov  7 00:51 .
drwxr-xr-x 5 cs14 teacher  4096 Nov  1 13:50 ..
-rw-r--r-- 1 cs14 teacher   167 Nov  1 14:12 \
-rw-r--r-- 1 cs14 teacher  4821 Nov  1 14:03 apue.h
-rw-r--r-- 1 cs14 teacher  2021 Nov  1 14:02 error.c
-rwxr-xr-x 1 cs14 teacher 12896 Nov  7 00:51 myshell
-rw-r--r-- 1 cs14 teacher  1871 Nov  7 00:51 myshell.c
%    who  am    i
cs14 pts/2        Nov  7 00:50 (13.80.0.138/000)
% ./myshell
%  /usr/ls
exec fail: /usr/ls
: No such file or directory
% Quit
```

##### 5) 实验体会

本次实验内容较为简单，但是实验内容涵盖了 fork()、exec()、exit()、waitpid() 等 Unix 进程控制原语以及信号机制等重要内容，通过这些实验我们可以进一步了解到 Unix 系统中程序的组成与构造，理解一个程序运行的整体流程与资源调度过程，初步认识了环境变量与程序的关系，这些内容都为今后学习 Unix 系统更为底层的机制以及操作系统的原理、实现等课题打下了相应基础。
