# myWeather
用QT开发的天气预报应用软件
文件说明：
1.文件夹"res"存储各种天气图标和城市code的json文件。
2."安装包和结果展示“文件夹中存放着打包好的天气预报软件，可以直接安装使用。
3."mainwindow.h"和"mainwindow.cpp"是主要类和函数功能的实现的文件。
4.“weatherdata.h”定义了两个类，“Today”表示今天的天气信息，"Day"记录的是过去未来的天气信息。
5."weatherCity.h"定义了通过城市名获取城市code的方法。定义了QMap<QString,QString> mCityMap，其中键为城市名，值为城市code。


