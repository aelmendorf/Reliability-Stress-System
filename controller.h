#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "twrap.h"
#include <QtGui>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <string>

enum comType
{
  READ=3,WRITE=4,ON=1,OFF=0
};

namespace Ui {
class Controller;
}

class Controller : public QMainWindow
{
    Q_OBJECT

public:

    QSerialPort *arduino;
    QStandardItemModel *model;
    bool connected;
    QByteArray recieved;

    std::vector<tWrap*> task;
    void writeDataOut(const tWrap &s);
    bool buildDir(const QString &dir);
    void setControlsEnabled(bool enable);
    void fillTable(const tWrap &s);
    void clearTable();
    void clearSlot(int s);
    bool getUserResponce(const QString &t,const QString &msg);
    void sendUserMessage(const QString &t,const QString &msg);
    void arduinoMSG(comType action,comType param,int c);
    explicit Controller(QWidget *parent = 0);
    ~Controller();

private slots:
    void on_startB_clicked();
    void on_StopB_clicked();
    void on_connectSerial_clicked();
    void readData();
    void timerEvent(QTimerEvent *event);
    void on_unload_B_clicked();

private:
    Ui::Controller *ui;
    static const quint16 arduinoVendorID=9025;
    static const quint16 megaProductID=66;

};





#endif // CONTROLLER_H
