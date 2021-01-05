#!/bin/bash
workspace=$1 #源码根目录

cd $workspace

dpkg-buildpackage -b -d -uc -us

project_path=$(cd `dirname $0`; pwd)
#获取工程名
project_name="${project_path##*/}"
echo "project name is: $project_name"

#获取打包生成文件夹路径
pathname=$(find . -name obj*)

echo $pathname

cd $pathname/test

mkdir -p coverage

lcov --directory ../ --capture --output-file ./coverage/coverage.info #收集代码运行后所产生的统计计数信息

#以下几行是过滤一些我们不感兴趣的文件的覆盖率信息，各模块根据自模块实际情况填写过滤信息
lcov --remove ./coverage/coverage.info '*/${project_name}_test_autogen/*' '*/${project_name}_autogen/*' '*/usr/include/*' '*/dbuslogin1manager*' '*obj-x86_64-linux-gnu/*' '*/test/*' -o ./coverage/coverage.info

mkdir ../report
genhtml -o ../report ./coverage/coverage.info

lcov -d $build_dir –z

exit 0
