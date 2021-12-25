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

#include "ukuimenuinterface.h"
#include <glib.h>
#include <QDir>
#include <QDebug>
#include <QCollator>
#include <QLocale>
#include <QStringList>

UkuiMenuInterface::UkuiMenuInterface()
{
    getDesktopFilePath();//获取系统deskyop文件路径
//    getDesktopListFromDesktop(1000);
}

UkuiMenuInterface::~UkuiMenuInterface()
{
}

//文件查询桌面目录的desktop文件
QStringList UkuiMenuInterface::getDesktopListFromDesktop(quint32 uid)
{
    desktopfilePathList.clear();

    QString usrpath;
    QFile file2("/etc/passwd");
    if(file2.open(QIODevice::ReadOnly))
    {
        while(!file2.atEnd())
        {
            QByteArray line = file2.readLine();
            QString str(line);
            QStringList list = str.split(':');
            if(list[2] == QString::number(uid))
            {
                QString str1 = list[0];
                usrpath = "/home/" + str1;
            }
        }
    }
    file2.close();

//    qDebug() << "Message send!";
    QLocale locale;
    QString desktop_path;
    if (locale.language() == QLocale::Chinese)
        desktop_path = usrpath + "/桌面";
    else
        desktop_path = usrpath + "/Desktop";
//    qDebug() << desktop_path;

    GError** error=nullptr;
    GKeyFileFlags flags=G_KEY_FILE_NONE;
    GKeyFile* keyfile=g_key_file_new ();

    QDir dir(desktop_path);
    if (!dir.exists()) {
        return desktopfilePathList;
    }
    dir.setFilter(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot);
    dir.setSorting(QDir::DirsFirst);
    QFileInfoList list = dir.entryInfoList();
//    qDebug() << list;
    if(list.size()< 1 ) {
        return desktopfilePathList;
    }
    int i=0;

    //递归算法的核心部分
    do{
        QFileInfo fileInfo = list.at(i);
        //如果是文件夹，递归
        bool isDir = fileInfo.isDir();
        if(isDir) {
            i++;
            continue;
        }
        else{
            //过滤后缀不是.desktop的文件
            QString filePathStr=fileInfo.filePath();
//            qDebug() << filePathStr << i;
            if(!filePathStr.endsWith(".desktop"))
            {
                i++;
                continue;
            }
            QByteArray fpbyte=filePathStr.toLocal8Bit();
            char* filepath=fpbyte.data();
            g_key_file_load_from_file(keyfile,filepath,flags,error);
            char* ret_1=g_key_file_get_locale_string(keyfile,"Desktop Entry","NoDisplay", nullptr, nullptr);
            if(ret_1!=nullptr)
            {
                QString str=QString::fromLocal8Bit(ret_1);
                if(str.contains("true"))
                {
                    i++;
                    continue;
                }
            }
            char* ret_2=g_key_file_get_locale_string(keyfile,"Desktop Entry","NotShowIn", nullptr, nullptr);
            if(ret_2!=nullptr)
            {
                QString str=QString::fromLocal8Bit(ret_2);
                if(str.contains("UKUI"))
                {
                    i++;
                    continue;
                }
            }

            //过滤LXQt、KDE
            char* ret=g_key_file_get_locale_string(keyfile,"Desktop Entry","OnlyShowIn", nullptr, nullptr);
            if(ret!=nullptr)
            {
                QString str=QString::fromLocal8Bit(ret);
                if(str.contains("LXQt") || str.contains("KDE"))
                {
                    i++;
                    continue;
                }
            }
            //过滤中英文名为空的情况
            QLocale cn;
            QString language=cn.languageToString(cn.language());
            if(QString::compare(language,"Chinese")==0)
            {
                char* nameCh=g_key_file_get_string(keyfile,"Desktop Entry","Name[zh_CN]", nullptr);
                char* nameEn=g_key_file_get_string(keyfile,"Desktop Entry","Name", nullptr);
                if(QString::fromLocal8Bit(nameCh).isEmpty() && QString::fromLocal8Bit(nameEn).isEmpty())
                {
                    i++;
                    continue;
                }
            }
            else {
                char* name=g_key_file_get_string(keyfile,"Desktop Entry","Name", nullptr);
                if(QString::fromLocal8Bit(name).isEmpty())
                {
                    i++;
                    continue;
                }
            }

            desktopfilePathList.append(filePathStr);
//            qDebug() << "standard" << filePathStr << i;
        }
        i++;

    } while(i < list.size());

    g_key_file_free(keyfile);

    return desktopfilePathList;
}

