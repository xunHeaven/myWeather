#ifndef WEATHERCITY_H
#define WEATHERCITY_H
#include <QString>
#include <QMap>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QDebug>
class weatherCity{
private:
    static QMap<QString,QString> mCityMap;
    static void initCityMap()
    {
        //1.读取文件
        //QString filePath="E:\\mycode\\forQT\\src\\weather\\citycode-2019-08-23.json";
        QString filePath=":/res/citycode-2019-08-23.json";
        QFile file(filePath);
        file.open(QIODevice::ReadOnly|QIODevice::Text);
        QByteArray json=file.readAll();
        file.close();
        //2.解析，并写入到map
        QJsonParseError err;
        QJsonDocument doc=QJsonDocument::fromJson(json,&err);
        if(err.error!=QJsonParseError::NoError)
        {
            return;
        }
        if(!doc.isArray()) return;
        QJsonArray cities=doc.array();
        for(int i=0;i<cities.size();i++)
        {
            QString city=cities[i].toObject().value("city_name").toString();
            QString code=cities[i].toObject().value("city_code").toString();
            if(code.size()>0)
            {
                mCityMap.insert(city,code);
            }

        }

    }
public:
    static QString getCityCode(QString cityName)
    {
        if(mCityMap.size()==0)
        {
            initCityMap();
        }
        qDebug()<<"cityMap size:"<<mCityMap.size();

        QMap<QString,QString>::iterator it=mCityMap.find(cityName);
        if(it==mCityMap.end())
        {
            it=mCityMap.find(cityName+"市");
        }
        if(it==mCityMap.end())
        {
            it=mCityMap.find(cityName+"县");
        }
        if(it!=mCityMap.end())
        {
            return it.value();
        }
        return "";
    }
};
QMap<QString,QString> weatherCity::mCityMap={};

#endif // WEATHERCITY_H
