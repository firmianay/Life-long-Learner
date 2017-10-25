# 第一本 Docker 书


## 第一章 简介
Docker 是一个能够把开发的应用程序自动部署到容器的开源引擎。Docker 在虚拟化的容器执行环境中增加了一个应用程序部署引擎，目标是提供一个轻量、快速的环境，能够运行开发者的程序，并方便高效地将程序从开发者的笔记本部署到测试环境，然后在部署到生产环境。

#### Docker 核心组件：
Docker客户端和服务器：Docker 是一个客户端/服务器（C/S）架构的程序。客户端只需向 Docker 服务器或守护进程发出请求，服务器或守护进程将完成所有工作并返回结果。Docker 守护进程也称作 Docker 引擎。

Docker 镜像：基于联合文件系统的一种层式结构

Registry：用来保存用户构建的镜像

Docker 容器：执行环境

#### Docker 技术组件
一个原生的 Linux 容器格式，Docker 中称为 libcontainer。

Linux 内核的命名空间（namespace），用于隔离文件系统、进程和网络。

文件系统隔离：每个容器都有自己的 root 文件系统。

进程隔离：每个容器都运行在自己的进程环境中。

网络隔离：容器间的虚拟网络接口和 IP 地址都是分开的。

资源隔离和分组：使用 cgroups（即 control group，Linux 内核特性）将 CPU 和内存之类的资源独立分配给每个 Docker 容器。

写时复制：文件系统都是通过写时复制创建的。

日志：容器产生的 `STDOUT` 、 `STDERR` 和 `STDIN` 这些 IO 流都会被收集并记入日志。

交互式 shell：用户可以创建一个伪 tty 终端，将其连接到 `STDIN` ，为容器提供一个交互式 shell。

扩展资源：
- http://www.docker.com/
- https://github.com/docker/docker
- https://github.com/dockerforge
- https://groups.google.com/forum/#!forum/docker-user
- https://twitter.com/docker
- http://stackoverflow.com/search?q=docker

## 第二章 安装Docker（Debian）

1. 检查内核版本：
```
uname -a
```

2. 查看系统版本：

http://distrowatch.com/

3. 添加 Docker 的 ATP 仓库：
```
sudo sh -c "echo deb https://apt.dockerproject.org/repo debian-stretch main > /etc/apt/sources.list.d/docker.list"
```

4. 添加 Docker 仓库的 GPG 密钥：
```
apt-key adv --keyserver hkp://p80.pool.sks-keyservers.net:80 --recv-keys 58118E89F3A912897C070ADBF76221572C52609D
```

5. 更新 APT 源：
```
sudo apt-get update
```

6. 安装 Docker：
```
sudo apt-get install docker-engine
```

7. 打开 Docker 守护进程：
```
sudo service docker start
```

8. 确认安装：
```
sudo docker info
```

#### 更新：
```
sudo apt-get upgrade docker-engine
```

#### 卸载：
```
sudo apt-get purge docker-engine
sudo apt-get autoremove
rm -rf /var/lib/docker
```

#### Docker 守护进程：
守护进程监听 `/var/run/docker.sock` 这个套接字，来获取来自客户端的 Docker 请求。

#### 配置 Docker 守护进程
使用 `-H` 标志指定不同的网络接口和端口配置：

```
sudo docker daemon -H tcp://0.0.0.0:2375
```

也可以用 `-H` 标志指定一个 Unix 套接字路径：
```
sudo docker daemon -H unix://home/docker/docker.sock
```

同时指定多个地址：
```
sudo docker daemon -H tcp://0.0.0.0:2375 -H unix://home/docker/docker.sock
```

开启 Docker 守护进程的调试模式：
```
sudo docker daemon -D
```

如果要永久改动，则需要修改 `/etc/default/docker` 文件的 `DOCKER_OPTS` 变量。

#### 检查 Docker 守护进程是否正在运行
```
// 检查状态
sudo service docker status
// 启动
sudo service docker start
// 停止
sudo service docker stop
```

## 第三章 Docker入门
#### 运行第一个容器

```
sudo docker run -i -t ubuntu /bin/bash
```

