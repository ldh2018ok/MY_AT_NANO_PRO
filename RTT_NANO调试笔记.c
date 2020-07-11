一、问题1：
    出现了多次在MDK或STUDIO的硬仿真条件下运行，可以正常，但是下载到CPU就出现HARD FAULT!拍查内存情况后问题依旧。
	排查发现是由于APP初始化未完成时硬件发生中断调用事件发送函数导致硬件错误（此时事件还没有被创建并初始化）！
	解决办法：
	1）对所有变量在外设板级初始化的同时进行初始化赋值；
	2）将所有外设的中硬件中断设置改在APP线程创建完成后；
	通过以上更改后，程序下载正常运行，控制台正常操作！
二、问题2：
    在AT指令测试过程中，发现第一条指令接收数据异常，而此后正常！
	怀疑在上电初始，中断异常引起！
	解决办法：
	1）在所有APP应用线程初始化前关闭硬件中断，在设置完各外设中断后，再统一开硬件总中断。
	通过以上更改后运行正常！
	
三、问题3：
    测试AT指时发现，有部份指令偶尔出现超时，导致重发，打印输出信息有误！
	怀疑是在发送数据时被线程调度干扰所致。
	解决办法：
	1）在发送数据时，启用关闭调度操作进行保护，发送完成后重启调度。
	通过以上更改后测试未发现类似问题。

	GITHUB:   ldh200@163.com   ldh20148
	
				（一）GIT HUB 的使用笔记：上传本地项目到GITHUB https://blog.csdn.net/xqhys/article/details/98113227
1）首先你要在github上申请一个账号
	网址：https://github.com/
	然后你要下载一个git工具
	网址：https://gitforwindows.org/进入官网直接下载就行。
2）下载完成后进入github首页，注册帐号并登录。点击新项目，点击Create repository,生成自己的仓库。
3）点击Clone or download, 然后复制第二步的地址，待会上传项目有用.
4）接下来就是在本地操作了，在此之前你一定要把git安装好，然后找到你要上传的文件夹项目，右键点击文件夹（注意：不能选单个文件或者压缩包）在选项里会有
     Git Gui Here,Git Bash Here出现，选择Git Bash Here
	 然后把你之前在clone or download里复制的地址按下图粘贴，记住地址前面要加git clone.
	 然后你的项目文件里就会增加一个你在github上创建的项目名称
	 然后把你在这个目录要上传的文件复制到新增加文件夹里。然后在git命令行切换目录在新增的文件里。
5）然后输入命令" git add . "(注意" . "不能省略，此操作是把meituan文件夹下面新的文件或修改过的文件添加进来，如果有的文件之前已经添加了，它会自动省略)
6）然后输入git commit  -m  "提交信息" （提交的信息是你的项目说明）。	 
	 有些刚开使用的用户输入提交时会出现错误，这时你要先全局配置好在git上的用户名和邮箱，如下图所示输入命令：然后再输入提交的命令
7）最后输入命令git push -u origin master（此操作目的是把本地仓库push到github上面，此步骤需要你输入登录github上的帐号和密码）	 
8）然后进入里的github页面，刷新下页面就会显示你的项目。	 

				（二）GIT HUB 的使用笔记：上传本地项目到GITHUB https://blog.csdn.net/xqhys/article/details/98113227	
1）在电脑上建立一个文件夹，在这个文件夹进入“Git Bash Here”
2）	初始化：git init
3)	建立远程连接：git remote add origin git@github.com:用户名/仓库名.git 
    例如：git remote add origin git@github.com:815464927/Git.git
4)	pull 代码下来 
	git pull git@github.com:815464927/Git.git 
	执行完之后代码就可以pull到你电脑本地了。 

