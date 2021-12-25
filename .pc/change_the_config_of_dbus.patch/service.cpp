#include "service.h"
#include <QDebug>
#include <QtDBus>
#include <QDBusMessage>
#include <QString>
#include <QDir>
#include <QSettings>

Service::Service()
{
    QDBusConnection::systemBus().unregisterService("com.ukui.desktop.software");
    QDBusConnection::systemBus().registerService("com.ukui.desktop.software");
    QDBusConnection::systemBus().registerObject("/com/ukui/desktop/software", this,QDBusConnection :: ExportAllSlots | QDBusConnection :: ExportAllSignals);
}

QString Service::getExec(QString filename)
{
    QFile file(filename);
    QString str = "";
    if(file.open(QIODevice::ReadOnly))
    {
        while(!file.atEnd())
        {
            QByteArray line = file.readLine();
            str = line;
            if (str.startsWith("Exec=", Qt::CaseSensitive))
            {
                QString str1 = str.mid (5);
                str = str1.section(' ', 0, 0);
                QStringList list = str.split('/');
                str = list[list.size()-1];
                list = str.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
                str = list[0];
                break;
            }
        }
    }
    return str;
}

QString Service::get_usr_path(int id)
{
    QFile file2("/etc/passwd");
    if(file2.open(QIODevice::ReadOnly))
    {
        while(!file2.atEnd())
        {
            QByteArray line = file2.readLine();
            QString str(line);
            QStringList list = str.split(':');
            if(list[2] == QString::number(id))
            {
                QString str1 = list[0];
                QString usrpath = "/home/" + str1;
                file2.close();
                return usrpath;
            }
        }
    }
    file2.close();
}

qint32 Service::appVisiableControl(QString app_name, bool visiable, quint32 userid) //->(int32 err_num)
{
    qDebug() << "Message send!";
    QString homePath = get_usr_path(userid);
    QString filename1 = homePath + "/.cache/ukui-menu/ukui-menu.ini";
    qDebug() << filename1;
    m_qsettings = new QSettings(filename1, QSettings::IniFormat);

    if(app_name.isEmpty() || userid < 1000)
        return 1;
    QString path = "/usr/share/applications/";
    //设置要遍历的目录
    QDir dir(path);
    //设置文件过滤器
    QStringList nameFilters;
    //设置文件过滤格式
    nameFilters << "*.desktop";
    //将过滤后的文件名称存入到files列表中
    QStringList files = dir.entryList(nameFilters, QDir::Files|QDir::Readable, QDir::Name);
//    qDebug() << files;
    QString file;
    QString nameExec;
    QString filename;
    int findapp = 0;
    foreach(file, files)
    {
        filename = path + file;
        nameExec =  getExec(filename);
        if(nameExec == app_name)
        {
            qDebug() << app_name;
            findapp = 1;
            break;
        }
    }
    if(!findapp)
        return 2;
    if(!visiable)
    {
        qDebug() << "not visiable";
        m_qsettings->beginGroup("application");
        m_qsettings->setValue(app_name, false);
        m_qsettings->endGroup();
        m_qsettings->sync();
    }
    else{
        qDebug() << "visiable";
        m_qsettings->beginGroup("application");
        if (m_qsettings->contains(app_name))    // 判断"键"是否存在
        {
             m_qsettings->remove(app_name);      // 删除此键
             qDebug() << "remove";
        }
        else{
            qDebug() << "not found";
        }
        m_qsettings->endGroup();
        m_qsettings->sync();
    }

    QString str = "true";
    QDBusMessage message =QDBusMessage::createSignal("/com/ukui/desktop/software", "com.ukui.desktop.software", "send_to_client");
    message << str;
    QDBusConnection::sessionBus().send(message);
    return 0;
}
