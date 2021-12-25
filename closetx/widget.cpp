#include "widget.h"
#include <QDebug>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    m_pServiceInterface = new QDBusInterface("com.ukui.desktop.software",
                                                             "/com/ukui/desktop/software",
                                                             "com.ukui.desktop.software",
                                                             QDBusConnection::systemBus());
    connect(m_pServiceInterface,SIGNAL(send_id(QString, bool)),this,SLOT(closetx(QString,bool)));

}

Widget::~Widget()
{
    delete m_pServiceInterface;
}
void Widget::closetx(QString id, bool status)
{
    qDebug()<<id;
    if (!status){
//        QDBusInterface iface("cn.txeduplatform.server",
//                             "/cn/txeduplatform/server",
//                             "cn.txeduplatform.server.apps",
//                             QDBusConnection::sessionBus());
//        iface.call("Close",id);
    }
}

