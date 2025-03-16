#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "weatherdata.h"
#include <QMainWindow>
#include <QContextMenuEvent>
#include <QNetworkAccessManager>
#include<QNetworkReply>
#include <QList>
#include <QLabel>
#include <QMap>
//#include "weatherCity.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    //获取天气数据
    void getweatherInfo(QString cityCode);
    //解析json
    void parseJson(QByteArray& byteArray);
    //更新UI
    void updateUI();
    //重写父类的eventfilter方法
    bool eventFilter(QObject* watch,QEvent* event);
    //绘制高低温曲线
    void paintHighCurve();
    //void paintLowCurve();

private slots:
    void onReplied(QNetworkReply* reply);
    void on_btn_Search_clicked();

private:
    QMenu* mExitMenu;//右键退出的菜单
    QAction* mExitAct;//退出的行为-菜单项

    QPoint mOffset;//窗口移动时，鼠标与窗口左上角的偏移
    //HTTP请求
    QNetworkAccessManager* mNetAccessManager;
    //今天和过去未来六天的天气
    Today mToday;
    Day mDay[6];
    //星期和日期的列表
    QList<QLabel*> mWeekList;
    QList<QLabel*> mDateList;
    //天气和天气图标的列表
    QList<QLabel*> mTypeList;
    QList<QLabel*> mTypeIcoList;
    //风力和风向的列表
    QList<QLabel*> mFlList;
    QList<QLabel*> mFxList;
    //天气污染指数
    QList<QLabel*> mAqiList;

    //天气图片的map
    QMap<QString,QString> mTypeMap;

};
#endif // MAINWINDOW_H
