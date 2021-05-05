# Android 安全攻防实战

## 第一章 Android开发工具

### 使用命令行创建 Android 虚拟设备（AVD）

1. 获得可以使用的系统镜像列表
```
[path-to-sdk-install]/tools/android list targets
```
2. 创建 AVD
```
[path-to-sdk-install]/tools/android create -avd -n [AVD name] -t [system image target id]

// more external storage
[path-to-sdk-install]/tools/android create -avd -n [AVD name] -t [system image target id] -c [size] [K|M]
```
3. 运行创建的 AVD
```
[path-to-sdk-install]/tools/emulator -avd [AVD name]

// specify internal storage
[path-to-sdk-install]/tools/emulator -avd [AVD name] -partition-size [size in MBs]
```

扩展阅读：
https://developer.android.com/studio/tools/help/android.html

### 使用 Android 调试桥（ADB）与 AVD 交互

1. 启动指定的 AVD
```
[path-to-sdk-install]/tools/emulator -avd [name]
```
2. 列出所有已经连接上来的 Android 设备
```
[path-to-sdk-install[/platform-tools/adb devices
```
3. 运行一个连接到 Android 设备的 shell
```
/sdk/platform-tools/adb shell
```

扩展阅读：
https://developer.android.com/studio/command-line/adb.html

### 从AVD 上复制出/复制入文件

1. 从 AVD 上把文件复制出来
```
adb {options} pull [path to copy from] [local path to copy to]
```
2. 把文件复制到 AVD 里去
```
adb {options} push [local path to copy from] [path to copy to on AVD]
```

### 通过 ADB 在 AVD 中安装 app

```
adb {options} install [path to apk]
```

## 第二章 实践 app 安全

### 检查 app 的证书和签名

1. 从 Android 中取出一个 app
```
/sdk/platform-tools/adb pull /system/app/Calendar/Calendar.apk
```
2. 解压 apk
```
unzip Calendar.apk
```
3. 在 "META_INF" 的文件夹里
  - `MANIFEST.MF`：声明了资源，与 `CERT.SF` 文件相似。
  - `CERT.RSA`：公钥证书。
  - `CERT.SF`：包含了 app 中所有的资源文件，负责对 app 进行签名。
```
[jdk]/bin/keytool -printcert -file META-INF/CERT.RSA
```
4. 查看签名文件的内容，包含了 app 中的各个资源文件的密码学意义上的 hash。
```
cat [path-to-unzipped-apk]/META-INF/CERT.SF
```
5. 使用 OpenSSL 查看证书
```
openssl pkcs7 -inform DER -in META-INF/CERT.RSA -noout -print_certs -text
```

扩展阅读：
https://datatracker.ietf.org/doc/rfc2459/?include_text=1
http://docs.oracle.com/javase/6/docs/technotes/guides/security/cert3.html

### 对 Android app 签名

先删掉现有的 `META-INF` 文件夹，然后建立签名。
1. 建立一个密钥储存器（keystore），用来存放签名 app 时要用的私钥。并把这个密钥存储器放在一个安全的地方。
```
// 生成
keytool -genkey -v -keystore [nameofkeystore] -alias [your_keyalias] -keyalg RSA -keysize 2048 -validity [numberofdays]
// 删除
keytool -delete -alias [your_keyalias] -keystore [nameofkeystore] -storepass [password]
```
```
/usr/local/java/jdk1.8.0_112/bin/keytool -genkey -v -keystore releasekey.keystore -alias keyalias -keyalg RSA   
Picked up _JAVA_OPTIONS:   -Dawt.useSystemAAFontSettings=gasp
输入密钥库口令:  
您的名字与姓氏是什么?
  [Unknown]:  Li Hua
您的组织单位名称是什么?
  [Unknown]:  Xi dian
您的组织名称是什么?
  [Unknown]:  Mo Ha Xie Hui
您所在的城市或区域名称是什么?
  [Unknown]:  xi'an
您所在的省/市/自治区名称是什么?
  [Unknown]:  shanxi
该单位的双字母国家/地区代码是什么?
  [Unknown]:  China
CN=Li Hua, OU=Xi dian, O=Mo Ha Xie Hui, L=xi'an, ST=shanxi, C=China是否正确?
  [否]:  y

正在为以下对象生成 2,048 位RSA密钥对和自签名证书 (SHA256withRSA) (有效期为 90 天):
	 CN=Li Hua, OU=Xi dian, O=Mo Ha Xie Hui, L=xi'an, ST=shanxi, C=China
输入 <keyalias> 的密钥口令
	(如果和密钥库口令相同, 按回车):  
[正在存储releasekey.keystore]
```
2. 用这个密钥存储器对一个 app 进行签名。
```
jarsigner -verbose -sigalg MD5withRSA -digestalg SHA1 -keystore [name of your keystore] [your .apk file] [your key alias]
```
`Keytool` 对公钥和私钥的实际处理方式是：把公钥放在 X.509 v3 证书中，该证书是用来声明公钥持有者的，并能验证相关公钥是否属于声明持有者的。

扩展阅读：
- http://docs.oracle.com/javase/6/docs/technotes/tools/windows/jarsigner.html
- http://docs.oracle.com/javase/6/docs/technotes/tools/solaris/keytool.html
- https://developer.android.com/studio/publish/app-signing.html

### 验证 app 的签名

```
jarsigner -verify -verbose [path-to-yout-apk]
```

### 探索 AndroidManifest.xml 文件

从 apk 包里提取出 AndroidManifest.xml 文件。
```
apktool d -f -s [apk file] -o decoded-data/
```
现在提取出来的文件就在 decoded-data 文件夹中。

扩展阅读：
https://developer.android.com/guide/topics/manifest/manifest-intro.html

### 通过 ADB 与 activity 管理器交互

1. 获取一个 shell
```
adb shell
```
2. 找一个要运行的 activity
```
pm list packages
```
3. 运行 activity
```
am start [package name]
```
4. 在运行 activity 之前，还可以通过使用 `start` 命令接收的 `intent` 参数，来指定传给 activity 的 intent。
```
am start <INTENT> < --user UID | current >
```
我们可以执行下面的或类似的命令：
```
am start -n com.android.MyPackage/
com.android.MyPackageLaunchMeActivity
-e MyInput HelloWorld -a android.intent.MyPackageIntentAction
-c android.intent.category.MyPackageIntentCategory
```
也可以使用 activity 管理器来启动服务：
```
am startservice <package name>/<component name> <INTENT>
```
还可以使用与下面命令类似的命令：
```
am startservice com.android.app/
com.android.app.service.ServiceComponent
```
当然，我们也可以使用 activity 管理器杀掉一个进程：
```
kill < --user UID | current > <package>
```

扩展阅读：
https://developer.android.com/studio/command-line/adb.html

### 通过 ADB 提取 app 里的资源

