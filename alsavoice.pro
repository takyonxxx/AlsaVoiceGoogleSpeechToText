QT += core gui multimedia

CONFIG += c++17 console
CONFIG -= app_bundle
TARGET = alsatranslator

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        alsarecorder.cpp \
        alsatranslator.cpp \
        main.cpp \
        qaudiotranslator.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    alsarecorder.h \
    alsatranslator.h \
    constants.h \
    qaudiotranslator.h

win32 {
    contains(TARGET_ARCH, x86_64){
        message("Win 64 bit enabled")

    }else{
        message("Win 32 bit enabled")
    }
}

macx{
    message("Macx enabled")
}

unix:!macx{
    message("Unix enabled")
    #QMAKE_INCDIR += /usr/local/include

    QMAKE_LIBDIR += usr/lib
    QMAKE_LIBDIR += /usr/local/lib

    INCLUDEPATH += /usr/local/include
    INCLUDEPATH += /usr/local/include/pocketsphinx
    INCLUDEPATH += /usr/local/include/sphinxbase
    LIBS += -lasound -lsphinxbase -lpocketsphinx -lsphinxad
    LIBS += -lFLAC
}

DISTFILES += \
    glscope/README

#sudo apt-get install bison
#sudo apt-get install swig
#sudo apt-get install libasound2-dev
#sudo apt-get install alsa alsa-tools
#sudo apt install libsphinxbase-dev
#sudo apt-get install libfftw3-dev
#sudo apt-get install libflac-dev
#pocketsphinx_continuous -infile test.wav -keyphrase "oh mighty computer" -kws_threshold 1e-20
#sudo ldconfig
# flac -c -d *flac | aplay

RESOURCES +=

FORMS +=


