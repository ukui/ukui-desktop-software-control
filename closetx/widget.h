#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDBusInterface>

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
private:
    QDBusInterface  *m_pServiceInterface;
private slots:
    void closetx(QString id, bool status);
};
#endif // WIDGET_H