1. 获取一个 shell 并切换到 `/data/data/` 目录
```
adb shell
cd /data/data/
```
注意 `data` 目录的拥有者和组，拥有者实际上就是 app 本身。

2. 查看 app 的资源和元数据
```
ls -alR */
ls -alR */files/
ls -al */*/*.mp3
```
3. 找到文件之后，可以把文件复制出来
```
adb pull /data/data/[package-name]/[filepath]
```

## 第三章 Android 安全评估工具

### 简介

Santoku：基于 Debian 的 Linux 发行版，用于移动安全评估。

Drozer：漏洞利用和 Android 安全评估框架。Drozer 分为两部分，一个是 console ，运行在本地计算机上，一个是 server，是一个安装在目标 Android 设备上的 app。在使用 console 和 Android 设备交互时，就是把 Java 代码输入到运行在实际设备上的 drozer 代理中。

### 运行一个 drozer 会话

1. 使用 ADB，设置好端口转发
```
adb forward tcp:31415 tcp:31415
```
2. 打开设备上 drozer 的 agent，启用代理。
3. 连接 drozer console，进入 console模式
```
drozer console connect
```

### 枚举已安装的包

```
dz> run app.package.list

dz> run app.package.list -f [application name]

dz> run app.package.info --package [package name]
OR
dz> run app.package.info -a [package name]

dz> run app.package.info -p [permission label]
```

工作原理：
https://github.com/mwrlabs/drozer/blob/develop/src/drozer/modules/app/package.py

```
    def add_arguments(self, parser):
        parser.add_argument("-a", "--package", default=None, help="the identifier of the package to inspect")
        parser.add_argument("-d", "--defines-permission", default=None, help="filter by the permissions a package defines")
        parser.add_argument("-f", "--filter", default=None, help="keyword filter conditions")
        parser.add_argument("-g", "--gid", default=None, help="filter packages by GID")
        parser.add_argument("-p", "--permission", default=None, help="permission filter conditions")
        parser.add_argument("-u", "--uid", default=None, help="filter packages by UID")
        parser.add_argument("-i", "--show-intent-filters", action="store_true", default=False , help="show intent filters")

    def execute(self, arguments):
        if arguments.package == None:
            for package in self.packageManager().getPackages(common.PackageManager.GET_PERMISSIONS | common.PackageManager.GET_CONFIGURATIONS | common.PackageManager.GET_GIDS | common.PackageManager.GET_SHARED_LIBRARY_FILES | common.PackageManager.GET_ACTIVITIES):
               self.__get_package(arguments, package) 
        else:
            package = self.packageManager().getPackageInfo(arguments.package, common.PackageManager.GET_PERMISSIONS | common.PackageManager.GET_CONFIGURATIONS | common.PackageManager.GET_GIDS | common.PackageManager.GET_SHARED_LIBRARY_FILES | common.PackageManager.GET_ACTIVITIES)
            self.__get_package(arguments, package)
            
    def get_completion_suggestions(self, action, text, **kwargs):
        if action.dest == "permission":
            return android.permissions

    def __get_package(self, arguments, package):
        application = package.applicationInfo
        activities = package.activities
        services = package.services
```

只要在 drozer console 中使用 `app.activity.info` 模块，就会调用 `execute()` 方法。

我们看到它调用了包管理器中的 API －－ `self.packageManager().getPackages(...)`。这个包返回一个带有各个包的权限、配置、GID，以及共享库的所有包对象的列表(list)。这个脚本对列表中的每个对象调用一次 `self.__get_package()` 函数，把它打印到 drozer console 的屏幕上。

### 枚举 activity

```
dz> run app.activity.info

dz> run app.activity.info --filter [activity name]
OR
dz> run app.activity.info -f [activity name]

dz> run app.activity.info --package [package name]
OR
dz> run app.activity.info -a [package name]
```

扩展阅读：
https://github.com/mwrlabs/drozer/blob/develop/src/drozer/modules/app/activity.py

### 枚举 content provider

```
dz> run app.provider.info

dz> run app.provider.info --package [package name]
OR
dz> run app.provider.info -a [package name]

dz> run app.provider.info --permission [permission label]
OR
dz> run app.provider.info -p [permission label]
```

工作原理：
https://github.com/mwrlabs/drozer/blob/develop/src/drozer/modules/app/provider.py

```
    def execute(self, arguments):
        if arguments.package == None:
            for package in self.packageManager().getPackages(common.PackageManager.GET_PROVIDERS | common.PackageManager.GET_URI_PERMISSION_PATTERNS):
                self.__get_providers(arguments, package)
        else:
            package = self.packageManager().getPackageInfo(arguments.package, common.PackageManager.GET_PROVIDERS | common.PackageManager.GET_URI_PERMISSION_PATTERNS)

            self.__get_providers(arguments, package)


    def __get_providers(self, arguments, package):
        providers = self.match_filter(package.providers, 'authority', arguments.filter)        
        
        if arguments.permission != None:
            r_providers = self.match_filter(providers, 'readPermission', arguments.permission)
            w_providers = self.match_filter(providers, 'writePermission', arguments.permission)
            
            providers = set(r_providers + w_providers)
```

这个脚本通过调用 Android 包管理器，并传给它一些标志位提取出一个包的列表。我们看到，一旦包管理器收集到这些关于 `content proviser` 的详细信息后，脚本会调用一个名为 `__get_provider()` 方法，这个方法提取了 `provider` 的读和写的权限。`__get_provider()` 方法的作用基本上就是在定义了 `content provider` 权限的段中寻找一些字符串值，它调用 `math_filters()` 执行一些简单的字符串匹配，如果 `content provider` 所需的权限是读，这个字符串会被标上 `readPermission`；如果 `content provider` 所需权限是写，它会被标上 `writePermission`。之后，它会设置一个 provider 对象，然后把结果输出到 console 上。

扩展阅读：
https://developer.android.com/guide/topics/providers/content-providers.html

### 枚举 service

```
dz> run app.service.info

dz> run app.service.info --package [package name]

dz> run app.service.info --permission [permission label]
OR
dz> run app.service.info -p [permission label]

dz> run app.service.info --filter [filter string]
OR
dz> run app.service.info -f [filter string]

dz> run app.service.info --unexported
OR
dz> run app.service.info -u
```

工作原理：

```
    def execute(self, arguments):
        if arguments.package == None:
            for package in self.packageManager().getPackages(common.PackageManager.GET_SERVICES | common.PackageManager.GET_PERMISSIONS):
                self.__get_services(arguments, package)
        else:
            package = self.packageManager().getPackageInfo(arguments.package, common.PackageManager.GET_SERVICES | common.PackageManager.GET_PERMISSIONS)

            self.__get_services(arguments, package)
```

