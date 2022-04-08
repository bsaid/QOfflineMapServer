CONFIG += ordered

TEMPLATE = subdirs

SUBDIRS += qhttpserver \
           src

src.depends = qhttpserver
