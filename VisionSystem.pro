################################
#
# QMake definitions for simple example
#

include ($$(ARTKP)/build/linux/options.pro)

TEMPLATE = app

TARGET   = VisionSystem

QMAKE_CLEAN = $$(ARTKP)/bin/VisionSystem

LIBS += -L$$(ARTKP)/lib -lARToolKitPlus

LIBS += -L$$(MIXLIB) -losg -losgART -losgProducer -losgText -losgGA -losgUtil -losgDB
LIBS += -L$$(MIXLIB) -lOpenThreads -lProducer

LIBS += -L/usr/local/lib -llcm
LIBS += -L/usr/lib -lcv -lcxcore -lhighgui

INCLUDEPATH += $$(MIXINC)
INCLUDEPATH += /usr/local/include

debug {
  OBJECTS_DIR     = $$(ARTKP)/VisionSystem/build/linux/debug
}

release {
  OBJECTS_DIR     = $$(ARTKP)/VisionSystem/build/linux/release
}


DESTDIR  = $$(ARTKP)/bin

debug {
  message("Building VisionSystem in debug mode ...")
}

release {
  message("Building VisionSystem in release mode ...")
}

HEADERS += src/util.h \
    src/CamTracker.h \
    src/PluginManager.h \
    src/Plugin.h \
    $$files(src/lcmtypes/*.h) \
    src/visionsystem.h \
    src/TablePlugin.h \
    src/CubePlugin.h \
    src/BallPlugin.h

SOURCES += src/main.cpp \
    src/util.cpp \
    src/CamTracker.cpp \
    src/Plugin.cpp \
    $$files(src/lcmtypes/*.c) \
    src/visionsystem.cpp \
    src/TablePlugin.cpp \
    src/CubePlugin.cpp \
    src/BallPlugin.cpp

target.path = ""/$$PREFIX/bin
INSTALLS += target

################################