这个脚本会检查特定的包是否需要传入一个参数，如果不需要参数，或者包名已经定义，这个脚本会提取出一张包的列表，并在循环中，对其中的每个包调用一次 `self.__get_services()` 方法，在提取包列表时，根据对方法 `self.packageManager().getPackageInfo(arguments.package, common, PackageManager.GET_SERVICES | common.PackageManager.GET_PERMISSIONS)` 返回的数据进行字符串匹配的结果，可以对包的某些属性进行过滤。

扩展阅读：
- https://github.com/mwrlabs/drozer/blob/develop/src/drozer/modules/app/service.py
- https://developer.android.com/guide/components/services.html
- https://developer.android.com/reference/android/app/Service.html
- https://developer.android.com/guide/components/bound-services.html

### 枚举 broadcast receiver

```
dz> run app.broadcast.info

dz> run app.broadcast.info --package [package]
OR
dz> run app.broadcast.info -a [package]

dz> run app.broadcast.info --filter [filter]
OR
dz> run app.broadcast.info -f [filter]

dz> run app.broadcast.info --unexported
OR
dz> run app.broadcast.info -u
```

扩展阅读：
- https://developer.android.com/reference/android/content/BroadcastReceiver.html
- https://github.com/mwrlabs/drozer/blob/develop/src/drozer/modules/app/broadcast.py

### 确定 app 的受攻击面（attack surface）

一个 app 的受攻击面就是它导出组件的数量。

```
dz> run app.package.attacksurface [package name]
```

工作原理：
https://github.com/mwrlabs/drozer/blob/develop/src/drozer/modules/app/package.py

```
from drozer import android
from drozer.modules import common, Module
class AttackSurface(Module, common.Filters, common.PackageManager):

    def execute(self, arguments):
        if arguments.package != None:
            package = self.packageManager().getPackageInfo(arguments.package, common.PackageManager.GET_ACTIVITIES | common.PackageManager.GET_RECEIVERS | common.PackageManager.GET_PROVIDERS | common.PackageManager.GET_SERVICES)
            application = package.applicationInfo

            activities = self.match_filter(package.activities, 'exported', True)
            receivers = self.match_filter(package.receivers, 'exported', True)
            providers = self.match_filter(package.providers, 'exported', True)
            services = self.match_filter(package.services, 'exported', True)
            
            self.stdout.write("Attack Surface:\n")
            self.stdout.write("  %d activities exported\n" % len(activities))
            self.stdout.write("  %d broadcast receivers exported\n" % len(receivers))
            self.stdout.write("  %d content providers exported\n" % len(providers))
            self.stdout.write("  %d services exported\n" % len(services))

            if (application.flags & application.FLAG_DEBUGGABLE) != 0:
                self.stdout.write("    is debuggable\n")

            if package.sharedUserId != None:
                self.stdout.write("    Shared UID (%s)\n" % package.sharedUserId)
        else:
            self.stdout.write("No package specified\n")
```

这个模块会通过包管理器 API 提取关于 `service`、`activity`、`broadcast receiver` 和 `content provider` 的信息，然后根据得到的信息，确定它们是不是被导出。

### 运行 activity

1. 寻找一些 activity
```
dz> run app.activity.info --package [package name]
```
2. 发送如何运行的 intent
```
dz> run app.activity.start --action [intent action] --category [intent category] --component [package name] [component name]

dz> run app.activity.forintent --action [intent action] -category [intent category]
```

工作原理：
https://raw.githubusercontent.com/mwrlabs/drozer/develop/src/drozer/modules/app/activity.py

```
    def execute(self, arguments):
        intent = android.Intent.fromParser(arguments)

        if len(intent.flags) == 0:
            intent.flags.append('ACTIVITY_NEW_TASK')
        
        if intent.isValid():
            self.getContext().startActivity(intent.buildIn(self))
        else:
            self.stderr.write("invalid intent: one of action or component must be set\n")
```

drozer 把通过参数解析器拿到的用户输入的参数放到一个 `intent` 里，检查是否有效后，就发送出去。

扩展阅读：
- https://developer.android.com/reference/android/content/Intent.html
- https://developer.android.com/guide/components/intents-filters.html
- https://developer.android.com/guide/components/activities.html

编写扩展：
- https://developer.android.com/reference/android/os/Build.html
- https://github.com/mwrlabs/drozer/wiki/Writing-a-Module

## 第四章 利用 app 中的漏洞

### 收集 logcat 泄漏的信息

```
adb logcat [options] [filter]

adb logcat > output.txt

adb logcat | grep [pattern]
```
Android 的 Monkey testing 框架用来向 app 发送系统/硬件级事件。
```
adb shell monkey -p [package] -v [event count]
```

扩展阅读：
- https://developer.android.com/studio/command-line/logcat.html
- http://blog.parse.com/learn/engineering/discovering-a-major-security-hole-in-facebooks-android-sdk/
- https://developer.android.com/studio/command-line/adb.html#logcat
- http://www.vogella.com/tutorials/AndroidTesting/article.html

### 检查网络流量

确认 tcpdump 和 netcat 已经安装在 Android 设备后，可以抓取网络流量
```
tcpdump -w - | nc -l -p 31337
```
把 tcpdump 的输出传给本地计算机上的 Wireshark，首先通过 ADB 设置端口转发
```
adb forward tcp:12345 tcp:31337
```
最后通过管道把输出转到 Wireshark 里
```
./adb forward tcp:12345 tcp:31337 && netcat 127.0.0.1 12345 | wireshark -k -S -i -
```

扩展阅读：
- https://code.tutsplus.com/tutorials/analyzing-android-network-traffic--mobile-10663
- https://wiki.wireshark.org/DisplayFilters
- https://wiki.wireshark.org/CaptureFilters
- http://www.tcpdump.org/tcpdump_man.html
- https://www.wireshark.org/docs/wsug_html_chunked/

### 攻击 service

1. 针对给定的 app，寻找哪些 service 是导出的
```
dz> run app.service.info --permission null
```
2. 找到一堆 service 之后，使用下列命令运行它们
```
dz> run app.service.start --action [ACTION] --category [CATEGORY] --data-uri [DATA-URI] --component [package name] [component name] --extra [TYPE KEY VALUE] --mimetype [MIMETYPE]
```
在停止和启动这些 service 时，可以同时运行 `logcat`。

3. 从根本上讲，在 XML 文件中寻找下列部分
```
<action
<meta-data
```
4. 为了把 intent 发送给这个 service，可以在 drozer console 中执行命令
```
// 以 `com.linkedin.android` 为例子
dz> run app.service.start --component com.linkedin.android com.linkedin.android.authenticator.AuthenitactionService --action android.accounts.AccountAuthenitcator
```

扩展阅读：
https://web.nvd.nist.gov/view/vuln/detail?vulnId=CVE-2011-4276&cid=6

### 攻击 broadcast receiver

在阅读漏洞源码时特备注意 broadcast receiver 中的 `intent filter` 的定义。

向 broadcast receiver 发送一个 intent

