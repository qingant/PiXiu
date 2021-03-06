PiXiu是什么？
---
PiXiu（貔貅）是我脑洞大开下的产物，灵感来源于[TerarkDB][1]，其作者表示使用了Trie和自动机，从而能在**压缩**的数据上直接进行索引，优于数据库常用的B-Tree，LSM。我非常感兴趣，但实现细节TerarkDB的作者并没有透露。于是，我自己脑补了PiXiu方法。

数据压缩的一种途径是将冗余的数据用更短的记号表示。我的设计是使用General Suffix Tree（通用后缀树）来发现字符串组之间的公共子串，然后用特殊标记来让这些字符串互相引用。

比如：
> 字符串A：BCAAAAAA
>
> 字符串B：EDAAAAAA

很显然，AB字符串的最后5个字符是相同的。我可以完美地这样表示：
> 字符串A：BCAAAAAA
>
> 字符串B：ED$A,3,6$

字符串B被改写了。$是特殊符号表示一个压缩组的开始与结束。$A,3,6$的含义很明显了，说明接下来的数据已经存在于叫A的字符串，从位置3到位置6。

后缀树不仅能发现不同字符串之间的重复，自身的重复也可以。字符串A可以这么写。
> 字符串A：BCA$A,3,7$

一下子不好理解吧！我也是在真正实现后缀树的时候发现的。$A,3,7$最终会**递归展开**成AAAAA。这个大家可以拿张纸模拟一下后缀树以及递归解压调用。

后缀树是目前所学中个人觉得最接近理论完美的算法。它能在N的时间，N的空间，表示N个排列，还是一个在线算法。

#### 还有一个问题！
数据嵌套压缩之后，对比操作将会疯狂递归来解析字符串，这对性能是不是会有影响？必然的。所以红黑树以及别的树就不是那么适用了。还好，我发现了[CritBit Tree][2]。这个算法的特点是所有原始数据永远只会对比一次，其余时间都是用CritBit（我译为特征比特）进行跳转。每添加一条数据，永远只会付出一个指针，一个16bit int，一个8bit int的空间代价。与此同时，时间上界还特别好，跟普通的Trie一样。缺点是指针跳转太多，不利于CPU Cache。

这里有什么？
---
以上的算法与设计说起来很简单，但用C++实现起来真的爆炸复杂！我在2016年10月左右开始系统学习使用C/C++，每天还要上班，业余时间全给PiXiu计划了。写到现在，感觉身体也变差了，需要休息一段。如果不赶快发布出来，要是有人开源类似的，那我的Credit不是没了？

原计划是接下来学习Linux C API，加上网络模块。再然后，加上分布式。真这么玩下去，要猝死在电脑前了。尽管，这是一个未真正完成的项目。但最独特的内核，我已经保质保量地完成了。所以，我现在发布的是一个没有**任何耦合**的索引引擎。

我有三个理由说明测试是充分的。

1. 所有的组件都和stl的vector或者map进行了随机对照测试，增删改数据，只有一致，才会认为可靠。

2. gcov的**coverage 100%**。几行没覆盖到的，是我认为理论上一定会发生的情况，但好像几百万次随机测试也没发生一次，为了保险就不删了。

3. 使用Xcode内置profiler修复干净了内存泄漏和越界（Valgrind在最新的OSX下跑不起）。


性能测试
---
如果性能上不去，那不是白说。事先声明，这个测试一定是有利于我的。这很正常，要是作者测试都不给力，这个项目还有什么价值？

方法：
Python抓取数万HTML页面，URL作为Key，页面代码作为Value，存入PiXiu，记录内存占用与速度。

为什么说有偏向呢？因为，URL和网页都是高冗余的数据，自然会跑出令人惊讶的压缩比。为了让数据不那么失真（不让PiXiu太厉害），我过滤掉了所有的不可见字符（空格等）。篇幅所限，更具体测试流程可以知乎私信或查看专栏。

以QQ首页 http://www.qq.com/ 为根，采集1W页面，每个取其开始的6W ASCII字符，写入PiXiu。页面总大小：395782474字节，PiXiu压缩大小：334142231字节，**压缩率高达84%！** 当然，这有注水猪肉嫌疑，但你有没有被打动呢？想打我脸的话，快来试试吧～ CPU运行时间相对来说会慢一点。如果每条记录100字节，1秒能写入1W-2W条。纯Hash应该上10W是很轻松的。算法必然会有trade off。

Playground
---
正如我前面提到的，这是一个完整的索引引擎。不过，为了让人快速玩起来，我给PiXiu封装了一个简单的命令行。实际API不止这两个，下一章我会说明。

> GET KEY
>
> 返回KV键值对
>
> SET KEY::VALUE
>
> 存储KV，返回压缩（节约）的bytes数

下载本仓库中release目录下的PiXiuPlus文件，目前只有Linux（Ubuntu）和Mac版本

