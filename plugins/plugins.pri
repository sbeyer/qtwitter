include(../common.pri)

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += $${TOP}/qtwitter-app/src
DESTDIR = $${DESTDIR}/plugins

# install
isEmpty( PREFIX ):INSTALL_PREFIX = /usr
else:INSTALL_PREFIX = $${PREFIX}
target.path = $${INSTALL_PREFIX}/lib$${LIB_SUFFIX}/$${APP_NAME}/plugins

INSTALLS += target