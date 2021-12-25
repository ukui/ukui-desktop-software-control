#include "service.h"
#include <QDebug>
#include <QtDBus>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QString>
#include <QDir>
#include <QSettings>
#include <gio/gio.h>
#include <QProcess>
#include <QByteArray>

Service::Service()
{
    interface = new UkuiMenuInterface;
    interface2 = new UkuiMenuInterface;

//    qDebug() << interface->filePathList;
    //QDBusConnection代表到D-Bus总线的一个连接，是一个D-Bus会话的起始点.
    //通过QDBusConnection连接对象，可以访问远程对象、接口，连接远程信号到本地槽函数，注册对象等。
    //作为两种最常用总线类型的辅助，sessionBus()和systemBus()函数分别创建到会话在总线和系统总线的连接并返回，会在初次使用时打开，在QCoreApplication析构函数调用时断开。

    //系统总线,注销服务
    QDBusConnection::systemBus().unregisterService("com.ukui.desktop.software");

    //注册服务,获取"服务名称"
    QDBusConnection::systemBus().registerService("com.ukui.desktop.software");

    //注册对象
    QDBusConnection::systemBus().registerObject("/com/ukui/desktop/software", this, QDBusConnection :: ExportAllSlots | QDBusConnection :: ExportAllSignals);

//    QDBusConnection::sessionBus().unregisterService("com.ukui.desktop.software");
//    QDBusConnection::sessionBus().registerService("com.ukui.desktop.software");
//    QDBusConnection::sessionBus().registerObject("/com/ukui/desktop/software", this, QDBusConnection :: ExportAllSlots | QDBusConnection :: ExportAllSignals);
}

//获取应用程序"Exce="名称
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

            //判断字符串str是否以Exce=开头的
            if (str.startsWith("Exec=", Qt::CaseSensitive))
            {
//                切割长度为5
                QString str1 = str.mid (5);
                //切割位置为从0开始到0结束
                str = str1.section(' ', 0, 0);
                QStringList list = str.split('/');
                str = list[list.size()-1];
                //获取表头，正则表达式筛选\r\n的形式，忽略空格。
                list = str.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
                str = list[0];
                break;
            }
        }
    }
    return str;
}

//获取用户路径
QString Service::get_usr_path(int id)
{
    if(id == 0){
        QString usrpath = "/root";
        return usrpath;
    }
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
                username = str1;
                QString usrpath = "/home/" + str1;
                file2.close();
                return usrpath;
            }
        }
    }
    file2.close();
}

//清除唤醒服务
void Service::cleanwake()
{
    QProcess p(this);
    p.start(QString("rtcwake -m disable"));
    p.waitForFinished();
}

//提供唤醒服务
void Service::rtcexec(int wake)
{
    QTime time = QTime::currentTime();
    int timeH = time.hour();
    int timeM = time.minute();
    int timeS = time.second();
    int rctwaketime =wake-timeH*3600-timeM*60-timeS-3;
    QString s =QString::number(rctwaketime);
    QProcess p(this);
    p.start(QString("rtcwake -s %1").arg(s));
    p.waitForFinished();
}

//腾讯应用路径
QStringList Service::alltxban(quint32 userid)
{
    QString homePath = get_usr_path(userid);
    QString filename = homePath + "/.cache/ukui-menu/ukui-tencent-menu.ini";
    m_qsettings3 = new QSettings(filename, QSettings::IniFormat);
    //给当前的group里由QSettings指定的所有键(key)，自动附上当前的group前缀。
    m_qsettings3->beginGroup("application");
    QStringList result = m_qsettings3->allKeys();
//    调用beginGroup()后，需要调用endGroup(), 复位当前的组(group)。
//    即：beginGroup()和endGroup()成套使用，否则会形成group网络。
    m_qsettings3->endGroup();
//    调用sync可以将系统缓冲区（内存中）的数据写入到文件系统（磁盘）中。
    m_qsettings3->sync();
    return result;
}