例子：
```
YUANJINdeMBP:cmake-build-release yuanjinlin$ ./PiXiuPlus
Command: GET 123

Command: SET 123::321
本次节约内存数 0
总共节约内存数 0

Command: GET 123
123::321

Command: SET BOBO::https://www.zhihu.com/question/55439090
本次节约内存数 0
总共节约内存数 0

Command: SET BOBO1::https://www.zhihu.com/question/22454692
本次节约内存数 27
总共节约内存数 27

Command: GET BOBO
BOBO::https://www.zhihu.com/question/55439090

Command: GET BOBO1
BOBO1::https://www.zhihu.com/question/22454692

Command: ~
```

可以看到SET BOBO是没有压缩的，因为没有任何已存字符串与当前的重复。当SET BOBO1的时候，之前已经有了BOBO，这下PiXiu只要存储差异就好了。所以，这次插入比直接存储两个字符串节约了27个bytes。最后，使用波浪号～退出。

API
---
后续的开发者只要调用PiXiuCtrl这个类就好了。别的最好不要动，我花了巨大的精力才把Bug杀干净。
```cpp
struct PiXiuCtrl {

    int setitem(uint8_t[], int, uint8_t[], int);

    bool contains(uint8_t[], int);

    PXSGen * getitem(uint8_t[], int);

    CBTGen * iter(uint8_t[], int);

    int delitem(uint8_t[], int);

    void init_prop(void);

    void free_prop(void);
};
```
插入{"WhoAmI":"ChengLin"}为例子：
```cpp
PiXiuCtrl ctrl;
ctrl.init_prop(); // 初始化
std::string key = "WhoAmI";
std::string val = "ChengLin";

// 存入数据
ctrl.setitem((uint8_t *) key.c_str(), (int) key.size(),
             (uint8_t *) val.c_str(), (int) val.size());

// 判断是否存在
ctrl.contains((uint8_t *) key.c_str(), (int) key.size());

// 建议看下 https://www.codeproject.com/tips/29524/generators-in-c
// 里面描述如何用C++写0浪费的协程
// getitem返回的是一个生成器
PXSGen * gen = ctrl.getitem((uint8_t *) key.c_str(), (int) key.size());
uint8_t rv;
while (gen->operator()(rv)) {
    printf("%c", rv); // 将会打印出 ChengLin
}
PXSGen_free(gen);
// 遍历（iter方法）也是类似的

// 删除数据
ctrl.delitem((uint8_t *) key.c_str(), (int) key.size());

ctrl.free_prop(); // 释放资源
```

注意事项、缺陷、可能的解决方案
---
### PiXiu编码
前文举例的时候，使用$作为压缩组的标记，实际实现的时候必须要考虑二进制安全。所以，我设计了PiXiu编码，所有值为251的byte将被escape为251,251。如果你的输入包含100个251，那么实际存储的时候将会有200个251（不考虑压缩）。e.g. (1,2,3,251,2,3) => (1,2,3,251,251,3)

为什么我不把这一细节隐藏起来呢？因为KV的分割跟251很有关系。Key的结尾标记是251,0，Value的结尾标记是251,2。上章的{"WhoAmI":"ChengLin"}，实际返回的会是WhoAmI 251,0 ChengLin 251,2。我确信这一细节对于理解PiXiu的设计与局限很重要，就暴露了出来。

- 解决方案：
套一层简单的Parser。
如果只是存储ASCII字符，把所有不可见的字符都忽略即可。

PiXiu编码还带来了另一个局限就是每一个记录最多记录**65531-值为251的bytes数**。比如，我有65531个字符，但里面有100个251，那么escape之后就是65631个字符，超出了最大大小。开发的时候用CMake的Debug模式，越界会有我写的assert提示。

- 解决方案：
将超大数据分块，使得每条记录escape之后KV加起来不超过65535。

实际上，每一个记录不止有为了二进制安全的内存成本，额外还会有12byte的内存overhead。因此，一大票零零散散的小数据，可能不会有压缩的效果。PiXiu确保副作用很小。

- 解决方案：
把多条记录写成一个大记录就好了，参考B-Tree的做法。

### 编译
这是一个巨坑。起初，我只是想要学C，接着开始写PiXiu。C很容易，但缺乏很多基础设施。我甚至自己用macro模仿了一个vector。后面就切换到C++去了，用的IDE是Mac OSX下的CLion，标准定在了C++14。

这导致了严重的兼容问题。不管是Linux还是Mac OSX下，都只能用CMake 3.6以及Clang 3.8以上的工具链编译。

Mac下我强烈建议下一个Clion 2016，然后直接编译。一定要手工的话，如果环境都有就很简单了，mkdir build&&cd build&&cmake -DCMAKE_BUILD_TYPE=Release ..&&make。

Linux下一般都是高手，记得CXX=clang++。

发布协议
---
BSD协议

[1]:https://www.zhihu.com/question/46787984
[2]:https://github.com/agl/critbit/