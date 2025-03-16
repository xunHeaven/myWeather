#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "weatherCity.h"
#include <QPainter>

#define INCREMENT 2 //温度每升高或者降低1°，y轴的增量
#define POINT_RADIUS 3 //曲线描点的大小
#define TEXT_OFFSETX 12 //曲线图中，温度文字相对对应x坐标的偏移量
#define TEXT_OFFSETY 12

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //1.设置窗口属性
    //设置窗口无边框
    setWindowFlag(Qt::FramelessWindowHint);
    //设置固定窗口大小
    setFixedSize(width(),height());

    //2.鼠标右键菜单：退出程序
    //构建右键菜单
    mExitMenu=new QMenu(this);
    mExitAct=new QAction();
    mExitAct->setText("退出");
    mExitAct->setIcon(QIcon(":/res/close.ico"));
    //将菜单项添加到菜单
    mExitMenu->addAction(mExitAct);
    //信号槽绑定
    connect(mExitAct,QAction::triggered,this,[=]{
        qApp->exit(0);
    });

    //将控件添加到控件数组
    mWeekList<<ui->label_week0<<ui->label_week1<<ui->label_week1<<ui->label_week3<<ui->label_week4<<ui->label_week5;
    mDateList<<ui->label_date0<<ui->label_date1<<ui->label_date2<<ui->label_date3<<ui->label_date4<<ui->label_date5;
    mTypeList<<ui->label_type0<<ui->label_type1<<ui->label_type2<<ui->label_type3<<ui->label_type4<<ui->label_type5;
    mTypeIcoList<<ui->label_typeIco0<<ui->label_typeIco1<<ui->label_typeIco2<<ui->label_typeIco3<<ui->label_typeIco4<<ui->label_typeIco5;
    mFlList<<ui->label_fx21<<ui->label_fx22<<ui->label_fx23<<ui->label_fx24<<ui->label_fx25<<ui->label_fx26;
    mFxList<<ui->label_fx11<<ui->label_fx12<<ui->label_fx13<<ui->label_fx14<<ui->label_fx15<<ui->label_fx16;
    mAqiList<<ui->label_quality0<<ui->label_quality1<<ui->label_quality2<<ui->label_quality3<<ui->label_quality4<<ui->label_quality5;

    //将天气类型作为key，图片资源文件路径作为value
    mTypeMap.insert("暴雪",":/res/baoxue.png");
    mTypeMap.insert("暴雨",":/res/baoyu.png");
    mTypeMap.insert("暴雨到大暴雨",":/res/baoyu.png");
    mTypeMap.insert("大暴雨",":/res/baoyu.png");
    mTypeMap.insert("大暴雨到特大暴雨",":/res/baoyu.png");
    mTypeMap.insert("大到暴雪",":/res/baoxue.png");
    mTypeMap.insert("大雪",":/res/daxue.png");
    mTypeMap.insert("大雨",":/res/dayu.png");
    mTypeMap.insert("冻雨",":/res/dongyu(2).png");
    mTypeMap.insert("多云",":/res/duoyun.png");
    mTypeMap.insert("浮尘",":/res/yangsha.png");
    mTypeMap.insert("雷阵雨",":/res/leizhenyu.png");
    mTypeMap.insert("雷阵雨伴有冰雹",":/res/leizhengyubanbingbao.png");
    mTypeMap.insert("霾",":/res/wumai.png");
    mTypeMap.insert("强沙尘暴",":/res/shachenbao.png");
    mTypeMap.insert("晴",":/res/qing.png");
    mTypeMap.insert("沙尘暴",":/res/shachenbao.png");
    mTypeMap.insert("特大暴雨",":/res/shachenbao.png");
    mTypeMap.insert("雾",":/res/wu.png");
    mTypeMap.insert("小到中雪",":/res/zhongxue.png");
    mTypeMap.insert("小到中雨",":/res/zhongyu.png");
    mTypeMap.insert("小雪",":/res/xiaoxue.png");
    mTypeMap.insert("小雨",":/res/xiaoyu.png");
    mTypeMap.insert("雪",":/res/xiaoxue.png");
    mTypeMap.insert("扬沙",":/res/yangsha.png");
    mTypeMap.insert("阴",":/res/yin.png");
    mTypeMap.insert("雨",":/res/xiaoyu.png");
    mTypeMap.insert("雨夹雪",":/res/yujiaxue.png");
    mTypeMap.insert("阵雪",":/res/xiaoxue.png");
    mTypeMap.insert("阵雨",":/res/zhenyu.png");
    mTypeMap.insert("中到大雪",":/res/daxue.png");
    mTypeMap.insert("中到大雨",":/res/dayu.png");
    mTypeMap.insert("中雪",":/res/zhongxue.png");
    mTypeMap.insert("中雨",":/res/zhongyu.png");



    //3.网络请求
    mNetAccessManager=new QNetworkAccessManager(this);
    connect(mNetAccessManager,&QNetworkAccessManager::finished,this,&MainWindow::onReplied);
    connect(ui->lineEdit_city,&QLineEdit::returnPressed,this,&on_btn_Search_clicked);
    //直接在构造中请求天气数据
    //“101010100”是北京的城市编码
    //getweatherInfo("101010100");
    //4.直接在构造中请求天气信息
    getweatherInfo("昆明");

    //5.给标签添加事件过滤器
    ui->label_highCurv->installEventFilter(this);
    //ui->label_lowCurv->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