`-i` 标志保证容器中的 `STDIN` 是开启的，`-t` 标志为要创建的容器分配一个伪 `tty` 终端。

首先 Docker 会检查本地镜像，如果没有则会下载到本地。

#### 使用容器
检查容器主机名：
```
root@d0cef31f8120:/# hostname
d0cef31f8120
```

检查容器的 /etc/hosts 文件：
```
root@d0cef31f8120:/# cat /etc/hosts 
127.0.0.1	localhost
::1	localhost ip6-localhost ip6-loopback
fe00::0	ip6-localnet
ff00::0	ip6-mcastprefix
ff02::1	ip6-allnodes
ff02::2	ip6-allrouters
172.17.0.2	d0cef31f8120
```

检查容器进程：
```
root@d0cef31f8120:/# ps -aux
USER       PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND
root         1  0.0  0.0  18240  3128 ?        Ss   14:44   0:00 /bin/bash
root        18  0.0  0.0  34424  2772 ?        R+   14:49   0:00 ps -aux
```

在容器中安装软件包：
```
root@d0cef31f8120:/# apt-get update && apt-get install vim
```

使用 `exit` 退出容器时，容器就停止运行了，但还是存在的，可以使用 `docker ps -a` 命令查看当前系统中容器的列表：
```
bibi@bibi-pc:~$ sudo docker ps -a
[sudo] bibi 的密码：
CONTAINER ID        IMAGE               COMMAND                  CREATED             STATUS                         PORTS               NAMES
d0cef31f8120        ubuntu              "/bin/bash"              51 minutes ago      Exited (1) 17 seconds ago                          nauseous_engelbart
26d00ca60271        ubuntu              "/bin/echo 'hello wor"   About an hour ago   Exited (0) About an hour ago                       gloomy_brown
```

#### 容器命名
```
sudo docker run --name bob_the_container -i -t ubuntu /bin/bash
```

#### 重新启动已经停止的容器
```
sudo docker start bob_the_container
```

或者通过 ID 来启动：
```
sudo docker start d0cef31f8120
```

也可以使用 `docker restart` 命令来重新启动一个容器。

#### 附着到容器上
```
sudo docker attach bob_the_container
// 也可以使用ID
```

#### 创建守护式容器
创建没有交互式会话的守护式容器，用于长期运行的应用程序的服务。

```
sudo docker run --name daemon_dave -d ubuntu /bin/sh -c "while true; do echo hello world; sleep 1;done"
```

使用了 `-d` 参数，把容器放到后台执行。

#### 容器内部都在干什么
使用 `docker logs` 命令来获取容器的日志。
```
bibi@bibi-pc:~$ sudo docker logs daemon_dave
hello world
hello world
hello world
hello world
. . .
```

可以使用 `-f` 参数来实时监控日志。
```
sudo docker logs -f daemon_dave
```

获取最后10行内容：
```
sudo docker logs --tail 10 daemon_dave
```

跟踪最新日志而不必读取整个日志文件：
```
sudo docker --tail 0 -f daemon_dave
```

使用 `-t` 为每条日志项加上时间戳：
```
sudo docker logs -ft daemon_dave
```

#### Docker日志驱动
在启动 Docker 守护进程或者执行 `docker run` 命令时使用 `--log-driver` 选项来控制日志驱动。
```
sudo docker run --log-driver="syslog" --name daemon_dwayne -d ubuntu /bin/sh -c "while true; do echo hello world; sleep 1; done"
```

上面的命令会将容器的日志输出到 `Syslog`。

#### 查看容器内的进程
```
bibi@bibi-pc:~$ sudo docker top daemon_dave
UID                 PID                 PPID                C                   STIME               TTY                 TIME                CMD
root                6743                6728                0                   10:05               ?                   00:00:00            /bin/sh -c while true; do echo hello world; sleep 1;done
root                7824                6743                0                   10:19               ?                   00:00:00            sleep 1
```

可以看到容器内的所有进程，运行进程的用户及进程 ID。