```
dz> run app.broadcast.send --action [ACTION] --category [CATEGORY] --component [PACKAGE COMPONENT] --data-uri [DATA-URI] -extra [TYPE KEY VALUE] -flags [FLAGS*] -mimetype [MIMETYPE]
```

扩展阅读：
- http://www.cs.wustl.edu/~jain/cse571-11/ftp/trojan/index.html
- https://blog.lookout.com/blog/2010/08/10/security-alert-first-android-sms-trojan-found-in-the-wild/

### 枚举有漏洞的 content provider

1. 枚举不需要权限的 content provider
```
dz> run app.provider.info --permission null
```
2. 选中一个 content provider 后，列出所有冠有主机名的 URI
```
dz> run app.provider.finduri [package]
```
工作原理：
```
    def findContentUris(self, package):
        """
        Search a package for content providers, by looking for content:// paths
        in the binary.
        """

        self.deleteFile("/".join([self.cacheDir(), "classes.dex"]))

        content_uris = []
        for path in self.packageManager().getSourcePaths(package):
            strings = []

            if ".apk" in path:
                dex_file = self.extractFromZip("classes.dex", path, self.cacheDir())

                if dex_file != None:
                    strings = self.getStrings(dex_file.getAbsolutePath())

                    dex_file.delete()
                
                # look for an odex file too, because some system packages do not
                # list these in sourceDir
                strings += self.getStrings(path.replace(".apk", ".odex")) 
            elif (".odex" in path):
                strings = self.getStrings(path)
            
            content_uris.append((path, filter(lambda s: ("CONTENT://" in s.upper()) and ("CONTENT://" != s.upper()), strings)))

        return content_uris
```
`.finduri` 模块列出所有可能的 content URI 的方法：打开 app 的 DEX 文件，直接在未经解析的文件中寻找那些看上去像是有效的 content URI 格式的字符串。

扩展阅读：
- https://github.com/mwrlabs/drozer/blob/develop/src/drozer/modules/common/provider.py
- https://github.com/mwrlabs/drozer/blob/develop/src/drozer/modules/app/provider.py
- https://developer.android.com/guide/topics/security/permissions.html#uri
- https://web.nvd.nist.gov/view/vuln/detail?vulnId=CVE-2013-2318&cid=3

### 从有漏洞的 content provider 中提取数据

1. 得到有漏洞的 content provider
```
run app.provider.info --permission null
run app.provider.finduri [package]
```
2. 提取、下载数据
```
dz> run app.provider.query [URI]
dz> run app.provider.download [URI]
```

扩展阅读：
- http://www.cvedetails.com/cve/CVE-2010-4804/
- http://vuln.sg/winzip101-en.html

### 向 content provider 插入数据

1. 列出数据的结构和各列的名称等信息
```
dz> run app.provider.columns [URI]
```
2. 插入数据
```
dz> run app.provider.insert [URI] [--boolean [name] [value]] [--integer [name] [value]] [--string [name] [value]]...
```

### 枚举有 SQL 注入漏洞的 content provider

```
dz> run app.provider.query [URI] --selection "1=1"
dz> run app.provider.query [URI] --selection "1-1=0"
......
```

扩展阅读：
- http://www.sqlite.org/lang.html
- https://www.owasp.org/index.php/SQL_Injection

### 利用可调试的 app

1. 检查一个 app 是不是可调试的，可以直接查看在 app 中的 manifest，也可以执行
```
dz> run app.package.debuggable
```
2. 运行
```
dz> run app.activity.start --component com.example.readmycontacts com.example.readmycontacts.MainActivity
```
3. 可以使用 ADB 连接 java 调试连接协议端口，它是一个在虚拟机实例上打开的专供调试使用的端口，返回可以连接 VM 的接口
```
adb jdwp
```
4. 用 ADB 转发端口
```
adb forward tcp:[localport] jdwp:[jdwp port on device]
```
5. 从本地计算机使用 Java 调试器连上 VM
```
jdb -attach localhost:]PORT]
```
6. 提取类信息
```
classes
```
7. 枚举指定类中的所有方法
```
> methods [class-path]
```
8. 列出类的域或类属性的名称和值，在 jdb 中执行
```
> fields [class name]
```

扩展阅读：
- http://docs.oracle.com/javase/1.5.0/docs/tooldocs/windows/jdb.html
- http://docs.oracle.com/javase/1.5.0/docs/guide/jpda/index.html
- https://developer.android.com/guide/topics/manifest/application-element.html#debug
- https://labs.mwrinfosecurity.com/blog/debuggable-apps-in-android-market/
- http://www.saurik.com/id/17
- https://www.packtpub.com/books/content/debugging-java-programs-using-jdb
- https://android.googlesource.com/platform/dalvik/+/gingerbread-release/vm/jdwp/JdwpAdb.c
- https://android.googlesource.com/platform/dalvik/+/eclair-passion-release/vm/jdwp/JdwpAdb.c
- https://android.googlesource.com/platform/dalvik/+/kitkat-release/vm/jdwp/JdwpAdb.cpp

## 第五章 保护 app

### 包含 app 的组件

两种方法：其一是正确使用 `AndroidManifest.xml` 文件，其二是代码级别上强制进行权限检查。

如果某个组件不需要被其他 app 调用，或者需要明确与其他部分的组件隔离，在该组件的 XML 元素中加入下面属性
```
<[component name] android:exported="false">
</[component name]>
```

扩展阅读：
- https://developer.android.com/guide/topics/manifest/service-element.html
- https://developer.android.com/guide/topics/manifest/receiver-element.html
- https://developer.android.com/guide/topics/manifest/activity-element.html
- https://developer.android.com/guide/topics/manifest/application-element.html
- https://developer.android.com/guide/topics/manifest/manifest-intro.html
- https://developer.android.com/reference/android/content/Context.html
- https://developer.android.com/reference/android/app/Activity.html

### 通过定制权限保护组件

1. 声明表示 `permission` 标签的字符串，编辑 `res/values/strings.xml` 文件
```
<string name="custom_permission_label">Custom Permission</string>
```
2. 添加保护级别为 `normal` 的定制权限，在 `AndroidManifest.xml` 文件中加入下列字符串
```
<permission android:name="android.permission.CUSTOM_PERMISSION"
    android:protectionLevel="normal"
    android:description="My custom permission"
    android:label="@string/custom_permission_label">
```
3. 把它添加到 app 某个组件的 `android:permission` 属性中
```
<activity ...
    android:permission="android.permission.CUSTOM_PERMISSION">
</activity>
```
也可以是 `provider`、`service`、`receiver`。

4. 也可以在其他 app 的 `AndroidManifest.xml` 文件中添加标签
```
<uses-permission android:name="android.permission.CUSTOM_PERMISSION"/>
```

定义权限组：