//重写父类的虚函数
//父类中默认的实现是忽略右键菜单事件
//重写之后可以处理右键菜单
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    //弹出右键菜单
    mExitMenu->exec(QCursor::pos());
    //表示这个事件已经被处理，不需要进一步传播。
    event->accept();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    //按下鼠标左键才记录
    if(event->button()==Qt::LeftButton)
    {
        //鼠标在整个屏幕的坐标，减去窗口的左上角坐标，得到鼠标在窗口的坐标
        mOffset=event->globalPos()-this->pos();
    }

}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons()&Qt::LeftButton)
    {
        this->move(event->globalPos()-mOffset);
    }
}

void MainWindow::getweatherInfo(QString cityName)
{
    QString cityCode=weatherCity::getCityCode(cityName);
    if(cityCode.isEmpty())
    {
        QMessageBox::warning(this,"天气","请检查输入是否正确","ok");
        return;
    }
    QUrl url("http://t.weather.itboy.net/api/weather/city/"+cityCode);
    mNetAccessManager->get(QNetworkRequest(url));
}

void MainWindow::parseJson(QByteArray &byteArray)
{
    QJsonParseError err;
    QJsonDocument doc=QJsonDocument::fromJson(byteArray,&err);
    if(err.error!=QJsonParseError::NoError)
    {
        //如果发生错误，直接返回
        return;
    }
    QJsonObject rootObj=doc.object();
    qDebug()<<rootObj.value("message").toString();

    //1.解析日期和城市
    mToday.date=rootObj.value("date").toString();
    mToday.city=rootObj.value("cityInfo").toObject().value("city").toString();

    //2.解析yesterday
    QJsonObject objData=rootObj.value("data").toObject();

    QJsonObject objYesterday=objData.value("yesterday").toObject();
    mDay[0].week=objYesterday.value("week").toString();
    mDay[0].date=objYesterday.value("ymd").toString();
    mDay[0].type=objYesterday.value("type").toString();

    //高温低温处理
    QString s;
    s=objYesterday.value("high").toString().split(" ").at(1);
    s=s.left(s.length()-1);
    mDay[0].high=s.toInt();

    s=objYesterday.value("low").toString().split(" ").at(1);
    //qDebug()<<"s0 is:"<<s;
    //“℃”两个字节一个字符，而s.length()返回的是字符数
    s=s.left(s.length()-1);
    //qDebug()<<"s1 is:"<<s;
    mDay[0].low=s.toInt();
    //qDebug()<<"low1 is:"<<mDay[0].low;
    //风向风力
    mDay[0].fx=objYesterday.value("fx").toString();
    mDay[0].fl=objYesterday.value("fl").toString();

    //污染指数,直接转换为数字，用toDouble()
    mDay[0].aqi=objYesterday.value("aqi").toDouble();

    //3.解析forcast 5天的数据
    QJsonArray forecastArr=objData.value("forecast").toArray();
    for(int i=0;i<5;i++)
    {
        QJsonObject objForecast=forecastArr[i].toObject();
        mDay[i+1].week=objForecast.value("week").toString();
        mDay[i+1].date=objForecast.value("ymd").toString();
        mDay[i+1].type=objForecast.value("type").toString();

        //高温低温处理
        s=objForecast.value("low").toString().split(" ").at(1);
        s=s.left(s.length()-1);
        mDay[i+1].low=s.toInt();

        s=objForecast.value("high").toString().split(" ").at(1);
        s=s.left(s.length()-1);
        mDay[i+1].high=s.toInt();

        //风向风力
        mDay[i+1].fx=objForecast.value("fx").toString();
        mDay[i+1].fl=objForecast.value("fl").toString();

        //污染指数,直接转换为数字，用toDouble()
        mDay[i+1].aqi=objForecast.value("aqi").toDouble();

    }
    //4.解析今天的数据

    mToday.ganmao=objData.value("ganmao").toString();
    QString wd=objData.value("wendu").toString();
    //qDebug()<<"wendu="<<wd;
    mToday.wendu=wd.toDouble();
    qDebug()<<"wendu="<<mToday.wendu;
    mToday.shidu=objData.value("shidu").toString();
    mToday.pm25=objData.value("pm25").toDouble();
    mToday.quality=objData.value("quality").toString();
    //5.forecast中第一个元素也是今天的数据
    mToday.type=mDay[1].type;
    mToday.fl=mDay[1].fl;
    mToday.fx=mDay[1].fx;
    mToday.high=mDay[1].high;
    mToday.low=mDay[1].low;
    //qDebug()<<"Ylow is:"<<mDay[0].low;

    //6.更新UI
    //6.1显示文本和图标
    updateUI();
    //6.2绘制温度曲线，如果不写，则温度曲线一直不变
    ui->label_highCurv->update();

}

