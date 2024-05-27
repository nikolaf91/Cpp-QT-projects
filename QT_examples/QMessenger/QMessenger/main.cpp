#include "stdafx.h"
#include "QMessenger.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QMessenger w;
    w.show();
    return a.exec();
}
