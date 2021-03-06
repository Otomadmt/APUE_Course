## UNIX系统环境编程课程实验报告

> 实验1 - 同步与异步 write 的效率比较
>
> 报告完成时间: 10th Oct, 2016



##### 1) 实验内容

编写程序测试 UNIX 系统同步与异步 write 的效率区别，掌握文件 I/O 系统调用。

输入： timewrite <outfile> [sync]

e.g.	timewrite fout <fin

​	timewrite fout sync <fin

​	当参数 sync 存在时，用 O_SYNC 标记打开文件。

输出： 参照书本 (APUE.2e) Fig. 3.2 的结果，其中 BUFFER SIZE 的大小从256开始到128K，省略表头与分割线。

##### 2) 实现分析

本次实验程序可能接受1-2个参数，执行时需要检查参数个数和参数内容是否符合要求。此外，实验主要目的是要统计 write 操作所使用的时间，应尽可能忽略程序运行时其他部分所耗费的时间，尤其是从硬盘读入文件的时间。

因而，程序的执行流程大致如下：

（1）输入参数检查，根据传入参数确定打开输出文件时所要使用的标志符，参数有误时输出出错信息。

（2）将输入文件从 STDIN 读入缓冲区，避免计时时从硬盘读入导致 write 操作时间大幅延长。

（3） 调用 times() 函数，在 write 操作开始前和 write 操作结束后分别计时，两次结果相减便能得到 write 耗费市时长。每次 write 从内存缓冲区读入 BUFFERSIZE 大小的数据写入输出文件，并记录循环次数。其中 BUFFERSIZE 由 256 字节增长到 128K (131072) 字节。

（4）输出结果。

具体实现见源代码 timewrite.c .

##### 3) 实验结果

在 CentOS Release 5.1.1 环境下采用 gcc 4.1.2 进行编译。测试文件 testfile 大小为 6875452 B.

测试命令： ./timewrite <testfile out

输出结果：(BufferSize UserTime SysTime LoopTime)

256 		   0.00		   0.06		   0.08		26858
512 		   0.01		   0.02		   0.02		13429
1024		   0.00		   0.02		   0.02		6715
2048		   0.00		   0.01		   0.01		3358
4096		   0.00		   0.01		   0.01		1679
8192		   0.00		   0.00		   0.00		840
16384		   0.00		   0.01		   0.01		420
32768		   0.00		   0.01		   0.01		210
65536		   0.00		   0.00		   0.00		105
131072		   0.00		   0.01		   0.01		53

测试命令： ./timewrite <testfile out sync

输出结果：(BufferSize UserTime SysTime LoopTime)

256 	 	   0.01		   0.51		 828.19		26858
512 	 	   0.00		   0.21		  82.60		13429
1024		   0.01		   0.10		  27.74		6715
2048		   0.00		   0.06		  14.11		3358
4096		   0.00		   0.02		   6.96		1679
8192		   0.00		   0.03		   3.43		840
16384		   0.00		   0.01		   1.79		420
32768		   0.00		   0.02		   1.13		210
65536		   0.00		   0.01		   0.51		105
131072		   0.00		   0.01		   0.30		53

可见采用同步 I/O 时，等待磁盘输入的时间大大增加，且与 write 循环次数正相关。

##### 4) 注意事项

在实验过程中，发现了以下需要注意的问题和值得一提的事项：

1. 参数 sync 的匹配：当程序收到 3 个传入参数时 (第一个参数是程序执行路径) 需要检查最后一个参数是否为 sync ，最好的检查方法是使用 strncmp() 函数而要避免使用 == 操作符，这样可以匹配形如 sync* 的所有输入。
2. 打开输出文件：在使用 O_CREAT 符号打开输出文件时，需要同时指定输出文件的有关权限，否则存在同名文件时可能出现不能覆盖的问题。引入头文件 <sys/stat.h> 后在使用 open() 函数时指定 S_IWUSR | S_IRUSR 之类的权限符可以很好的解决这个问题，具体可以参照书本 (APUE.3e) 4.5节 Fig. 4-6.
3. times() 的使用：参照书本 (APUE.3e) 8.17节 Fig. 8-31. 对于 tms 结构，本次试验中 write 操作没有使用子进程所以不必使用相关的值，只是需要注意时间单位的转换。
4. write ：使用 write 时，每次都仅向输出文件写入 BUFFERSIZE 大小的内容，但是文件大小可能不被 BUFFERSIZE 整除，因此应该事先判断是否能被整除，若不能则最后一次 write 写入余下数据而非 BUFFERSIZE.