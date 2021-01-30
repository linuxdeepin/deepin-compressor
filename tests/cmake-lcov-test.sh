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

cd $pathname/tests

mkdir -p coverage

#收集代码运行后所产生的统计计数信息
lcov -d ../ -c -o ./coverage/coverage.info 

#只收集src、3rdparty部分目录
lcov --extract ./coverage/coverage.info '*/src/*' '*/3rdparty/*' -o ./coverage/coverage.info
#过滤 tests 3rdparty/ChardetDetector目录
lcov --remove ./coverage/coverage.info '*/tests/*' '*/ChardetDetector/*' -o ./coverage/coverage.info

mkdir ../report
genhtml -o ../report ./coverage/coverage.info

exit 0
