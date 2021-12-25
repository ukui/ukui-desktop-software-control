QT -= gui
QT += dbus

TARGET = ukui_desktop_software_control

target.source += $$TARGET
target.path = /usr/sbin
service.files += etc/com-ukui-desktop-software.service
service.path = /lib/systemd/system/
config.files += etc/com.ukui.desktop.software.conf
config.path = /etc/dbus-1/system.d/


INSTALLS += target service config

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        service.cpp \
        ukuimenuinterface.cpp

HEADERS += \
    service.h \
    ukuimenuinterface.h

PKGCONFIG += glib-2.0 gio-2.0 gio-unix-2.0
CONFIG += link_pkgconfig no_keywords