QStringList Service::txban(quint32 userid)
{
    QString homePath = get_usr_path(userid);
    QString filename = homePath + ".cache/ukui-menu/ukui-tencent-menu.ini";
    m_qsettings3 = new QSettings(filename, QSettings::IniFormat);

    m_qsettings3->beginGroup("application");
    QStringList result = m_qsettings3->allKeys();

    m_qsettings3->endGroup();
    m_qsettings3->sync();
    return result;
}

/**
 * @brief Service::appVisiableControl
 * @param appid     获取App的Id标识
 * @param visiable  获取禁用选项标识
 * @param userid    获取用户的Id标识
 * @return
 */
qint32 Service::appVisiableControl(QString appid, bool visiable, quint32 userid) //->(int32 err_num)
{
//    qDebug() << "Message send!";
    QString homePath = get_usr_path(userid);
    QString filename1 = homePath + "/.cache/ukui-menu/ukui-menu.ini";
    QString filename2 = homePath + "/.cache/ukui-menu/ukui-tencent-menu.ini";
//    qDebug() << filename1;
    m_qsettings = new QSettings(filename1, QSettings::IniFormat);


    if(appid.isEmpty() || userid < 1000)
        return 1;
    QString path = "/usr/share/applications/";
    //设置要遍历的目录
    QDir dir(path);
//    //设置文件过滤器
//    QStringList nameFilters;
//    //设置文件过滤格式
//    nameFilters << "*.desktop";
//    //将过滤后的文件名称存入到files列表中
//    QStringList files = dir.entryList(nameFilters, QDir::Files|QDir::Readable, QDir::Name);
//    qDebug() << files;
    QString nameExec;
    QString filename = path + appid + ".desktop";
//    qDebug() << filename;

    //桌面desktop文件刷新
    interface->getDesktopFilePath();

//    输入的大小写要敏感
    if(interface->filePathList.contains(filename, Qt::CaseInsensitive))
    {
        nameExec = interface->getAppExec(filename);

//        qDebug() << nameExec;

        // 从/usr/bin/xxx --replace中提取出xxx
//        QString str = nameExec.section(' ', 0, 0);
//        QStringList list = str.split('/');
//        str = list[list.size()-1];
//        list = str.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
//        nameExec = list[0];
//        qDebug() << nameExec;

        if(!visiable)
        {
//            qDebug() << "not visiable";
            m_qsettings->beginGroup("application");
            m_qsettings->setValue(nameExec, false);
            m_qsettings->endGroup();
            m_qsettings->sync();
            if (!nameExec.contains("TXEduPlatformRuntime")){
                QProcess p1(this);
                qDebug()<<nameExec;
                if (nameExec.contains("/usr")) {
                    nameExec = getExec(filename);
                }
                p1.start(QString("setfacl -m u:%1:r-- /usr/bin/%2").arg(username).arg(nameExec));
                p1.waitForFinished();
//                QProcess p2(this);
//                p2.start(QString("killall %1").arg(nameExec));
//                p2.waitForFinished();
            } else {
                m_qsettings2 = new QSettings(filename2, QSettings::IniFormat);
                QString id = interface->getAppId(filename);
                m_qsettings2->beginGroup("application");
                m_qsettings2->setValue(id, false);
                m_qsettings2->endGroup();
                m_qsettings2->sync();
                QDBusMessage message =QDBusMessage::createSignal("/com/ukui/desktop/software", "com.ukui.desktop.software", "send_id");
                message << id << visiable;
                qDebug() << id;
//                返回一个打开到system总线的QDBusConnection对象
                QDBusConnection::systemBus().send(message);
            }
        } else {
//            qDebug() << "visiable";

            m_qsettings->beginGroup("application");
            if (m_qsettings->contains(nameExec))    // 判断"键"是否存在
            {
                 m_qsettings->remove(nameExec);      // 删除此键
//                 qDebug() << "remove";
            }
            else{
//                qDebug() << "not found";
            }
            m_qsettings->endGroup();
            m_qsettings->sync();
            if(!nameExec.contains("TXEduPlatformRuntime")){
               QProcess p(this);
               if (nameExec.contains("/usr")) {
                   nameExec = getExec(filename);
               }
               p.start(QString("setfacl -x u:%1 /usr/bin/%2").arg(username).arg(nameExec));
               p.waitForFinished();
            } else {
                m_qsettings2 = new QSettings(filename2, QSettings::IniFormat);
                m_qsettings2->beginGroup("application");
                QString id = interface->getAppId(filename);
                if (m_qsettings2->contains(id))    // 判断"键"是否存在
                {
                     m_qsettings2->remove(id);      // 删除此键
    //                 qDebug() << "remove";
                }
                else{
    //                qDebug() << "not found";
                }
                m_qsettings2->endGroup();
                m_qsettings2->sync();
                QDBusMessage message =QDBusMessage::createSignal("/com/ukui/desktop/software", "com.ukui.desktop.software", "send_id");
                message << id << visiable;
                qDebug() << id;
                QDBusConnection::systemBus().send(message);
            }
        }
    }
    else
        return 2;
    /*
    interface->getDesktopListFromDesktop(userid);
    for(int i = 0; i < interface->desktopfilePathList.size(); i++)
    {
        QString deskpath = interface->desktopfilePathList.at(i);
//        qDebug() << deskpath;

        QString fullnameExec = interface->getAppExec(deskpath);
//        qDebug() << fullnameExec;
        // 从/usr/bin/xxx --replace中提取出xxx
        QString str = fullnameExec.section(' ', 0, 0);
        QStringList list = str.split('/');
        str = list[list.size()-1];
        list = str.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
        if(list[0] == nameExec){
            qDebug() << "desktop" << nameExec << i;
            QString desktopfp = "file://" + deskpath;
            GFile*     fileDesktop = g_file_new_for_uri(desktopfp.toUtf8().constData());
            GFileInfo* info = g_file_info_new();
            GError*    errDesktop = nullptr;
            char*      value;
            if (visiable)
                value = "1";
            else
                value = "0";
            g_file_info_set_attribute(info, "metadata::exec_disable",
                                      G_FILE_ATTRIBUTE_TYPE_STRING, (gpointer)value);
            g_file_set_attribute(fileDesktop, "metadata::exec_disable", G_FILE_ATTRIBUTE_TYPE_STRING, (gpointer)value,
                                 G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, &errDesktop);
            if (errDesktop) {
                qDebug() << "error in desktop" << errDesktop->message;
                g_error_free(errDesktop);
            }
            g_object_unref(info);
            g_object_unref(fileDesktop);
        }
    }
    */

    nameExec = interface->getAppExec(filename);
    QDBusMessage message =QDBusMessage::createSignal("/com/ukui/desktop/software", "com.ukui.desktop.software", "send_to_client");
    message << nameExec << visiable;
    qDebug() << nameExec << visiable;
    QDBusConnection::systemBus().send(message);
    return 0;
}

