#include <QtGui>
#include <QCoreApplication>
#include "SlotTimer.h"
#include "ui_mainwindow.h"
#include <vector>
#include <algorithm>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

/**
*Author: Andrew Elmendorf
*Date: 3/20/17
*To-Do:
*   1:Add Qtimer object to each slot
*   2:convert BoardSlots to a class
*   3:Cleanup your damn code!  There is a lot to fix here.
*
*/

SlotTimer::SlotTimer(QWidget *parent) :QMainWindow(parent),ui(new Ui::SlotTimer)
{
    ui->setupUi(this);
    this->arduino=new QSerialPort();
    this->connected=false;

    //Setup the table
    this->model = new QStandardItemModel(10,7,this);
    this->model->setHorizontalHeaderItem(0, new QStandardItem(QString("Name")));
    this->model->setHorizontalHeaderItem(1, new QStandardItem(QString("BoardSlot")));
    this->model->setHorizontalHeaderItem(2, new QStandardItem(QString("Start Time")));
    this->model->setHorizontalHeaderItem(3, new QStandardItem(QString("Last Measure")));
    this->model->setHorizontalHeaderItem(4, new QStandardItem(QString("Duration")));
    this->model->setHorizontalHeaderItem(5, new QStandardItem(QString("Elapsed")));
    this->model->setHorizontalHeaderItem(6, new QStandardItem(QString("Reading")));
    this->model->setHorizontalHeaderItem(7, new QStandardItem(QString("Running?")));

    //build slot list
    for(int i=0;i<8;i++)
        this->boardSlots.push_back(BoardSlot(0,i+1));

    //set status table status to not loaded
    for(int i=0;i<8;i++){
        QStandardItem *item = new QStandardItem(QString("Not Loaded"));
        this->model->setItem(i,0,item);
        //this->boardSlots.push_back(BoardSlot(0));
    }//End for loop

    ui->timerList->model();
    ui->timerList->setModel(this->model);
}//End constructor, init program

void SlotTimer::on_Start_t_clicked()
{
    bool found=false;
    int index=0;

    //Find index of requested slot
    for(int i=0;i<this->boardSlots.size();i++){
        if(this->boardSlots[i].channel==(long)ui->channel_in->value()){
            found=true;
            index=i;
        }//end if == channel
    }//end loop through

    if(found){
        if(!this->boardSlots[index].running && !this->boardSlots[index].loaded){

            //set the table values
            this->boardSlots[index].duration_t=(long)ui->duration->value()*this->hrsTomilli;
            this->boardSlots[index].channel=(long)ui->channel_in->value();
            this->boardSlots[index].start=QTime::currentTime();
            this->boardSlots[index].current=this->boardSlots[index].start;
            this->boardSlots[index].inc=(long)ui->increment->value()*this->minsToMilli;
            this->boardSlots[index].elapsed_t=this->boardSlots[index].inc;
            this->boardSlots[index].running=true;
            this->boardSlots[index].loaded=true;
            this->boardSlots[index].slotName=ui->testName_in->text();
            this->boardSlots[index].filename+=this->boardSlots[index].slotName+"_data.txt";

            ui->current->setText(this->boardSlots[index].start.toString("hh:mm:ss"));

            this->boardSlots[index].ID=startTimer((int)this->boardSlots[index].inc);

            ui->ID->setText(QString::number(this->boardSlots[index].ID));

            this->fillTable(this->boardSlots[index]);


            //Write out to the arduino
            // "PX Y"  P: switching relay designator X:channel 1-8  Y:  on or off
            QByteArray output;
            QString txt;
            QTextStream(&txt)<<"P"<<this->boardSlots[index].channel<<" 0\n";
            output =txt.toLocal8Bit();
            this->arduino->write(output);
            this->arduino->flush();

        }else{
            this->sendUserMessage("Warning","Board Loaded");
        }//End check for slot loaded
    }else{
        this->sendUserMessage("Warning","Slot Not Found");
    }//End for slot found
}//End start timer procedure

