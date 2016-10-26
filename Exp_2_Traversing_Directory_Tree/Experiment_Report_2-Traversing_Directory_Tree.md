## UNIX系统环境编程课程实验报告

> 实验2 - 目录树的遍历
>
> 报告完成时间: 24th Oct, 2016



##### 1) 实验内容

在课本 (APUE.2e) 程序 ftw4.c 的基础上编写程序 myfind，详细要求见下：

命令语法： ```myfind <pathname> [-comp <filename> | -name <str>...]```

功能：

* ```myfind <pathname>```

  统计 \<pathname> 及其目录子树下各类文件所占比例，并统计长度不大于4096字节的普通文件占所有允许访问的普通文件的百分比，不允许打印任何路径。


* ```myfind \<pathname> -comp \<filename>```

  输出在 \<pathname> 及其目录子树下所有与 \<filename> 文件内容一致的文件的绝对路径。其中 \<filename> 为一个常规文件的路径。


* ```myfind \<pathname> -name \<str>…```

  输出在 \<pathname> 及其目录子树下所有文件名与 \<str> 序列中某一字符串相同的文件的绝对路径。   

  ​

其中 \<str>… 是一个不带任何路径的以空格分隔的文件名序列， \<pathname> 和 \<filename> 中的路径既可以是绝对路径也可以是相对路径。此外 \<pathname> 既可以是目录也可以是文件；为文件时其目录为当前工作目录。

##### 2) 实现分析

本次实验程序要求实现 3 个功能：

（1）统计 \<pathname> 及其目录子树下各种文件所占比例：

​	实现本功能主要需要应用递归下降遍历目录的方法（通过教材上的 myftw() 与 dopath() 函数实现），根据 lstat() 函数所返回的文件统计信息来分别统计有权限访问到的各类文件的数量。

​	相比教材实验新增了“不打印路径”和“统计不大于 4096 字节的普通文件比例”这两个要求。对于前一个要求，简单修改课本上的代码可以实现；对于后一个要求，通过比较 lstat() 返回的结构体中的 st_size 大小也很容易完成。

（2）-comp : 打印出 \<pathname> 及其目录子树下与 \<filename> 内容一致的文件的绝对路径 ：

​	在（1）的框架下我们可以保证对 \<pathname> 及其目录子树下的所有文件的遍历，进而我们可以通过比较每一个文件与给定的常规文件 \<filename> 的内容来实现寻找内容一致的文件的功能。要实现这一新的功能，我们只需向 myftw() 函数传递不同的函数指针即可。课本上提供的这一框架很大程度上提高了程序的可扩展性。

​	但本实验要求输出内容一致的文件的绝对路径。为解决这个问题，可以引入一个新的函数 getAbsPath() 实现将给定路径转化为绝对路径的功能。借助于 getAbsPath() 函数，我们可以在进行搜索之前提前将 \<pathname> 转化为绝对路径再传入，从而保证输出的路径均为绝对路径。

​	此外，在比较文件内容时，以下两点可以很大程度上提高比较效率：

​	  a. 事先将源文件的内容读入内存，避免重复从硬盘读取（类似实验一）；

​	  b. 事先通过 lstat() 比较文件大小，只对文件大小一致的文件进行内容比对。

（3）-name : 打印出 \<pathname> 及其目录子树下所有文件名与 \<str>... 字符序列中任一字符串一致的文件的绝对路径 ：

​	同样通过对 \<pathname> 及其目录子树下的所有文件的遍历，结合 getAbsPath() 函数，我们可以实现寻找文件名相同的文件的绝对路径这一功能。

​	实现时仍然有以下问题需要考虑：

​	a. 从绝对路径中获取文件名：为比较文件名是否相同，我们需要从经过了 getAbsPath() 处理后的绝对路径中获得文件名。为此我们引入 getFilenamePos() 这一新的函数来返回文件名在绝对路径中的开始位置以供比较。

​	b. 处理多个输入：\<str>... 是一个字符序列，允许输入多个文件名。我们可以采用一个 for-loop 来对序列中的每个字符串都进行一次遍历比对（文件量远大于序列中字符串数目时可对每一个文件查找其文件名是否在序列内）。同时在比对前需要保证序列内没有包含任何目录。

具体实现与注释见源代码 myfind.c .

##### 3) 实现 getAbsPath() 与 getFilenamePos() 的注意事项

* getAbsPath() 的实现：

​	getAbsPath() 接受两个字符串指针 srcpath, abspath 作为传入，没有返回值。

​	abspath 为储存 srcpath 对应绝对路径的字符串。

​	getAbsPath() 将调用 chdir() 与 getcwd() 函数，先保存当前工作路径，然后切换到 srcpath 下获取其绝对路径，完成绝对路径拼接后再回到保存的工作路径以免打乱程序正常遍历（argv[0] 与 srcpath 不相同的情况）。

* getFilenamePos() 的实现：

​	相对而言 getFilenamePos() 的实现较为简单——寻找绝对路径中的最后一个 / 出现的位置即可，未找到则返回 0。需要注意的是对于根目录 / 该函数也会返回 0。为区分根目录与未找到的情况，可以更改未找到的返回值或对 / 单独做判断处理。

##### 4) 实验结果

在 CentOS Release 5.1.1 环境下采用 gcc 4.1.2 进行编译，工作目录为 /home/cs14/csunix/homework/2 .

测试命令： ```./myfind /```

输出结果：
```
regular files  =  114285, 84.34 %
<=4096B in reg =   80117, 70.10 %
directories    =   14883, 10.98 %
block special  =      34,  0.03 %
char special   =     162,  0.12 %
FIFOs          =       3,  0.00 %
symbolic links =    6129,  4.52 %
sockets        =      11,  0.01 %
```
测试命令： ```./myfind . -comp ./testfile/new```

输出结果：
```
Matching content of './testfile/new':
/home/cs14/csunix/homework/2/testfile/testfile_new
/home/cs14/csunix/homework/2/testfile/newb_3copy
/home/cs14/csunix/homework/2/testfile/subtree/testfile_new
/home/cs14/csunix/homework/2/testfile/subtree/new
/home/cs14/csunix/homework/2/testfile/newb
5 match(es)found.
```
测试命令： ```./myfind . -name new newb ntsm```

输出结果：
```
3 filename(s) to match:

Matching filename 'new'(1/3):
/home/cs14/csunix/homework/2/testfile/subtree/new
/home/cs14/csunix/homework/2/testfile/new
2 match(es) found.

Matching filename 'newb'(2/3):
/home/cs14/csunix/homework/2/testfile/newb
1 match(es) found.

Matching filename 'ntsm'(3/3):
/home/cs14/csunix/homework/2/testfile/subtree/ntsm
/home/cs14/csunix/homework/2/testfile/ntsm
2 match(es) found.
```
测试时所使用的 testfile 文件夹已上传。

##### 5) 实验体会

本次实验内容较上一次实验而言更为繁复，尤其是在异常处理上两次实验都需要花许多心思，代码也因而显得繁杂了许多。

作为操作系统中最为基本的程序，健壮性 (Robust) 的要求与必要程度不言而喻，但是大多数情况下我们都没法处理所有异常，就像任何程序都会在计算资源耗尽时崩溃一样——无论你做了多少事前的工作来防止未料到的情况发生。

因而，POSIX 标准的出现便是为了在一定程度上解决类似问题：工程师在进行设计师需要制定一套相应的标准来指定哪些事项是必须遵循的，哪些事项是未定义的、不必多加考虑的。

这种思维的出现在保证了一个项目拥有合理鲁棒性的同时也将推动了项目的快速发展——这便是标准化的作用之一。
