# play with CTP - Release


### INFO

原始 git repo 不小心上传了账号密码等信息，所以无法直接 make it public。新建了一个项目后续代码会上传这里, Issue也会陆续transfer到这里。


### BUILD

include文件夹下是CTP-API接口文件，从上期技术官网下载得到。另外将两个`.so`文件放到 /usr/lib/ 路径下 并改名为 libxxx.so

将 `congfig_template.h` 重命名为 `config.h`, 并填入自己的SimNow账号密码。

主程序
```
$ g++ main.cpp -o main -Wall -std=c++11  -lthostmduserapi_se -lthosttraderapi_se  
$ ./main
```

命令行下单工具
```
$ g++ console_trader.cpp -o console_trader -Wall -std=c++11  -lthostmduserapi_se -lthosttraderapi_se  
$ ./console_trader
```

收行情
```
# 将 Md.hpp 结尾的 `int main() {...}` 注释删除，直接编译即可得到一个收行情的程序

$ g++ Md.cpp -o market_data -Wall -std=c++11  -lthostmduserapi_se -lthosttraderapi_se  
$ ./market_data
```