/*
bool Service::GetLockState(quint32 id)
{
    get_usr_path(id);
    qDebug() << "username = " << username;
    QProcess process;
//    QString cmd = "su - " + username + " -c " + "\"gsettings get org.ukui.screensaver.state lock-state\"";
    QString cmd = "ps -ef \| grep ukui-screensaver-dialog \| grep " + username;
//    QString cmd = "ps -ef \| grep ukui-screensaver-dialog \| grep " + username + " \| wc -l";
    qDebug() << cmd;
    process.start("bash", QStringList() << "-c" << cmd);
    process.waitForStarted(); //等待程序启动
    process.waitForFinished();//等待程序关闭
//    qDebug() << process.readAllStandardError().data();
    QString temp = QString::fromLocal8Bit(process.readAll());//.trimmed();
//    QString temp(process.readAllStandardOutput().data());
    qDebug() << temp << endl;
    QStringList list = temp.split("\n");
    int number = 0;
    for (int i = 0; i < list.size(); i++)
    {
        qDebug() << list.at(i);
        QString name = list.at(i).section(" ",0, 0);
        qDebug() << "name = " << name;
        if(name == username)
            number++;
    }
    qDebug() << number;
    if(id != 0){
        if(number > 1)
            return true;
        else
            return false;
    }
    else{
        if(number > 2)
            return true;
        else
            return false;
    }

}


}
*/
