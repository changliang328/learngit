1.添加远程库需要自己对添加的库命名，尤其是添加多个库，通过命名来区分；
2.clone自己远程库的地址为git@github.com:changliang328/test.git;  test是克隆后在本地存储的文件名;
3.git diff file_name  查看file_name文件的修改情况；
4.git reset --head HEAD^  回退到上一个版本，或者修改HEAD^为版本号（commit_id），可以回退到指定版本；
5.git push 仓库名 master 将本地内容推送到远程master分支
6.指纹远程库地址git@git.oschina.net:paul_liang/fingerprint.git   代码提交规范[姓名][项目/分享]：[提交内容]
7.梁波git库名paul_origin
8.git_remote -v 查看本地关联的库////  git remote remove origin   取消本地关联的远程库
9.git rm ***   删除一个文件     /// 继续删除版本库则执行git commit -m "****"
10.查看分支：git branch
创建分支：git branch <name>
切换分支：git checkout <name>
创建+切换分支：git checkout -b <name>
合并某分支到当前分支：git merge <name>
删除分支：git branch -d <name>
11.关联一个远程库，       使用命令git remote add origin git@server-name:path/repo-name.git；
        注意：×××××××repo-name必须是自己远程仓库存在的文件名，path是用户名，server-name服务器端的用户名
   eg:git remote add origin git@github.com:changliang328/changliang.git

   关联后，使用命令git push -u origin master第一次推送master分支的所有内容；
      git pull origin master   同步远程库到本地

此后，每次本地提交后，只要有必要，就可以使用命令git push origin master推送最新修改；
12.   场景1：当你改乱了工作区某个文件的内容，想直接丢弃工作区的修改时，用命令git checkout -- file。

场景2：当你不但改乱了工作区某个文件的内容，还添加到了暂存区时，想丢弃修改，分两步，第一步用命令git reset HEAD file，就回到了场景1，第二步按场景1操作。

场景3：已经提交了不合适的修改到版本库时，想要撤销本次提交，参考版本回退一节，不过前提是没有推送到远程库。

12.git pull 取回origin主机的next分支，与本地的master分支合并，需要写成下面这样。  git pull origin next:master
13.git branch
	git branch可以用来列出分支,创建分支和删除分支.
	git branch -v可以看见每一个分支的最后一次提交.
	git branch: 列出本地所有分支,当前分支会被星号标示出.
	git branch (branchname): 创建一个新的分支(当你用这种方式创建分支的时候,分支是基于你的上一次提交建立的). 
							 git branch -d (branchname)
14. git clone git@120.25.125.199:BTL.git   表示从服务器克隆到本地BTL,不需要git init初始化，和关联远程仓库
15. 生成ssh key指令
				   git config --global user.name "changliang"
	               git config --global user.email "changliang328@163.com"
				  ssh-keygen -t rsa -C "changliang328@163.com"
16. 基本操作：
	git status查看状态
	git add命令实际上就是把要提交的所有修改放到暂存区（Stage），
	git commit -m "xiugai"就可以一次性把暂存区的所有修改提交到分支。
	git push
17.使远程库覆盖本地库，本地与远程库保持同步更新
   git fetch --all  
   git reset --hard origin/master 
   git pull
18.git pull 时冲突解决办法：

	1.$ git stash	(暂存修改)先将本地修改存储起来,这样本地的所有修改就都被暂时存储起来 。是用git stash list可以看到保存的信息：
					其中stash@{0}就是刚才保存的标记。
	2、pull内容  	暂存了本地修改之后，就可以pull了。 	$ git pull
	3、还原暂存的内容  	$ git stash pop stash@{0}
		系统提示如下类似的信息：
		Auto-merging c/environ.c CONFLICT (content): Merge conflict in c/environ.c
		意思就是系统自动合并修改的内容，但是其中有冲突，需要解决其中的冲突。
	4、解决文件中冲突的的部分
		打开冲突的文件，手动修改
19.对远程仓库分支建立追踪关系：
	git branch --set-upstream-to=origin/（远程分支） （本地分支）
20.git rebase 修复冲突，
	如果从origin/master新建分支test，test和远程master都有多次提交时，push test分支之前先git rebase，rebase指令可以将提交暂时取消保存为补
	丁，然后更新为origin/master的最新状态，最后在将test的commit提交与最新状态合并。合并中可能有冲突，解决冲突然后git add 不需要git commit，
	然后执行git rebase --continue，test的commit提交就相当于在最新代码的基础上只做为一次commit合并















git log 查看git库的提交
