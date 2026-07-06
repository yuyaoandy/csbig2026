QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    AutoWrapHelper.cpp \
    CardSelectionWidget.cpp \
    battlecore.cpp \
    battlewindow.cpp \
    blocktowerwidget.cpp \
    gamewindow.cpp \
    jsonreader.cpp \
    main.cpp \
    map_gen.cpp \
    saveloadmanager.cpp \
    storewidget.cpp \
    storyloader.cpp \
    storynode.cpp \
    style.cpp \
    towerdisplaywidget.cpp \
    widget.cpp

HEADERS += \
    AutoWrapHelper.h \
    CardSelectionWidget.h \
    battlecore.h \
    battlewindow.h \
    blocktowerwidget.h \
    card.h \
    gamewindow.h \
    jsonreader.h \
    map_gen.h \
    saveloadmanager.h \
    scrollable.h \
    storewidget.h \
    storyloader.h \
    storynode.h \
    storywidget.h \
    style.h \
    towerdisplaywidget.h \
    widget.h

FORMS += \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icons.qrc

DISTFILES += \
    data/enemy.json
RC_ICONS = icon.ico