1. 在 `res/values/string.xml` 中添加一个表示权限组标签的字符串
```
<string name="my_permissions_group_label">Personal Data Access</string>
```
2. 在 AndroidManifest.xml 中添加代码
```
<permission-group
    android:name="android.permissions.persomal_data_access_group"
    android:label="@string/my_permissions_group_label"
    android:description="Permissions that allow access to personaldata" />
```
3. 把定义的权限分配到组中
```
<permission ...
    android:permissionGroup="android.permission.personal_data_access_group" />
```

扩展阅读：
- https://developer.android.com/guide/topics/manifest/permission-element.html
- https://developer.android.com/guide/topics/manifest/uses-permission-element.html
- https://developer.android.com/guide/topics/manifest/permission-group-element.html
- https://developer.android.com/reference/android/Manifest.permission.html

### 保护 content provider 的路径

1. 设置一个用于管理所有与你的认证相关路径的读和写权限的 permission，在 `manifest` 中添加下面的元素，其中的 `[permission name]` 是其他 app 在读或写任何 content provider 路径时必须拥有的权限。
```
<provider android:enabled="true"
    android:exported="true"
    android:authorities="com.android.myAuthority"
    android:permission="com.myapp.provider"
    android:permission="[permission name]">
</provider>
```
2. 加上读和写的权限。
```
<provider
    android:writePermission="[write permission name]"
    android:readPermission="[read permission name]">
</provider>
```

扩展阅读：
- https://developer.android.com/guide/topics/manifest/provider-element.html
- https://developer.android.com/guide/topics/manifest/path-permission-element.html

### 防御 SQL 注入攻击

1. 实例化一个 RssItemDAO 对象时，把 insertStatement 对象编译成一个参数化的 SQL insert 语句字符串
```
public class RssItemDAO {

private SQLiteDatabase db;
private SQLiteStatement insertStatement;

private static String COL_TITLE = "title";
private static String TABLE_NAME = "RSS_ITEMS";

private static String INSERT_SQL = "insert into " + TABLE_NAME + " (content, link, title) values (?,?,?)";

public RssItemDAO(SQLiteDatabase db) {
    this.db = db;
    insertStatement = db.compileStatement(INSERT_SQL);
}
```
2. 把一个新的 RssItem 对象插入到数据库中时，可以按语句中出现的顺序，绑定各个属性
```
public long save(RssItem item) {
    insertStatement.bindString(1, item.getContent());
    insertStatement.bindString(2, item.getLink());
    insertStatement.bindString(3, item.getTitle());
    return insertStatement.executeInsert();
}
```
3. 展示了如何使用 SQLiteDatabase.query 去获取与指定搜索条件相匹配的 RssItems
```
public List<RssItem> fetchRssItemsByTitle(String searchTerm) {
    Cursor cursor = db.query(TABLE_NAME, null, COL_TITLE + "LIKE ?", new String[] { "%" + searchTerm + "%" }, null, null, null);
    // process cursor into list
    List<RssItem> rssItems = new ArrayList<RssItemDAO.RssItem>();
    cursor.moveToFirst();
    while (!cursor.isAfterLast()) {
    // maps cursor columns of RssItem properties
        RssItem item = cursorToRssItem(cursor);
        rssItems.add(item);
        cursor.moveToNext();
    }
    return rssItems;
}
```

扩展阅读：
- https://developer.android.com/reference/android/database/sqlite/SQLiteDatabase.html
- https://developer.android.com/reference/android/database/sqlite/SQLiteStatement.html
- https://www.owasp.org/index.php/Query_Parameterization_Cheat_Sheet
- http://www.sqlite.org/lang_expr.html

### 验证 app 的签名

确保已经有一个签名私钥，没有的话使用下面的方法创建
```
keytool -genkey -v -keystore your_app.keystore -alias alias_name -keyalg RSA -keysize 2048 -validity 10000
```
1. 找出你证书的 SHA1 签名
```
keytool -list -v -keystore your_app.keystore

// for example
71:92:0A:C9:48:6E:08:7D:CB:CF:5C:7F:6F:EC:95:21:35:85:BC:C5 :
```
2. 把 hash 复制到 app 中，在 Java `.class` 文件中，删除冒号并把它定义为一个静态字符串
```
private static String CERTIFICATE_SHA1 = "71920AC9486E087DCBCF5C7F6FEC95213585BCC5";
```
3. 编写在运行时获取 .apk 文件的当前签名的代码
```
public static boolean validateAppSignature(Context context) {
    try {
        // get the signature form the package manager
        PackageInfo packageInfo = context.getPackageManager().getPackageInfo(context.getPackageName(), PackageManager.GET_SIGNATURES);
        Signature[] appSignatures = packageInfo.signatures;
        
        //this sample only checks the first certificate
        for (Signature signature : appSignatures) {
            byte[] signatureBytes = signature.toByteArray();
            //calc sha1 in hex
            String currentSignature = calcSHA1(signatureBytes);
            //compare signatures
            return CERTIFICATE_SHA1.equalsIgnoreCase(currentSignature);
        }
    } catch (Exception e) {
    // if error assume failed to validate
    }
    return false;
}
```
4. 把签名的 hash 存下来，把它转换成十六进制
```
private static String calcSHA1(byte[] signature) throws NoSuchAlgorithmException {
    MessageDigest digest = MessageDigest.getInstance("SHA1");
    digest.update(signature);
    byte[] signatureHash = digest.digest();
    return bytesToHex(signatureHash);
}
public static String bytesToHex(byte[] bytes) {
    final char[] hexArray = { '0', '1', '2', '3', '4', '5', '6', '7', '8','9', 'A', 'B', 'C', 'D', 'E', 'F' };
    char[] hexChars = new char[bytes.length * 2];
    int v;
    for (int j = 0; j < bytes.length; j++) {
        v = bytes[j] & 0xFF;
        hexChars[j * 2] = hexArray[v >>> 4];
        hexChars[j * 2 + 1] = hexArray[v & 0x0F];
    }
    return new String(hexChars);
}
```
5. 比较
```
CERTIFICATE_SHA1.equalsIgnoreCase(currentSignature);
```

扩展阅读：
- https://developer.android.com/studio/publish/app-signing.html
- https://gist.github.com/scottyab/b849701972d57cf9562e
- https://developer.android.com/reference/android/content/pm/Signature.html
- https://developer.android.com/reference/android/content/pm/PackageManager.html
- http://www.saurik.com/id/17
- http://docs.oracle.com/javase/6/docs/technotes/tools/windows/keytool.html

### 通过检测安装程序、模拟器、调试标志位反逆向工程

