# High Performance Python

- [第一章　理解高性能 Python](#第一章-理解高性能-python)
- [第二章　通过性能分析找到瓶颈](#第二章-通过性能分析找到瓶颈)
- [第三章　列表和元组](#第三章-列表和元组)
- [第四章　字典和集合](#第四章-字典和集合)
- [第五章　迭代器和生成器](#第五章-迭代器和生成器)
- [第六章　矩阵和矢量计算](#第六章-矩阵和矢量计算)
- [第七章　编译成 C](#第七章-编译成-c)
- [第八章　并发](#第八章-并发)
- [第九章　multiprocessing 模块](#第九章-multiprocessing-模块)
- [第十章　集群和工作队列](#第十章-集群和工作队列)
- [第十一章　使用更少的 RAM](#第十一章-使用更少的-ram)


## 第一章　理解高性能 Python
- Python 全局解释器锁（GIL）确保了 Python 进程一次只能执行一条指令，这意味着任意时间仅有一个核心在工作。
- 理想计算模型
```python
import math

def check_prime(number):
    sqrt_number = math.sqrt(number)
    number_float = float(number)
    for i in xrange(2, int(sqrt_number)+1):
        if (number_float / i).is_integer():
            return False
    return True

print "check_prime(10000000) = ", check_prime(10000000) # False
print "check_prime(10000019) = ", check_prime(10000019) # True
```
在代码循环部分，与其一次次将 i 输入 CPU，我们更希望一次就将 number_float 和多个 i 的值输入 CPU 进行检查。CPU 的矢量操作可以一次进行多个独立计算。概念如下：
```python
import math

def check_prime(number):
    sqrt_number = math.sqrt(number)
    number_float = float(number)
    numbers = range(2, int(sqrt_number)+1)
    for i in xrange(0, len(numbers), 5):
        # the following line is not valid python code
        result = (number_float / numbers[i:(i+5)]).is_integer()
        if any(result):
            return False
    return True
```
我们让程序一次对 5 个 i 的值进行除法和整数检查。但这并不是合法的代码。

- Python 虚拟机
```python
def search_fast(haystack, needle):
    for item in haystack:
        if item == needle:
            return True
    return False
```
```python
def search_slow(haystack, needle):
    return_value = False
    for item in haystack:
        if item == needle:
            return_value = True
    return return_value
```
寻找这些无用的操作并删除它们。


## 第二章　通过性能分析找到瓶颈
- time 模块
```python
import time

start_time = time.time()
...
end_time = time.time()
secs = end_time - start_time
```

- 打印和修饰
```
print secs, "seconds"
```
打印的方法很快很简单，但会搞乱输出。

```python
from functools import wraps

def timefn(fn):
    @wraps(fn)
    def measure_time(*args, **kwargs):
        t1 = time.time()
        result = fn(*args, **kwargs)
        t2 = time.time()
        print ("@timefn:" + fn.func_name + " took " + str(t2 - t1) + " seconds ")
        return result
    return measure_time

@timefn
def calculate_z_serial_purepython(maxiter, zs, cs):
...
```

- timeit 模块
```
$ python -m timeit -n 5 -r 5 -s "import julia1" "julia1.calc_pure_python(...)"
```
`-s` 命令在设置阶段导入模块，`-n` 设置循环次数，`-r` 设置重复次数。

- %timeit 魔法函数
在 IPython 内部的 `%timeit` 魔法函数：
```
%timeit calc_pure_python(...)
```

- UNIX 的 time 命令
```
$ time -p python julia1_nopil.py
```
通过使用 `-p`，会得到三个结果：`real` 记录了整体的耗时，`user` 记录了 CPU 花在任务上的时间，但不包含内核函数耗费的时间，`sys` 记录了内核函数耗费的时间。time 还包含了启动 python 解释器的时间。开启 `--verbose` 得到更多输出信息。

- 使用 cProfile 模块
cProfile 钩入 CPython 的虚拟机来测量其每一个函数运行所花费的时间。
```
$ python -m cProfile -s cumulative julia1_nopil.py
```
`-s cumulative` 对每个函数累计花费的时间进行排序。

- line_profiler 逐行分析
`line_profiler` 是调查 CPU 密集型性能最强大的工具，首先使用 cProfile 找到需要分析的函数，然后使用 `line_profiler` 对函数进行分析。（`pip install line_profiler`）
```
kernprof.py -l -v julia1_lineprofiler.py
```
使用装饰器 `@profile` 标记选中的函数，用 kernprof.py 脚本运行你的代码。`-l` 表示逐行分析而不是逐函数分析，`-v` 用于显示输出。

- memory_profiler 诊断内存用量
`memory_profiler` 模块可以逐行测量内存占用率。（`pip install memory_profiler`）
```
$ python -m memory_profiler julia1_memoryprofiler.py
```

- 用 heapy 调查堆上的对象
`pip install guppy`
```python
from guppy import hpy

...
hp = hpy()
h = hp.heap()
print h
```

- 用 dis 模块检查 CPython 字节码
```
import dis
import julia1_nopil

dis.dis(julia1_nopil.calculate_z_serial_purepython)
```


## 第三章　列表和元组
- 动态数组：列表
列表支持 resize 操作，可以增加数组的容量。当一个大小为 N 的列表第一次需要添加数据时，Python 会超额分配一个大小为 M 的列表，M>N，多出的部分给未来的添加预留空间。然后旧列表的数据被复制到新列表中，旧列表销毁。我们继续添加数据直到 M 用完，Python 又重新分配一个列表。

- 静态数组：元组
元组不支持改变大小，但可以将两个元组合并为一个新元组。但新元组不会超额分配空间，对元组添加每一个新元素都会有分配和复制操作。

## 第四章　字典和集合
字典和集合使用散列表来获得 O(1) 的查询和插入。能得到这一效率是因为散列函数将一个任意的键转变成了一个列表的索引。

每当 Python 访问一个变量、函数或模块时，都有一个体系来决定它去哪里查找这些对象。首先，查找 locals() 数组，其内保存了所有本地变量的条目。如果不在本地变量里，则搜索 globals() 字典。最后，如果如果还没找到，就搜索 `__builtin__` 对象。


## 第五章　迭代器和生成器


## 第六章　矩阵和矢量计算

## 第七章　编译成 C

## 第八章　并发

## 第九章　multiprocessing 模块

## 第十章　集群和工作队列

## 第十一章　使用更少的 RAM