void SlotTimer::timerEvent(QTimerEvent *e)
{

    std::vector<BoardSlot>::iterator itr;
    itr = std::find (this->boardSlots.begin(),this->boardSlots.end(),BoardSlot(e->timerId()));

    if (itr != this->boardSlots.end()){
          itr->elapsed_t=itr->elapsed_t+itr->inc;
          if(!itr->check()){
              itr->current=QTime::currentTime();
              this->fillTable(*itr);

              //Request Current Reading
              QByteArray output;
              QString txt;
              QTextStream(&txt)<<"R"<<itr->channel<<"\n";
              output =txt.toLocal8Bit();
              this->arduino->write(output);
              this->arduino->flush();
          }else{
             itr->running=false;

             this->model->setItem(itr->channel-1,7,new QStandardItem(QString("Done")));
             killTimer(itr->ID);

             //sending a message to arduino
             QByteArray output;
             QString txt;
             // "PX Y"  P: switching relay designator X:channel 1-8  Y:  on or off
             QTextStream(&txt)<<"P"<<itr->channel<<" 1\n";
             output =txt.toLocal8Bit();
             this->arduino->write(output);
             this->arduino->flush();
          }//if not finished do continue on, if done stop it all
    }else{
        this->sendUserMessage("Error","No Such slot");
    }// if slot not found.  should never happen
}//End timer event

void SlotTimer::readData()
{
    if (!this->arduino){
        qDebug()<<"Why am I here!?!";
        //Need a message
        return;
    }
        QByteArray in;

        //get buffer size and resize
        int buffSize=this->arduino->bytesAvailable();
        in.resize(buffSize);

        //read and append the data
        this->arduino->read(in.data(),in.size());
        this->recieved.append(in);

        //if it is a newline stop and finalize read.  there is no guarantee that this
        //function will be called once for one command.  have to make a buffer in struct/class and append until finished
        if(in.contains('\n')){
            QString str = QString::fromUtf8(recieved.split('\n').first());
            ui->current->setText(str);
            int index=str.split(':').first().toInt()-1;
            this->boardSlots[index].lastRead=str.split(':').last().toLong();
            this->writeDataOut(this->boardSlots[index]);
            recieved.clear();
        }//End terminator check
}//End serial read data

void SlotTimer::setControlsEnabled(bool enable)
{
    ui->Start_t->setEnabled(enable);
}//End button enabled

void SlotTimer::on_connectSerial_clicked()
{
    QSerialPortInfo port;

    //Find the arduino port using vendor and manufacturer IDs
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()){
        ui->serialPortList->addItem(info.portName());
        if(info.vendorIdentifier()==this->arduinoVendorID && info.productIdentifier()==this->megaProductID){
            qDebug()<<info.manufacturer();
            qDebug()<<info.vendorIdentifier();
            qDebug()<<info.productIdentifier();
            port=info;
            //found=true;
        }//end check for arduino
    }//end iteration of open ports

    //Set the port up!
    this->arduino->setPortName(port.portName());
    this->arduino->setBaudRate(QSerialPort::Baud9600);
    this->arduino->setDataBits(QSerialPort::Data8);
    this->arduino->setParity(QSerialPort::NoParity);
    this->arduino->setStopBits(QSerialPort::OneStop);
    this->arduino->setFlowControl(QSerialPort::NoFlowControl);
    this->arduino->open(QIODevice::ReadWrite);
    this->arduino->setDataTerminalReady(true);

    //Connect the slot for readData()
    connect(arduino, SIGNAL(readyRead()),this,SLOT(readData()));

    //making sure setup worked.
    this->connected=this->arduino->isWritable() && this->arduino->isReadable();

    if(this->connected==true){
        ui->serialStatus->setText("Connected");
        qDebug()<<"Success!";
    }// if success!!
}//connect serial

void SlotTimer::fillTable(const BoardSlot &s)
{
    this->model->setItem(s.channel-1,0,new QStandardItem(s.slotName));
    this->model->setItem(s.channel-1,1,new QStandardItem(QString::number(s.channel)));
    this->model->setItem(s.channel-1,2,new QStandardItem(QString(s.start.toString("hh:mm:ss"))));
    this->model->setItem(s.channel-1,3,new QStandardItem(QString(s.current.toString("hh:mm:ss"))));
    this->model->setItem(s.channel-1,4,new QStandardItem(QString::number((double)s.duration_t/SlotTimer::hrsTomilli)));
    this->model->setItem(s.channel-1,5,new QStandardItem(QString::number((double)s.elapsed_t/SlotTimer::hrsTomilli)));
    if(s.running){
        this->model->setItem(s.channel-1,7,new QStandardItem(QString("Running")));
    }else{
        this->model->setItem(s.channel-1,7,new QStandardItem(QString("Not Running")));
    }//End check for running
        this->model->setItem(s.channel-1,6,new QStandardItem(QString::number(s.lastRead)));
}//End fill table

