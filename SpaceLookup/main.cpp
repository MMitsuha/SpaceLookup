#include "stdafx.h"
#include "SpaceLookup.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SpaceLookup w;
    w.show();
    return a.exec();
}
