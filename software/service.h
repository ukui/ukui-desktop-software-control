#ifndef SERVICE_H
#define SERVICE_H

#include <QObject>
#include <QString>
#include "ukuimenuinterface.h"
class QSettings;

class Service:public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.ukui.desktop.software")
public:
    Service();

public Q_SLOTS:

    //对一些app应用实现禁用的功能
    qint32 appVisiableControl(QString appid, bool visiable, quint32 userid);

    //给腾讯看的接口,能看到哪些应用被禁用了，
    QStringList alltxban(quint32 userid);

    QStringList txban(quint32 userid);

    //清除闹钟的服务
    void cleanwake();

    //提供闹钟的服务
    void rtcexec(int wake);

    //获取应用的程序命令，"Exec="
    QString getExec(QString filename);

//   bool GetLockState(quint32 id);
private:

    UkuiMenuInterface * interface;
    UkuiMenuInterface * interface2;

    QString get_usr_path(int id);
    QSettings *m_qsettings = nullptr;
    QSettings *m_qsettings2 = nullptr;
    QSettings *m_qsettings3 = nullptr;

    QString username;
Q_SIGNALS:
    void send_to_client(QString, bool);
    void send_id(QString, bool);
};

#endif // SERVICE_H