//文件递归查询，保存到私有成员filePathList里
void UkuiMenuInterface::recursiveSearchFile(const QString& _filePath)
{
    GError** error=nullptr;
    GKeyFileFlags flags=G_KEY_FILE_NONE;
    GKeyFile* keyfile=g_key_file_new ();

    QDir dir(_filePath);
    if (!dir.exists()) {
        return;
    }

    dir.setFilter(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot);
    dir.setSorting(QDir::DirsFirst);
    QFileInfoList list = dir.entryInfoList();
    list.removeAll(QFileInfo("/usr/share/applications/screensavers"));
    if(list.size()< 1 ) {
        return;
    }
    int i=0;

    //递归算法的核心部分
    do{
        QFileInfo fileInfo = list.at(i);
        //如果是文件夹，递归
        bool isDir = fileInfo.isDir();
        if(isDir) {
            recursiveSearchFile(fileInfo.filePath());
        }
        else{
            //过滤后缀不是.desktop的文件
            QString filePathStr=fileInfo.filePath();
            if(!filePathStr.endsWith(".desktop"))
            {
                i++;
                continue;
            }
            QByteArray fpbyte=filePathStr.toLocal8Bit();
            char* filepath=fpbyte.data();
            g_key_file_load_from_file(keyfile,filepath,flags,error);
            char* ret_1=g_key_file_get_locale_string(keyfile,"Desktop Entry","NoDisplay", nullptr, nullptr);
            if(ret_1!=nullptr)
            {
                QString str=QString::fromLocal8Bit(ret_1);
                if(str.contains("true"))
                {
                    i++;
                    continue;
                }
            }
            char* ret_2=g_key_file_get_locale_string(keyfile,"Desktop Entry","NotShowIn", nullptr, nullptr);
            if(ret_2!=nullptr)
            {
                QString str=QString::fromLocal8Bit(ret_2);
                if(str.contains("UKUI"))
                {
                    i++;
                    continue;
                }
            }

            //过滤LXQt、KDE
            char* ret=g_key_file_get_locale_string(keyfile,"Desktop Entry","OnlyShowIn", nullptr, nullptr);
            if(ret!=nullptr)
            {
                QString str=QString::fromLocal8Bit(ret);
                if(str.contains("LXQt") || str.contains("KDE"))
                {
                    i++;
                    continue;
                }
            }
            //过滤中英文名为空的情况
            QLocale cn;
            QString language=cn.languageToString(cn.language());
            if(QString::compare(language,"Chinese")==0)
            {
                char* nameCh=g_key_file_get_string(keyfile,"Desktop Entry","Name[zh_CN]", nullptr);
                char* nameEn=g_key_file_get_string(keyfile,"Desktop Entry","Name", nullptr);
                if(QString::fromLocal8Bit(nameCh).isEmpty() && QString::fromLocal8Bit(nameEn).isEmpty())
                {
                    i++;
                    continue;
                }
            }
            else {
                char* name=g_key_file_get_string(keyfile,"Desktop Entry","Name", nullptr);
                if(QString::fromLocal8Bit(name).isEmpty())
                {
                    i++;
                    continue;
                }
            }

            filePathList.append(filePathStr);
        }
        i++;

    } while(i < list.size());

    g_key_file_free(keyfile);

}

