1.目录结构
build.sh:编译脚本
demo.cpp：demo程序的UI显示
client：tcp client接口文件

2.编译说明
本程序依赖于opencv库，编译本程序前请安装opencv4(从v3更新到v4了，注意！)
./build.sh(gcc改clang了，注意！)

3.运行说明
  a.请先运行本程序，再在板子上运行算法程序，不然可能得不到前几帧的图像结果。
  b.运行命令(YUV改264，注意！)
./demo <cam num> <each 264 file name for the input cameras> <width> <height> <server ip> <enable jpg output:1 enable 0 disable>
如：
  4路输出的运行命令
./demo 4 test0_1920x1080.264 test1_1920x1080.264 test2_1920x1080.264 test3_1920x1080.264 1920 1080 192.168.20.199 1
  1路输出的运行命令
./demo 1 test0_1920x1080.264 1920 1080 192.168.20.199 1
c.命令行运行killall -9 demo，停止程序运行

4. (以下为新增内容，注意！)
  a 在Ubuntu16.04上测试通过。显示延迟正常，但比实际播放慢，需要在桌面版Ubuntu上直接播放，解决显示延迟问题。
  b demo.hpp中增加_ES_FILE_宏，定义后用于播放264文件（支持各种常见视频格式），关闭后仍旧仅处理YUV I420。