1. 检查安装程序是不是谷歌应用商店
```
public static boolean checkGooglePlayStore(Context context) {
    String installerPackageName = context.getPackageManager().getInstallerPackageName(context.getPackageName());
    return installerPackageName != null && installerPackageName.startsWith("com.google.android");
}
```
2. 检查是不是运行在一台模拟器中
```
public static boolean isEmulator() {
    try {
        Class systemPropertyClazz = Class.forName("android.os.SystemProperties");
        boolean kernelQemu = getProperty(systemPropertyClazz, "ro.kernel.qemu").length() > 0;
        boolean hardwareGoldfish = getProperty(systemPropertyClazz, "ro.hardware").equals("goldfish");
        boolean modelSdk = getProperty(systemPropertyClazz, "ro.product.model").equals("sdk");
        
        if (kernelQemu || hardwareGoldfish || modelSdk) {
            return true;
        }
    } catch (Exception e) {
    // error assumes emulator
    }
    return false;
}

private static String getProperty(Class clazz, String propertyName) throws Exception {
    return (String) clazz.getMethod("get", new Class[] { String. class }).invoke(clazz, new Object[] { propertyName });
}
```
3. 检查可调试标志位是否被打开
```
public static boolean isDebuggable(Context context){
    return (context.getApplicationInfo().flags & ApplicationInfo.FLAG_DEBUGGABLE) != 0;
}
```

扩展阅读：
- https://github.com/android/platform_frameworks_base/blob/master/core/java/android/os/SystemProperties.java
- https://developer.android.com/reference/android/content/pm/PackageManager.html
- https://developer.android.com/reference/android/content/pm/ApplicationInfo.html

### 使用 ProGuad

1. Android Studio下，需要在 Gradle Build 系统中 `buildType` 的 release 部分加入下面代码
```
android {
...
    buildTypes {
        release {
            runProguard true
            proguardFile file('../proguard-project.txt)
            proguardFile getDefaultProguardFile('proguard-android.txt')
        }
    }
}
```
2. 保证 `proGuard-android.txt` 文件始终在配置中规定的位置。

扩展阅读：
- https://developer.android.com/studio/build/shrink-code.html
- https://sourceforge.net/projects/proguard/
- http://proguard.sourceforge.net/index.html#manual/examples.html

## 第六章 逆向 app

### 把 Java 源码编译成 DEX 文件

1. 打开文本编辑器，创建一个文件
```
public class Example{
    public static void main(String []args){
        System.out.printf("Hello World!\n");
    }
}
```
2. 编译得到 .class 文件
```
javac -source 1.6 -target 1.6 Example.java
```
3. 使用 dx 得到一个 DEX 文件
```
/sdk/build-tools/25.0.0/dx --dex --output=Example.dex Example.class
```

### 解析 DEX 文件格式

https://github.com/android/platform_dalvik/blob/master/libdex/DexFile.h

DEX 文件的格式：
```
struct DexFile {
    /* directly-mapped "opt" header */
    const DexOptHeader* pOptHeader;

    /* pointers to directly-mapped structs and arrays in base DEX */
    const DexHeader*    pHeader;
    const DexStringId*  pStringIds;
    const DexTypeId*    pTypeIds;
    const DexFieldId*   pFieldIds;
    const DexMethodId*  pMethodIds;
    const DexProtoId*   pProtoIds;
    const DexClassDef*  pClassDefs;
    const DexLink*      pLinkData;

    /*
     * These are mapped out of the "auxillary" section, and may not be
     * included in the file.
     */
    const DexClassLookup* pClassLookup;
    const void*         pRegisterMapPool;       // RegisterMapClassPool

    /* points to start of DEX file data */
    const u1*           baseAddr;

    /* track memory overhead for auxillary structures */
    int                 overhead;

    /* additional app-specific data structures associated with the DEX */
    //void*               auxData;
};
```
下面我们分别来分析各个区段：

DEX 文件头：
```
struct DexHeader {
    u1  magic[8];           /* includes version number */
    u4  checksum;           /* adler32 checksum */
    u1  signature[kSHA1DigestLen]; /* SHA-1 hash */
    u4  fileSize;           /* length of entire file */
    u4  headerSize;         /* offset to start of next section */
    u4  endianTag;
    u4  linkSize;
    u4  linkOff;
    u4  mapOff;
    u4  stringIdsSize;
    u4  stringIdsOff;
    u4  typeIdsSize;
    u4  typeIdsOff;
    u4  protoIdsSize;
    u4  protoIdsOff;
    u4  fieldIdsSize;
    u4  fieldIdsOff;
    u4  methodIdsSize;
    u4  methodIdsOff;
    u4  classDefsSize;
    u4  classDefsOff;
    u4  dataSize;
    u4  dataOff;
};
```
```
/*
 * These match the definitions in the VM specification.
 */
typedef uint8_t             u1;
typedef uint16_t            u2;
typedef uint32_t            u4;
typedef uint64_t            u8;
typedef int8_t              s1;
typedef int16_t             s2;
typedef int32_t             s4;
typedef int64_t             s8;
```
StringIds 区段：

由一系列相对 DEX 文件的加载基地址的偏移量组成，用于计算定义在 Data 区段中的各个静态字符串的起始位置。
```
struct DexStringId {
    u4 stringDataOff;      /* file offset to string_data_item */
};
```
编译器、反编译器、虚拟机如何寻找字符串：
```
/* return the const char* string data referred to by the given string_id */
DEX_INLINE const char* dexGetStringData(const DexFile* pDexFile,
        const DexStringId* pStringId) {
    const u1* ptr = pDexFile->baseAddr + pStringId->stringDataOff;

    // Skip the uleb128 length.
    while (*(ptr++) > 0x7f) /* empty */ ;

    return (const char*) ptr;
}
```
TypeIds 区段：存放寻找各个 type 的相应字符串时需要的信息。
```
struct DexTypeId {
    u4  descriptorIdx;      /* index into stringIds list for type descriptor */
};
```
ProtoIds 区段：存放一系列用来描述方法的 prototype ID，其中含有关于各个方法的返回类型和参数的信息。
```
struct DexProtoId {
    u4  shortyIdx;          /* index into stringIds for shorty descriptor */
    u4  returnTypeIdx;      /* index into typeIds list for return type */
    u4  parametersOff;      /* file offset to type_list for parameter types */
};
```
FieldIds 区段：由 StringIds 和 TypeIds 区段中数据的索引号组成，描述类中各个成员。
```
struct DexFieldId {
    u2  classIdx;           /* index into typeIds list for defining class */
    u2  typeIdx;            /* index into typeIds for field type */
    u4  nameIdx;            /* index into stringIds for field name */
};
```
MethodIds 区段：
```
struct DexMethodId {
    u2  classIdx;           /* index into typeIds list for defining class */
    u2  protoIdx;           /* index into protoIds for method prototype */
    u4  nameIdx;            /* index into stringIds for method name */
};
```
ClassDefs 区段：
```
struct DexClassDef {
    u4  classIdx;           /* index into typeIds for this class */
    u4  accessFlags;
    u4  superclassIdx;      /* index into typeIds for superclass */
    u4  interfacesOff;      /* file offset to DexTypeList */
    u4  sourceFileIdx;      /* index into stringIds for source file name */
    u4  annotationsOff;     /* file offset to annotations_directory_item */
    u4  classDataOff;       /* file offset to class_data_item */
    u4  staticValuesOff;    /* file offset to DexEncodedArray */
};
```
下面我们用 dexdump 工具解析 DEX：
```
/sdk/build-tools/25.0.0/dexdump Example.dex
```
还可以用 dx 工具，用更接近 DEX 文件格式的方法解析：
```
dx --dex --verbose-dump -dump-to=[output-file].txt [input-file].class
```