//获取系统deskyop文件路径
QStringList UkuiMenuInterface::getDesktopFilePath()
{
    filePathList.clear();
//    递归算法在QT中的实现（QT遍历文件夹）
    recursiveSearchFile("/usr/share/applications/");
    /*
    filePathList.removeAll("/usr/share/applications/software-properties-livepatch.desktop");
    filePathList.removeAll("/usr/share/applications/mate-color-select.desktop");
    filePathList.removeAll("/usr/share/applications/blueman-adapters.desktop");
    filePathList.removeAll("/usr/share/applications/blueman-manager.desktop");
    filePathList.removeAll("/usr/share/applications/mate-user-guide.desktop");
    filePathList.removeAll("/usr/share/applications/nm-connection-editor.desktop");
    filePathList.removeAll("/usr/share/applications/debian-uxterm.desktop");
    filePathList.removeAll("/usr/share/applications/debian-xterm.desktop");
    filePathList.removeAll("/usr/share/applications/im-config.desktop");
    filePathList.removeAll("/usr/share/applications/fcitx.desktop");
    filePathList.removeAll("/usr/share/applications/fcitx-configtool.desktop");
    filePathList.removeAll("/usr/share/applications/onboard-settings.desktop");
    filePathList.removeAll("/usr/share/applications/info.desktop");
    filePathList.removeAll("/usr/share/applications/ukui-power-preferences.desktop");
    filePathList.removeAll("/usr/share/applications/ukui-power-statistics.desktop");
    filePathList.removeAll("/usr/share/applications/software-properties-drivers.desktop");
    filePathList.removeAll("/usr/share/applications/software-properties-gtk.desktop");
    filePathList.removeAll("/usr/share/applications/gnome-session-properties.desktop");
    filePathList.removeAll("/usr/share/applications/org.gnome.font-viewer.desktop");
    filePathList.removeAll("/usr/share/applications/xdiagnose.desktop");
    filePathList.removeAll("/usr/share/applications/gnome-language-selector.desktop");
    filePathList.removeAll("/usr/share/applications/mate-notification-properties.desktop");
    filePathList.removeAll("/usr/share/applications/transmission-gtk.desktop");
    filePathList.removeAll("/usr/share/applications/mpv.desktop");
    filePathList.removeAll("/usr/share/applications/system-config-printer.desktop");
    filePathList.removeAll("/usr/share/applications/org.gnome.DejaDup.desktop");
    filePathList.removeAll("/usr/share/applications/yelp.desktop");
    filePathList.removeAll("/usr/share/applications/peony-computer.desktop");
    filePathList.removeAll("/usr/share/applications/peony-home.desktop");
    filePathList.removeAll("/usr/share/applications/peony-trash.desktop");
    filePathList.removeAll("/usr/share/applications/peony.desktop");

    //v10
    filePathList.removeAll("/usr/share/applications/mate-about.desktop");
    filePathList.removeAll("/usr/share/applications/time.desktop");
    filePathList.removeAll("/usr/share/applications/network.desktop");
    filePathList.removeAll("/usr/share/applications/shares.desktop");
    filePathList.removeAll("/usr/share/applications/mate-power-statistics.desktop");
    filePathList.removeAll("/usr/share/applications/display-im6.desktop");
    filePathList.removeAll("/usr/share/applications/display-im6.q16.desktop");
    filePathList.removeAll("/usr/share/applications/openjdk-8-policytool.desktop");
    filePathList.removeAll("/usr/share/applications/kylin-io-monitor.desktop");
    filePathList.removeAll("/usr/share/applications/wps-office-uninstall.desktop");
    filePathList.removeAll("/usr/share/applications/wps-office-misc.desktop");
    */
    return filePathList;
}

//获取应用名称
QString UkuiMenuInterface::getAppName(QString desktopfp)
{
    GError** error=nullptr;
    GKeyFileFlags flags=G_KEY_FILE_NONE;
    GKeyFile* keyfile=g_key_file_new ();

    QByteArray fpbyte=desktopfp.toLocal8Bit();
    char* filepath=fpbyte.data();
    g_key_file_load_from_file(keyfile,filepath,flags,error);

    char* name=g_key_file_get_locale_string(keyfile,"Desktop Entry","Name", nullptr, nullptr);
    QString namestr=QString::fromLocal8Bit(name);

    g_key_file_free(keyfile);
    return namestr;
}

//获取英应用英文名
QString UkuiMenuInterface::getAppEnglishName(QString desktopfp)
{
    GError** error=nullptr;
    GKeyFileFlags flags=G_KEY_FILE_NONE;
    GKeyFile* keyfile=g_key_file_new ();

    QByteArray fpbyte=desktopfp.toLocal8Bit();
    char* filepath=fpbyte.data();
    g_key_file_load_from_file(keyfile,filepath,flags,error);
    char* name=g_key_file_get_string(keyfile,"Desktop Entry","Name", nullptr);
    QString namestr=QString::fromLocal8Bit(name);
    return namestr;
}

//获取应用分类
QString UkuiMenuInterface::getAppCategories(QString desktopfp)
{
    GError** error=nullptr;
    GKeyFileFlags flags=G_KEY_FILE_NONE;
    GKeyFile* keyfile=g_key_file_new ();

    QByteArray fpbyte=desktopfp.toLocal8Bit();
    char* filepath=fpbyte.data();
    g_key_file_load_from_file(keyfile,filepath,flags,error);
    char* category=g_key_file_get_locale_string(keyfile,"Desktop Entry","Categories", nullptr, nullptr);
    g_key_file_free(keyfile);
    return QString::fromLocal8Bit(category);
}

//获取应用图标
QString UkuiMenuInterface::getAppIcon(QString desktopfp)
{   
    GError** error=nullptr;
    GKeyFileFlags flags=G_KEY_FILE_NONE;
    GKeyFile* keyfile=g_key_file_new ();

    QByteArray fpbyte=desktopfp.toLocal8Bit();
    char* filepath=fpbyte.data();
    g_key_file_load_from_file(keyfile,filepath,flags,error);
    char* icon=g_key_file_get_locale_string(keyfile,"Desktop Entry","Icon", nullptr, nullptr);
    g_key_file_free(keyfile);
    return QString::fromLocal8Bit(icon);
}

