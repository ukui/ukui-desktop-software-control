/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#ifndef UKUIMENUINTERFACE_H
#define UKUIMENUINTERFACE_H
#include <QtCore>
#include <sys/types.h>
#include <unistd.h>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QSettings>
class QStringList;

class UkuiMenuInterface
{
private:
protected:
    //递归搜索文件
    void recursiveSearchFile(const QString& _filePath);//遍历/usr/share/applications/文件夹

public:
    UkuiMenuInterface();
    ~UkuiMenuInterface();

    /**
     * 获取系统应用名称
     * @param desktopfp 为应用.desktop文件所在路径
     * @return 返回应用名称
     */
    QString getAppName(QString desktopfp);//获取应用名
    QString getAppEnglishName(QString desktopfp);//获取英语英文名
    QString getAppIcon(QString desktopfp);//获取应用图像
    QString getAppCategories(QString desktopfp);//获取应用分类
    QString getAppId(QString desktopfp);//获取应用Id
    QString getAppExec(QString desktopfp);//获取应用命令
    QString getAppType(QString desktopfp);//获取应用类型
    QString getAppComment(QString desktopfp);//获取应用注释
    QStringList getDesktopFilePath();//获取系统desktop文件路径
    QStringList getDesktopListFromDesktop(quint32 uid);//从用户桌面获取desktop文件绝度路径

    static bool cmpApp(QStringList &arg_1,QStringList &arg_2);
    bool matchingAppCategories(QString desktopfp,QStringList categorylist);//匹配应用Categories

    //获取用户图像
    QString getUserIcon();
    //获取用户姓名
    QString getUserName();
    QStringList filePathList;
    QStringList desktopfilePathList;

};

#endif // UKUIMENUINTERFACE_H