#### Docker 统计信息
```
bibi@bibi-pc:~$ sudo docker stats daemon_dave nauseous_engelbart
CONTAINER            CPU %               MEM USAGE / LIMIT   MEM %               NET I/O             BLOCK I/O           PIDS
daemon_dave          0.07%               0 B / 0 B           0.00%               12.25 kB / 648 B    2.234 MB / 0 B      2
nauseous_engelbart   0.00%               0 B / 0 B           0.00%               20.73 kB / 648 B    4.687 MB / 0 B      1
```

#### 在容器内部运行进程
可以通过 `docker exec` 命令在容器内部额外启动新进程，有后台任务和交互式任务两种类型。

在容器中运行后台任务：
```
sudo docker exec -d daemon_dave touch /etc/new_config_file
```

`-d` 标志之后，指定的是要在内部执行这个命令的容器的名字以及要执行的命令。

运行交互命令：
```
sudo docker exec -t -i daemon_dave /bin/bash
```

停止守护式容器：
```
sudo docker stop daemon_dave
```

自动重启容器：

如果由于某种错误而导致容器停止运行，可以通过 `--restart` 标志，让 Docker 自动重新启动该容器。

```
sudo docker run --restart=always --name daemon_dave -d ubuntu /bin/sh -c "while true; do echo hello world; sleep 1; done"
```

`--restart` 标志被设置为 `always`，无论容器的退出代码是什么，都会自动重启。如果设置为 `on-failure`，那么只有当退出代码为非0值的时候，才会自动重启。

#### 深入容器
获得更多的容器信息：
```
sudo docker inspect daemon_dave
```

可以使用 `-f` 或者 `--format` 标志来选定查看结果：
```
// 返回容器的运行状态
sudo docker inspect --format '{{ .State.Running }}' daemon_dave
// 查看容器的IP地址
sudo docker inspect --format '{{ .NetworkSettings.IPAddress }}' daemon_dave
```

#### 删除容器
```
sudo docker rm -f daemon_dave
```

删除所有容器：
```
sudo docker rm 'sudo docker ps -a -q'
```

## 第四章 使用Docker镜像和仓库

Docker镜像是由文件系统叠加而成的。一个镜像可以放到另一个镜像的顶部。

当创建一个新容器时，Docker会构建一个镜像栈，并在栈的最顶层添加一个读写层。

#### 列出镜像
本地镜像都保存在本地的 `/var/lib/docker` 目录下。
```
bibi@bibi-pc:~$ sudo docker images
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
ubuntu              latest              f753707788c5        4 weeks ago         127.1 MB
```

镜像保存在仓库中，而仓库存在于 `Registry` 中，默认为公共 `Registry` 服务，即 `Docker Hub`。每个镜像仓库可以存放多个镜像。
```
// 拉取Ubuntu镜像
sudo docker pull ubuntu:12.04
// 列出Docker镜像
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
ubuntu              latest              f753707788c5        4 weeks ago          127.1 MB
ubuntu              12.04              0b310e6bf058        3 months ago       225.4 MB
ubuntu              precise           0b310e6bf058        3 months ago       225.4 MB
```

这里就表明 ubuntu 镜像实际上是聚集在一个仓库下的一系列镜像。

通过标签来区分镜像（同一个镜像可以有多个标签）：
```
sudo docker run -t -i --name new_container ubuntu:12.04 /bin/bash
```

#### 查找镜像
```
bibi@bibi-pc:~$ sudo docker search ubuntu
NAME                              DESCRIPTION                                     STARS     OFFICIAL   AUTOMATED
ubuntu                            Ubuntu is a Debian-based Linux operating s...   5056      [OK]       
ubuntu-upstart                    Upstart is an event-based replacement for ...   68        [OK]       
rastasheep/ubuntu-sshd            Dockerized SSH service, built on top of of...   49                   [OK]
consol/ubuntu-xfce-vnc            Ubuntu container with "headless" VNC sessi...   29                   [OK]
```

返回信息有仓库名、镜像描述、用户评价、是否官方、自动构建。

#### 构建镜像
登陆到 Docker Hub，可以使用 `docker logout` 从一个 Registry 服务器退出。