//获取应用命令
QString UkuiMenuInterface::getAppExec(QString desktopfp)
{
    GError** error=nullptr;
    GKeyFileFlags flags=G_KEY_FILE_NONE;
    GKeyFile* keyfile=g_key_file_new ();

    QByteArray fpbyte=desktopfp.toLocal8Bit();
    char* filepath=fpbyte.data();
    g_key_file_load_from_file(keyfile,filepath,flags,error);
    char* exec=g_key_file_get_locale_string(keyfile,"Desktop Entry","Exec", nullptr, nullptr);
    g_key_file_free(keyfile);
    return QString::fromLocal8Bit(exec);
}

//获取应用Id
QString UkuiMenuInterface::getAppId(QString desktopfp)
{
    GError** error=nullptr;
    GKeyFileFlags flags=G_KEY_FILE_NONE;
    GKeyFile* keyfile=g_key_file_new ();

    QByteArray fpbyte=desktopfp.toLocal8Bit();
    char* filepath=fpbyte.data();
    g_key_file_load_from_file(keyfile,filepath,flags,error);
    char* exec=g_key_file_get_locale_string(keyfile,"Desktop Entry","Appid", nullptr, nullptr);
    g_key_file_free(keyfile);
    return QString::fromLocal8Bit(exec);
}

//获取应用注释
QString UkuiMenuInterface::getAppComment(QString desktopfp)
{
    GError** error=nullptr;
    GKeyFileFlags flags=G_KEY_FILE_NONE;
    GKeyFile* keyfile=g_key_file_new ();

    QByteArray fpbyte=desktopfp.toLocal8Bit();
    char* filepath=fpbyte.data();
    g_key_file_load_from_file(keyfile,filepath,flags,error);
    char* comment=g_key_file_get_locale_string(keyfile,"Desktop Entry","Comment", nullptr, nullptr);
    g_key_file_free(keyfile);
    return QString::fromLocal8Bit(comment);

}

//获取应用类型
QString UkuiMenuInterface::getAppType(QString desktopfp)
{
    GError** error=nullptr;
    GKeyFileFlags flags=G_KEY_FILE_NONE;
    GKeyFile* keyfile=g_key_file_new ();

    QByteArray fpbyte=desktopfp.toLocal8Bit();
    char* filepath=fpbyte.data();
    g_key_file_load_from_file(keyfile,filepath,flags,error);
    char* type=g_key_file_get_locale_string(keyfile,"Desktop Entry","Type", nullptr, nullptr);
    g_key_file_free(keyfile);
    return QString::fromLocal8Bit(type);
}

bool UkuiMenuInterface::cmpApp(QStringList &arg_1, QStringList &arg_2)
{
    QLocale local;
    QString language=local.languageToString(local.language());
    if(QString::compare(language,"Chinese")==0)
        local=QLocale(QLocale::Chinese);
    else
        local=QLocale(QLocale::English);
    QCollator collator(local);
    if(collator.compare(arg_1.at(1),arg_2.at(1))<0)
        return true;
    else
        return false;
}

bool UkuiMenuInterface::matchingAppCategories(QString desktopfp, QStringList categorylist)
{
    QString category=getAppCategories(desktopfp);
    int index;
    for(index=0;index<categorylist.count();index++)
    {
        if(category.contains(categorylist.at(index),Qt::CaseInsensitive))
            return true;
    }
    if(index==categorylist.count())
        return false;

    return false;
}

//获取用户图像
QString UkuiMenuInterface::getUserIcon()
{
    qint64 uid=static_cast<qint64>(getuid());
    QDBusInterface iface("org.freedesktop.Accounts",
                         "/org/freedesktop/Accounts",
                         "org.freedesktop.Accounts",
                         QDBusConnection::systemBus());
    QDBusReply<QDBusObjectPath>objPath=iface.call("FindUserById",uid);

    QDBusInterface useriface("org.freedesktop.Accounts",
                             objPath.value().path(),
                             "org.freedesktop.DBus.Properties",
                             QDBusConnection::systemBus());
    QDBusReply<QVariant> var=useriface.call("Get","org.freedesktop.Accounts.User","IconFile");
    QString iconstr=var.value().toString();
    return iconstr;
}

QString UkuiMenuInterface::getUserName()
{
    qint64 uid=static_cast<qint64>(getuid());
    QDBusInterface iface("org.freedesktop.Accounts",
                         "/org/freedesktop/Accounts",
                         "org.freedesktop.Accounts",
                         QDBusConnection::systemBus());
    QDBusReply<QDBusObjectPath> objPath=iface.call("FindUserById",uid);
    QDBusInterface useriface("org.freedesktop.Accounts",
                             objPath.value().path(),
                             "org.freedesktop.DBus.Properties",
                             QDBusConnection::systemBus());
    QDBusReply<QVariant> var=useriface.call("Get","org.freedesktop.Accounts.User","UserName");
    QString name=var.value().toString();
    return name;
}