扩展阅读：
- http://retrodev.com/android/dexformat.html
- https://github.com/JesusFreke/smali
- http://www.strazzere.com/papers/DexEducation-PracticingSafeDex.pdf
- https://github.com/android/platform_dalvik/tree/master/libdex
- http://source.android.com/devices/tech/dalvik/dex-format.html

### 解释 Dalvik 字节码

使用 baksmali 工具把 DEX 文件反编译成语法格式良好的 smali 文件。
```
baksmali [dex filename].dex
```

扩展阅读：
- http://source.android.com/devices/tech/dalvik/instruction-formats.html
- http://source.android.com/devices/tech/dalvik/dalvik-bytecode.html

### 把 DEX 反编译回 Java

1. 使用 Dex2jar 工具把 DEX 文件转换成 .class 文件
```
dex2jar [dex file].dex
```
2. 使用 jd-gui 工具得到源文件

### 反编译 app 的原生库

Android NDK 中 toolchain 里提供的 objdump。
```
objdump -D [native library].so
```

扩展阅读：
- http://www.atmel.com/Images/DDI0029G_7TDMI_R3_trm.pdf
- http://infocenter.arm.com/help/topic/com.arm.doc.ihi0044f/IHI0044F_aaelf.pdf
- http://www.arm.com/products/processors/instruction-set-architectures/index.php
- http://refspecs.linuxbase.org/elf/elf.pdf

### 使用 GDB server 调试 Android 进程

1. 确保设备上安装了 gdbserver，然后查看正在运行的所有进程
```
ps
```
2. 得到一个有效的 PID 后，使用 gdbserver 附加它
```
gdbserver :[tcp-port number] --attach [PID]
```
3. 在本地计算机上执行端口转发
```
adb forward tcp:[device port-number] tcp:[local port-number]
```
4. 运行 gdb 后（NDK toolchains）
```
target remote :[PID]
```

## 第七章 网络安全
- https://developer.android.com/training/articles/security-ssl.html
- http://www.bouncycastle.org/latest_releases.html
- https://developer.android.com/reference/javax/net/ssl/HttpsURLConnection.html
- https://developer.android.com/reference/javax/net/ssl/SSLSocketFactory.html
- https://guardianproject.info/code/onionkit/
- https://play.google.com/store/apps/details?id=org.torproject.android
- https://code.google.com/archive/p/httpclientandroidlib/
- https://github.com/guardianproject/cacert
- https://www.owasp.org/index.php/Man-in-the-middle_attack
- https://www.madboa.com/geek/openssl/
- https://www.owasp.org/index.php/Certificate_and_Public_Key_Pinning
- https://github.com/moxie0/AndroidPinning
- https://www.imperialviolet.org/2011/05/04/pinning.html

## 第八章 原生代码中漏洞的利用与分析

### 检查文件权限

首先安装 busybox 工具。

列出所有用户都可读的文件：
```
./busybox find [path-to-search] -perm 0444 -exec ls -al {} \;
```
所有用户都可写的文件：
```
./busybox find [path-to-search] -perm 0222 -exec ls -al {} \;
```
所有用户都可执行的文件：
```
./busybox find [path-to-search] -perm 0111 -exec ls -al {} \;
```
如果要找规定的位被设为1，其他的位设不设都没关系的文件，可以在搜索权限的参数前面加上一个 `"-"` 符号作为前缀。

找出 setuid 位被设为1的可执行文件：
```
./busybox find [path-to-search] -perm -4111 -exec ls -al {} \;
```
找出所有 setguid 位和可执行位都被设为1的文件：
```
./busybox find [path-to-search] -perm -2111 -exec ls -al {} \;
```
找出所有属于 root 用户的文件：
```
./busybox find [path-to-search] -user 0 -exec ls -al {} \;
```
找出所有系统用户的文件：
```
./busybox find [path-to-search] -user 1000 -exec ls -al {} \;
```
根据组 ID 列出文件：
```
./busybox find [path-to-search] -group 0 -exec ls -al {} \;
```
建立各个 app 的 UID 列表：
```
ls -al /data/data/
```
我们看到 app 的命名格式为 `u[数字]_a[数字]`，具体意思为 `u[用户配置文件号]_a[ID]`，在 `ID` 上加上10000，就得到系统中实际使用的 `UID`。

找出该 UID 的用户的所有文件：
```
./busybox find /data/data/ -user [UID] -exec ls -al {} \;
```

扩展阅读：
- https://web.nvd.nist.gov/view/vuln/detail?vulnId=CVE-2009-1894
- https://android.googlesource.com/platform/system/core/+/android-4.4.2_r1/include/private/android_filesystem_config.h
- http://www.tldp.org/HOWTO/HighQuality-Apps-HOWTO/fhs.html
- http://www.pathname.com/fhs/pub/fhs-2.3.pdf

### 交叉编译原生可执行程序

使用 Android 原生开发工具包（NDK）。

1. 为代码建一个目录，例如“buffer-overflow”，这个目录下必须建立一个名为 `jni` 的子目录，因为 NDK 的编译脚本会专门寻找这个目录。
2. 在 JNI 目录下创建一个 Android.mk 文件，记录一些编译相关的属性。
```
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
# give module name
LOCAL_MODULE := buffer-overflow #name of folder
# list your C files to compile
LOCAL_SRC_FILES := buffer-overflow.c #name of source to compile
# this option will build executables instead of building library for Android application.
include $(BUILD_EXECUTABLE)
```
3. 示例代码，保存在 jni 目录中：
```
#include<stdio.h>
#include<string.h>
void vulnerable(char *src) {
    char dest[10];
    strcpy(dest, src);
    printf("[%s]\n", dest);
    return;
}
void call_me_maybe() {
    printf("so much win!!\n");
    return;
}
int main(int argc, char **argv) {
    vulnerable(argv[1]);
    return 0;
}
```
4. 调用 NDK build 脚本完成编译
```
[path-to-ndk]/ndk-build
```
5. 用 NDK 特定版本的 objdump 解析这个可执行文件的汇编代码
```
[path-to-ndk]/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-objdump -D buffer-overflow/obj/local/armeabi/buffer-overflow > buffer-overflow.txt
```

