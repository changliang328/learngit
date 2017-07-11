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
11.要关联一个远程库，使用命令git remote add origin git@server-name:path/repo-name.git；

关联后，使用命令git push -u origin master第一次推送master分支的所有内容；

此后，每次本地提交后，只要有必要，就可以使用命令git push origin master推送最新修改；
12.   场景1：当你改乱了工作区某个文件的内容，想直接丢弃工作区的修改时，用命令git checkout -- file。

场景2：当你不但改乱了工作区某个文件的内容，还添加到了暂存区时，想丢弃修改，分两步，第一步用命令git reset HEAD file，就回到了场景1，第二步按场景1操作。

场景3：已经提交了不合适的修改到版本库时，想要撤销本次提交，参考版本回退一节，不过前提是没有推送到远程库。



123
