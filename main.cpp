#include <QApplication>
#include "SlotTimers.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SlotTimers w;
    w.show(); 
    return a.exec();
}//