void MainWindow::updateUI()
{
    //1.更新日期和城市
    ui->label_date6->setText(QDateTime::fromString(mToday.date,"yyyyMMdd").toString("yyyy/MM/dd"));
    //ui->label_date6->setText(mToday.date);
    //qDebug()<<"date is:"<<mToday.date;
    ui->label_week6->setText(mDay[1].week);
    ui->label_city->setText(mToday.city);
    ui->label_city->setWordWrap(true);
    //2.更新今天
    ui->label_temp->setText(QString::number(mToday.wendu));
    ui->label_type6->setText(mToday.type);
    if(mTypeMap.contains(mToday.type))
    {
        ui->label_typeIco6->setPixmap(mTypeMap[mToday.type]);
    }
    else
    {
        ui->label_typeIco6->setPixmap(QPixmap(":/res/yin.png"));
    }
    ui->label_tempScale->setText(QString::number(mToday.low)+"~"+QString::number(mToday.high)+"°C");
    qDebug()<<"low is:"<<mToday.low;
    ui->label_GM->setText("感冒指数: "+mToday.ganmao);
    ui->label_FX1->setText(mToday.fx);
    ui->label_FX2->setText(mToday.fl);
    ui->label_PM2->setText(QString::number(mToday.pm25));
    ui->label_SD2->setText(mToday.shidu);
    ui->label_KQ2->setText(mToday.quality);
    //3.更新六天

    for(int i=0;i<6;i++)
    {
        //更新日期和时间
        mWeekList[i]->setText("周"+mDay[i].week.right(1));
        ui->label_week0->setText("昨天");
        ui->label_week1->setText("今天");
        ui->label_week2->setText("明天");

        QStringList ymdList=mDay[i].date.split("-");
        mDateList[i]->setText(ymdList[1]+"/"+ymdList[2]);

        //更新天气类型
        mTypeList[i]->setText(mDay[i].type);
        if(mTypeMap.contains(mDay[i].type))
        {
            mTypeIcoList[i]->setPixmap(mTypeMap[mDay[i].type]);
        }
        else
        {
            mTypeIcoList[i]->setPixmap(QPixmap(":/res/yin.png"));
        }

        //更新空气质量
        if(mDay[i].aqi>=0&&mDay[i].aqi<=50){
            mAqiList[i]->setText("优");
            mAqiList[i]->setStyleSheet("background-color:rgb(121,184,0)");

        }
        else if(mDay[i].aqi>50&&mDay[i].aqi<=100){
            mAqiList[i]->setText("良");
            mAqiList[i]->setStyleSheet("background-color:rgb(255,187,23)");

        }
        else if(mDay[i].aqi>100&&mDay[i].aqi<=150){
            mAqiList[i]->setText("轻度");
            mAqiList[i]->setStyleSheet("background-color:rgb(255,87,97)");
        }
        else if(mDay[i].aqi>150&&mDay[i].aqi<=200){
            mAqiList[i]->setText("中度");
            mAqiList[i]->setStyleSheet("background-color:rgb(235,17,27)");

        }
        else if(mDay[i].aqi>200&&mDay[i].aqi<=250){
            mAqiList[i]->setText("重度");
            mAqiList[i]->setStyleSheet("background-color:rgb(170,0,0)");

        }
        else
        {
            mAqiList[i]->setText("严重");
            mAqiList[i]->setStyleSheet("background-color:rgb(110,0,0)");
        }

        //更新风力和风向
        mFxList[i]->setText(mDay[i].fx);
        mFlList[i]->setText(mDay[i].fl);
    }

}

bool MainWindow::eventFilter(QObject *watch, QEvent *event)
{
    //如果监控到控件label_highCurv，并且该空控件发送了Paint事件，则调用相应方法
    if(watch==ui->label_highCurv&&event->type()==QEvent::Paint)
    {
        paintHighCurve();
    }
    /*if(watch==ui->label_lowCurv&&event->type()==QEvent::Paint)
    {
        paintLowCurve();
    }*/
    return QWidget::eventFilter(watch,event);
}

