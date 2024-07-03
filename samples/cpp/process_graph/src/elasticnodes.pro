QT += widgets

FORMS       = mainwindow.ui
HEADERS     += \
            edge.h \
            node.h \
            graphwidget.h \
            treeitem.h \
            treemodel.h \
            mainwindow.h
RESOURCES   = editabletreemodel.qrc
SOURCES     += \
            edge.cpp \
            main.cpp \
            node.cpp \
            graphwidget.cpp \
            treeitem.cpp \
            treemodel.cpp \
            mainwindow.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/graphicsview/elasticnodes
INSTALLS += target
