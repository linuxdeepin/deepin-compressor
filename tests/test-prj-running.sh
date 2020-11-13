#!/bin/bash


workdir=../../"build-compressor(dev)-Desktop-Debug"/tests #编译路径
# workdir=../"build-tests-Desktop-Debug" #编译路径

# executable=deepin-compressor #可执行程序的文件名
executable1=test_output/cli7zplugin_test #可执行程序的文件名
executable2=test_output/clirarplugin_test #可执行程序的文件名
executable3=test_output/cliunarchiverplugin_test #可执行程序的文件名
executable4=test_output/lbarchive_test #可执行程序的文件名
executable5=test_output/lreadwritelibarchiveplugin_test #可执行程序的文件名
executable6=test_output/readonlylibarchiveplugin_test #可执行程序的文件名
executable7=test_output/libzipplugin_test #可执行程序的文件名
executable8=test_output/common_test #可执行程序的文件名
executable9=test_output/interface_test #可执行程序的文件名
executable10=test_output/deepin-compressor_test #可执行程序的文件名

build_dir=$workdir
# cd $build_dir
result_coverage_dir=$build_dir/coverage
result_report_dir=$build_dir/report/report.xml

#下面是覆盖率目录操作，一种正向操作，一种逆向操作
# extract_info="../deepin-compressor/source/src/*"  #针对当前目录进行覆盖率操作

# remove_info=/usr/include/* #排除当前目录进行覆盖率操作

# # ./$build_dir/$executable --gtest_output=xml:$result_report_dir
$executable1 --gtest_output=xml:$result_report_dir
$executable2 --gtest_output=xml:$result_report_dir
$executable3 --gtest_output=xml:$result_report_dir
$executable4 --gtest_output=xml:$result_report_dir
$executable5 --gtest_output=xml:$result_report_dir
$executable6 --gtest_output=xml:$result_report_dir
$executable7 --gtest_output=xml:$result_report_dir
$executable8 --gtest_output=xml:$result_report_dir
$executable9 --gtest_output=xml:$result_report_dir
$executable10 --gtest_output=xml:$result_report_dir

# # lcov -d `pwd` -c -o coverage.info

# # lcov --remove coverage.info  '*/deepin-log-viewer-test_autogen/*' '*/deepin-log-viewer_autogen/*' '*/usr/include/*' '*/dbuslogin1manager*' '*/tests/*' '*/googletest/*' '*/3rdparty/*' '*/test/*' 'accessible.h' 'accessibledefine.h' --output-file coverage.info

# # lcov --extract coverage.info $extract_info --output-file  coverage.info

# # lcov --list-full-path -e coverage.info –o coverage-stripped.info

# lcov -d $build_dir -c -o $build_dir/coverage.info

# #    #3.过滤一些我们不感兴趣的文件的覆盖率信息
# # lcov --remove $build_dir/coverage.info '*/deepin-log-viewer-test_autogen/*' '*/deepin-log-viewer_autogen/*' '*/usr/include/*' '*/dbuslogin1manager*' '*/tests/*' '*/googletest/*' '*/3rdparty/*' '*/test/*' '*/accessible.h' '*/accessibledefine.h' -o coverage.info
# lcov --extract $build_dir/coverage.info $extract_info --output-file  $build_dir/coverage.info

# lcov常用的参数
# -d 项目路径，即.gcda .gcno所在的路径
# -a 合并（归并）多个lcov生成的info文件
# -c 捕获，也即收集代码运行后所产生的统计计数信息
# --external 捕获其它目录产生的统计计数文件
# -i/--initial 初始化所有的覆盖率信息，作为基准数据
# -o 生成处理后的文件
# -r/--remove 移除不需要关注的覆盖率信息文件
# -z 重置所有执行程序所产生的统计信息为0

# lcov -d $build_dir -c -i -o $build_dir/init.info
# lcov -d $build_dir -c -o $build_dir/total.info
# # -a 合并文件
# lcov -a $build_dir/init.info -a $build_dir/total.info -o $build_dir/coverage.info
lcov -d $build_dir -c -o $build_dir/coverage.info
lcov --remove $build_dir/coverage.info "/usr/include/*" "*/googletest/*" "*/tests/UnitTest/*" -o $build_dir/coverage.info
# lcov --extract $build_dir/coverage.info "../deepin-compressor/source/src/*" --output-file  $build_dir/coverage.info
lcov --list-full-path -e $build_dir/coverage.info –o $build_dir/coverage-stripped.info

#如果是git目录，可以获取此次版本的commitID，如果不是，忽略此步
# commitId=$(git log | head -n1 | awk '{print $2}')
# 这里可以带上项目名称和提交ID，如果没有，忽略此步
#genhtml -o cover_report --legend --title "${project_name} commit SHA1:${commitId}" --prefix=${curr_path} final.info
# -o 生成的html及相关文件的目录名称，--legend 简单的统计信息说明
# --title 项目名称，--prefix 将要生成的html文件的路径 
# genhtml -o cover_report --legend --title "lcov"  --prefix=./ final.info

genhtml -o $result_coverage_dir --legend $build_dir/coverage.info

nohup xdg-open $result_coverage_dir/index.html &

nohup xdg-open $result_report_dir &

lcov -d $build_dir –z

exit 0
