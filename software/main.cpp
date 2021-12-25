#include <QCoreApplication>
#include <QDebug>
#include <QtDBus>
#include <QString>
#include <thread>
#include "service.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Service service;
//    UkuiMenuInterface face;
//    qDebug()<<face.getAppExec("/home/jiaodian/桌面/wps-office-pdf.desktop");
//    qDebug()<<service.getExec("/home/jiaodian/桌面/wps-office-pdf.desktop");
    return a.exec();
}
