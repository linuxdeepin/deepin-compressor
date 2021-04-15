utdir=build-ut
rm -r $utdir
rm -r ../$utdir
mkdir ../$utdir
cd ../$utdir

cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j16

./bin/bz2plugin_test
./bin/cli7zplugin_test
./bin/clirarplugin_test
./bin/deepin-compressor_test
./bin/gzplugin_test
./bin/interface_test
./bin/libarchive_test
./bin/libminizipplugin_test
./bin/libzipplugin_test
./bin/readonlylibarchiveplugin_test
./bin/readwritelibarchiveplugin_test
./bin/singlefile_test
./bin/xzplugin_test

workdir=$(cd ../$(dirname $0)/$utdir; pwd)

mkdir -p report
lcov -d $workdir -c -o ./report/coverage.info

#只收集src、3rdparty部分目录
lcov --extract ./report/coverage.info '*/src/*' '*/3rdparty/*' -o ./report/coverage.info
#过滤 tests 3rdparty/ChardetDetector目录
lcov --remove ./report/coverage.info '*/tests/*' '*/ChardetDetector/*' -o ./report/coverage.info

genhtml -o ./report ./report/coverage.info

exit 0
