#ifndef SLOTTIMER_H
#define SLOTTIMER_H

#include <QMainWindow>
#include <QtGui>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <string>


namespace Ui
{
    class SlotTimer;
}//UI namespace

class SlotTimer : public QMainWindow
{
    Q_OBJECT

public:
    explicit SlotTimer(QWidget *parent = 0);
    ~SlotTimer();

    struct BoardSlot
    {
        QTime current,start;
        long elapsed_t;
        long duration_t;
        long inc;
        long ID;
        int channel;
        bool running;
        bool loaded;
        long lastRead;
        QString slotName;
        QString filename;

        //Constructor overload for timerID
        BoardSlot(int id)
        {
            ID=id;
            loaded=false;
            running=false;
            lastRead=0;
            filename="/home/pi/Monitor/";
            slotName="";
            elapsed_t=0;
            duration_t=0;
        }//end timerID overload

        //constructor overload for initial channel and timerID
        BoardSlot(int id,int c)
        {
            ID=id;
            channel=c;
            loaded=false;
            running=false;
            lastRead=0;
            filename="/home/pi/Monitor/";
            slotName="";
            elapsed_t=0;
            duration_t=0;
        }//end channel/timerID overload

        //reset this timer
        void reset()
        {
            ID=0;
            loaded=false;
            running=false;
            lastRead=0;
            filename="/home/pi/Monitor/";
            slotName="";
            elapsed_t=0;
            duration_t=0;
        }//End timer reset

        //checking if done
        bool check()
        {
            return elapsed_t>duration_t;
        }//end check if done

        //equals overload for vector search
        bool operator==(const BoardSlot& b) const
        {
          return ID == b.ID;
        }//end equals overload
    };//End BoardSlot struct

    //Constants for timer conversion
    static const long hrsTomilli=3600000;
    static const long minsToMilli=60000;


    QSerialPort *arduino;  //serial port of arduino
    QStandardItemModel *model; //model for building display table
    std::vector<BoardSlot> boardSlots;//  using only in timerEvent.  really don't need a vector, left because ran out of time
    bool connected; //are we connected/
    QByteArray recieved; //from arduino buffer
    void setControlsEnabled(bool enable);//enable/disable controls.  NOT EVEN USING
    void fillTable(const BoardSlot &s);//fille the table
    void writeDataOut(const BoardSlot &s);//write out to a txt file
    void clearTable();//clear the table
    void sendUserMessage(const QString &t,const QString &msg);//send a message a user
    bool getUserResponce(const QString &t,const QString &msg);//send a message a user requesting feedback
    void clearSlot(int s);//clear that slot!

private slots:
    void timerEvent(QTimerEvent *e);//timer events!
    void readData();//read in serial
    void on_Start_t_clicked();//start a timer
    void on_connectSerial_clicked();//connect to the arduino
    void on_timerList_clicked(const QModelIndex &index);//not even used, was going to have the table as the slot selector.  still might
    void on_Unload_Chan_clicked();//unload a channel
    void on_Stop_Chan_clicked();//stop a channel


private:
    Ui::SlotTimer *ui;
    static const quint16 arduinoVendorID=9025;//arduino vendor ID for finding the port
    static const quint16 megaProductID=66;//arduino atmega product ID ID for finding the port

    void processError(const QString &error);//not even using, will implement later
    void initSerial();//initialize the arduino com!

};

#endif // SLOTTIMER_h
