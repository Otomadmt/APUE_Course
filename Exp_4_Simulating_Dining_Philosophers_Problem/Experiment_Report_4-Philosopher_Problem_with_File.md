## UNIX系统环境编程课程实验报告

> 实验 4 - “五个哲学家”问题模拟
>
> 报告完成时间: 19th Nov, 2016
>



##### 1) 实验内容

编程模拟“五个哲学家“问题，掌握并发进程同步的概念与方法。

要求使用文件实现进程间信号传递，不能出现哲学家“饿死”的情况。

进程语法为 `philosopher [-t <time>]`.

其中整数  `time` 是哲学家进程进餐和思考的持续时间值，默认值为 2 秒。

##### 2) 实现分析

要避免“饿死”的情况的出现，主要需要解决“死锁”问题，即 A 进程等待 B 进程释放资源的同时，B 进程也在等待 A 释放资源这种情况。

本次试验中采用文件来实现信号传递，程序运行时会先删除实现存在的“叉子”文件。当哲学家进程尝试拿起“叉子”时，若“叉子”文件已存在，则说明叉子被占用，进入阻塞状态；否则成功拿起叉子，新建文件，用餐完毕后删除文件，解除占用，具体实现见源代码 lock.c 与 lock.h 。

哲学家的行为在思考  `time` 秒 -> 尝试拿起叉子 -> 进食 `time` 秒 -> 放下叉子 -> 思考  `time` 秒间循环，具体实现与注释见源代码 philosopher.c .

##### 3) 死锁问题的避免

要避免出现死锁， 重点是避免出现所有哲学家都没法同时拿起两把叉子的情况。这里只需要让哲学家群体中同时存在左利手与右利手即可，可通过下面这种对 `takeFork()` 函数与 `putFork()` 函数的定义来控制哲学家行为。

```C
void takeFork(int i) {
	if(i == _PHILO_NUM_ - 1) {
		/* Left hand first */
		lock(forks[0]);
		lock(forks[i]);
	} else {
		/* Right hand first */
		lock(forks[i]);
		lock(forks[i + 1]);
	}	
}

void putFork(int i) {
	if (i == _PHILO_NUM_ - 1) {
		/* Left hand first */
		unlock(forks[0]);
		unlock(forks[i]);
	} else {
		/* Right hand first */
		unlock(forks[i]);
		unlock(forks[i + 1]);
	}
}
```

##### 4) 实验结果

在 CentOS Release 5.1.1 环境下采用 gcc 4.1.2 进行编译。

部分输出内容：

```Bash
philosopher 0 is thinking
philosopher 1 is thinking
philosopher 2 is thinking
philosopher 3 is thinking
philosopher 4 is thinking
philosopher 3 is eating
philosopher 3 is thinking
philosopher 2 is eating
philosopher 2 is thinking
philosopher 1 is eating
philosopher 3 is eating
philosopher 1 is thinking
philosopher 0 is eating
philosopher 2 is eating
philosopher 3 is thinking
philosopher 0 is thinking
philosopher 4 is eating
...
```

按 Ctrl + C 结束父进程。

##### 5) 实验体会

哲学家问题体现了多进程编程的基本问题之一：资源分配中的死锁现象。

出现这种死锁现象的原因之一是操作缺乏原子性（比如不可同时分配到所有资源）导致的险象、竞争等问题，特别是在高并发环境下尤为明显。

除此之外，对于资源的合理调度、并发性能的提高、错误恢复等也是高并发环境编程的重要课题，在目前的分布式计算渐为流行的背景下显得更为重要。

本次实验简单的采用了文件来传递相关信号，存在耗时较高、鲁棒性差等弊端。相比之下采用信号、IPC 机制或线程／协程的方案更加可行。

---