void SlotTimer::clearTable()
{
    for(int i=0;i<8;i++){
        this->model->setItem(i,0,new QStandardItem("Not Loaded"));
        this->model->setItem(i,1,new QStandardItem(" "));
        this->model->setItem(i,2,new QStandardItem(" "));
        this->model->setItem(i,3,new QStandardItem(" "));
        this->model->setItem(i,4,new QStandardItem(" "));
        this->model->setItem(i,5,new QStandardItem(" "));
        this->model->setItem(i,6,new QStandardItem(" "));
        this->model->setItem(i,7,new QStandardItem(" "));
    }//End for loop
}//End fill table


void SlotTimer::clearSlot(int s)
{
    this->model->setItem(s,0,new QStandardItem("Not Loaded"));
    this->model->setItem(s,1,new QStandardItem(" "));
    this->model->setItem(s,2,new QStandardItem(" "));
    this->model->setItem(s,3,new QStandardItem(" "));
    this->model->setItem(s,4,new QStandardItem(" "));
    this->model->setItem(s,5,new QStandardItem(" "));
    this->model->setItem(s,6,new QStandardItem(" "));
    this->model->setItem(s,7,new QStandardItem(" "));
}//End fill table

void SlotTimer::on_timerList_clicked(const QModelIndex &index)
{
    qDebug()<<index.row();
}


void SlotTimer::on_Unload_Chan_clicked()
{
    if(!this->boardSlots[ui->channel_in->value()-1].running){
        int channel=ui->channel_in->value();
        QString msg="Unload channel "+QString::number(channel)+"?";
        if(this->getUserResponce("Unload?",msg))
        {
         this->boardSlots[channel-1].reset();
         this->clearSlot(channel-1);
        }//end check user response
    }else{
        this->sendUserMessage("Warning","Board Loaded");
    }//End check if slot as running board
}//End unload

void SlotTimer::on_Stop_Chan_clicked()
{
    if(this->boardSlots[ui->channel_in->value()-1].running){
        int channel=ui->channel_in->value();
        QString msg="Stop channel "+QString::number(channel)+"?";
        if(this->getUserResponce("Unload?",msg))
        {

            this->boardSlots[ui->channel_in->value()-1].running=false;
            killTimer(this->boardSlots[ui->channel_in->value()-1].ID);
            QByteArray output;
            QString txt;
            QTextStream(&txt)<<"P"<<ui->channel_in->value()<<" 1\n";
            output =txt.toLocal8Bit();
            this->arduino->write(output);
            this->arduino->flush();
        }//End check with user
    }else{
        this->sendUserMessage("Error","Not Running");
    }//End check if running
}//End stop channel

void SlotTimer::writeDataOut(const BoardSlot &s)
{
    QFile out(s.filename);
    if (!out.open(QIODevice::ReadOnly | QIODevice::Text | QIODevice::ReadWrite |QIODevice::Append)){
        qDebug() << "FAIL TO CREATE FILE / FILE NOT EXIT***";
    }else{
        QTextStream stream(&out);
        QDateTime temp=QDateTime::currentDateTime();
        stream <<s.lastRead<<"\t"<<temp.toString("dd.MM.yyyy")<<"\t"<<s.current.toString("hh:mm:ss")<<endl;
        out.close();
    }//End check if file
}//End write data out

bool SlotTimer::getUserResponce(const QString &t,const QString &msg)
{
    bool ret=false;
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(0,t,msg,QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
      ret=true;
    } else {
        ret=false;
    }//End set ret answer
    return ret;
}//end request feedback

void SlotTimer::sendUserMessage(const QString &t,const QString &msg)
{
    QMessageBox messageBox;
    messageBox.critical(0,t,msg);
    messageBox.setFixedSize(500,200);
}//End send message

SlotTimer::~SlotTimer()
{
    if(this->arduino->isOpen()){
        this->arduino->close();
    }//end serial open check
    delete ui;
}//