void MainWindow::paintHighCurve()
{
    //创建画家，并指定在哪里作画
    QPainter painter(ui->label_highCurv);
    //平滑设置(抗锯齿)
    painter.setRenderHint(QPainter::Antialiasing,true);
    //1.获取x坐标
    int pointX[6]={0};
    for(int i=0;i<6;i++)
    {
        //获取中心点的坐标
        pointX[i]=mDateList[i]->pos().x()+mDateList[i]->width()/2;
    }
    //2.获取y坐标
    int tempHSum=0;
    int tempHAve=0;
    int tempLSum=0;
    int tempLAve=0;
    int tempAve=0;
    for(int i=0;i<6;i++)
    {
        tempHSum+=mDay[i].high;
        tempLSum+=mDay[i].low;
    }
    tempHAve=tempHSum/6;//计算最高温的平均值
    tempLAve=tempLSum/6;//计算最低温的平均值
    tempAve=(tempHAve+tempLAve)/2;
    int yCenter=ui->label_highCurv->height()/2;
    //计算y轴坐标
    int pointYh[6]={0};
    int pointYl[6]={0};
    for(int i=0;i<6;i++)
    {
        pointYh[i]=yCenter-((mDay[i].high-tempAve)*INCREMENT);
        pointYl[i]=yCenter-((mDay[i].low-tempAve)*INCREMENT);
        qDebug()<<"plw is:"<<pointYl[i];
        qDebug()<<"phw is:"<<pointYh[i];
    }

    //3.开始绘制高温曲线
    //3.1初始化画笔
    QPen pen=painter.pen();
    pen.setWidth(1);//画笔宽度
    pen.setColor(QColor(255,170,0));//画笔颜色

    painter.setPen(pen);
    painter.setBrush(QColor(255,0,0));//画刷，填充圆点的内部
    //3.2画点，写文本
    for(int i=0;i<6;i++)
    {
        //画点
        painter.drawEllipse(pointX[i],pointYh[i],POINT_RADIUS,POINT_RADIUS);
        //显示温度
        painter.drawText(pointX[i]-TEXT_OFFSETX,pointYh[i]-TEXT_OFFSETY,QString::number(mDay[i].high)+"°");
        //qDebug()<<"Hw is:"<<mDay[i].high;
    }
    //连线
    for(int i=0;i<5;i++)
    {
        if(i==0)//昨天到今天为虚线
        {
            pen.setStyle(Qt::DotLine);
            painter.setPen(pen);
        }
        else
        {
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
        }
        painter.drawLine(pointX[i],pointYh[i],pointX[i+1],pointYh[i+1]);
    }

    //4.开始绘制低温曲线
    pen.setWidth(1);//画笔宽度
    pen.setColor(QColor(0,255,255));//画笔颜色

    painter.setPen(pen);
    painter.setBrush(QColor(0,255,255));//画刷，填充圆点的内部

    //3.2画点，写文本
    for(int i=0;i<6;i++)
    {
        //画点
        painter.drawEllipse(pointX[i],pointYl[i],POINT_RADIUS,POINT_RADIUS);
        //显示温度
        painter.drawText(pointX[i]-TEXT_OFFSETX,pointYl[i]+TEXT_OFFSETY+2,QString::number(mDay[i].low)+"°");
        //qDebug()<<"lw is:"<<mDay[i].low;
    }
    //连线
    for(int i=0;i<5;i++)
    {
        if(i==0)//昨天到今天为虚线
        {
            pen.setStyle(Qt::DotLine);
            painter.setPen(pen);
        }
        else
        {
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
        }
        painter.drawLine(pointX[i],pointYl[i],pointX[i+1],pointYl[i+1]);
    }


}



void MainWindow::onReplied(QNetworkReply *reply)
{
    qDebug()<<"onReplied success!";
    //如果请求成功，则获得的数据都被封装在reply中
    int status_code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug()<<"Operation:"<<reply->operation();
    qDebug()<<"Status code:"<<status_code;
    qDebug()<<"url:"<<reply->url();
    qDebug()<<"raw header:"<<reply->rawHeaderList();
    if(reply->error()!=QNetworkReply::NoError||status_code!=200)
    {
        qDebug()<<reply->errorString().toLatin1().data();
        QMessageBox::warning(this,"天气","请求数据失败",QMessageBox::Ok);
    }
    else
    {
        QByteArray byteArray=reply->readAll();
        qDebug()<<"data:"<<byteArray.data();
        parseJson(byteArray);
    }
    reply->deleteLater();

}

void MainWindow::on_btn_Search_clicked()
{
    QString cityName=ui->lineEdit_city->text();
    getweatherInfo(cityName);
}