扩展阅读：
- http://infocenter.arm.com/help/topic/com.arm.doc.qrc0001l/QRC0001_UAL.pdf
- http://simplemachines.it/doc/arm_inst.pdf
- http://101.96.8.164/web.eecs.umich.edu/~prabal/teaching/eecs373-f10/readings/ARMv7-M_ARM.pdf
- https://www.exploit-db.com/docs/16151.pdf
- http://101.96.8.165/infocenter.arm.com/help/topic/com.arm.doc.ihi0042e/IHI0042E_aapcs.pdf
- https://www.exploit-db.com/docs/16151.pdf
- http://infocenter.arm.com/help/topic/com.arm.doc.ihi0042f/IHI0042F_aapcs.pdf
- http://infocenter.arm.com/help/topic/com.arm.doc.dui0068b/DUI0068.pdf
- https://android.googlesource.com/platform/bionic/
- https://android.googlesource.com/platform/bionic/+/jb-mr0-release/libc/bionic/dlmalloc.c

### 利用竞争条件引发的漏洞

竞争条件问题是当进程运行在使用抢占式进程调度系统的多线程系统中时，由于缺乏强制互斥条件而引发的。抢占式调度允许任务调度器强制中断一个正在运行中的线程或进程。

要利用竞争条件漏洞，必须至少具备一下条件：
1. 能访问存在漏洞的进程所要竞争访问的资源
2. 恶意改变这些资源
3. 使用时间/检查时间（TOU/TOC）的窗口大小

我们先准备一个例子，使用 Jelly Bean 模拟器：
```c
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#define MAX_COMMANDSIZE 100
int main(int argc, char *argv[], char **envp) {
    char opt_buf[MAX_COMMANDSIZE];
    char *args[2];
    args[0] = opt_buf;
    args[1] = NULL;
    int opt_int;
    const char *command_filename = "/data/race-condition/commands.txt";
    FILE *command_file;
    printf("option: ");
    opt_int = atoi(gets(opt_buf));
    printf("[*] option %d selected...\n", opt_int);
    if (access(command_filename, R_OK|F_OK) == 0) {
        printf("[*] access okay...\n");
        command_file = fopen(command_filename, "r");
        for (; opt_int>0; opt_int--) {
            fscanf(command_file, "%s", opt_buf);
        }
        printf("[*] executing [%s]...\n", opt_buf);
        fclose(command_file);
    }
    else {
        printf("[x] access not granted...\n");
    }
    int ret = execve(args[0], &args, (char **)NULL);
    if (ret != NULL) {
        perror("[x] execve");
    }
    return 0;
}
```
1. 用交叉编译原生可执行程序的方法编译，并部署到 Android 设备上，把它放在系统上一个任何用户都有可读和执行权限的目录下。
2. 在 `/data/` 下新建一个 `race-condition` 文件夹，并把 `command.txt` 文件进去。
3. 在把它复制到设备后，赋予这个可执行文件 `setuid` 权限，使任何用户都对这个文件拥有执行权限。
```
chmod 4711 /system/bin/race-condition
```
4. 用 `ADB shell` 登陆，尝试访问一些没有执行、读取或写入权限的属于 root 的文件，显然不行
5. 我们利用 `race-condition` 这个可执行文件，先向 `commands.txt` 文件写入命令
```
echo "/system/bin/sh" >> /data/race-condition/commands.txt
```
命令被写入到最后一行，可以用 `cat [filename]` 查看文件并记下刚写入的命令是第几行。

6. 执行 `race-condition`，在要求输入 `option` 时输入刚才的行数，这个有漏洞的二进制可执行文件会去执行 `sh` 命令并给你 `root` 权限。

扩展阅读：
- https://web.nvd.nist.gov/view/vuln/detail?vulnId=CVE-2013-1727&cid=8
- https://web.nvd.nist.gov/view/vuln/detail?vulnId=CVE-2013-1731&cid=8
- https://packetstormsecurity.com/files/122145/Sprite-Software-Android-Race-Condition.html
- http://www.wright.edu/static/cats/maint/maintenance.html

### 栈溢出漏洞的利用

- https://www.exploit-db.com/docs/24493.pdf
- http://phrack.org/issues/49/14.html#article
- http://thinkst.com/stuff/bh10/BlackHat-USA-2010-Meer-History-of-Memory-Corruption-Attacks-wp.pdf
- http://cseweb.ucsd.edu/~hovav/dist/noret-ccs.pdf
- https://www.informatik.tu-darmstadt.de/fileadmin/user_upload/Group_TRUST/PubsPDF/ROP-without-Returns-on-ARM.pdf

### 自动 fuzzing 测试 Android 原生代码

- http://www.linuxjournal.com/article/10844
- https://code.google.com/archive/p/ouspg/wikis/Radamsa.wiki
- https://code.google.com/archive/p/ouspg/wikis/Blab.wiki
- http://gcc.gnu.org/onlinedocs/gcc/Code-Gen-Options.html
- http://www.cs.tut.fi/tapahtumat/testaus12/kalvot/Wieser_20120606radamsa-coverage.pdf

## 第九章 加密与在开发时使用设备管理策略
- https://www.owasp.org/index.php/Using_the_Java_Cryptographic_Extensions
- http://rtyley.github.io/spongycastle/#downloads
- http://www.bouncycastle.org/java.html
- http://cs.ucsb.edu/~yanick/publications/2013_ccs_cryptolint.pdf
- https://developer.android.com/reference/java/security/SecureRandom.html
- https://developer.android.com/reference/javax/crypto/KeyGenerator.html
- http://www.openhandsetalliance.com/oha_members.html
- https://developer.android.com/reference/android/content/SharedPreferences.html
- http://www.codeproject.com/Articles/549119/Encryption-Wrapper-for-Android-SharedPreferences
- https://github.com/scottyab/secure-preferences
- https://github.com/commonsguy/cwac-prefs
- https://developer.android.com/reference/javax/crypto/SecretKeyFactory.html
- https://developer.android.com/reference/javax/crypto/spec/PBEKeySpec.html
- http://docs.oracle.com/javase/6/docs/technotes/guides/security/crypto/CryptoSpec.html
- http://android-developers.blogspot.co.uk/2013/02/using-cryptography-to-store-credentials.html
- https://github.com/nelenkov/android-pbe
- https://www.zetetic.net/sqlcipher/buy/
- https://github.com/sqlcipher/android-database-sqlcipher
- https://guardianproject.info/code/iocipher/
- https://developer.android.com/reference/java/security/KeyStore.html
- https://developer.android.com/samples/BasicAndroidKeyStore/index.html
- http://nelenkov.blogspot.co.uk/2013/08/credential-storage-enhancements-android-43.html
- http://www.arm.com/products/security-on-arm/trustzone
- https://developer.android.com/guide/topics/admin/device-admin.html
- https://developer.android.com/guide/topics/admin/device-admin.html#sample
- https://developer.android.com/work/device-management-policy.html
- https://developer.android.com/reference/android/content/pm/PackageManager.html#FEATURE_DEVICE_ADMIN
