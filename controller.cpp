#include "controller.h"
#include "ui_controller.h"
#include <QTextStream>
#include <QDebug>
#include <QDir>



Controller::Controller(QWidget *parent) :QMainWindow(parent),ui(new Ui::Controller)
{
    ui->setupUi(this);
    this->arduino=new QSerialPort();
    this->connected=false;

    //setup table
    this->model = new QStandardItemModel(16,6,this);
    this->model->setHorizontalHeaderItem(0, new QStandardItem(QString("BoardSlot")));
    this->model->setHorizontalHeaderItem(1, new QStandardItem(QString("Start Time")));
    this->model->setHorizontalHeaderItem(2, new QStandardItem(QString("Duration")));
    this->model->setHorizontalHeaderItem(3, new QStandardItem(QString("Elapsed")));
    this->model->setHorizontalHeaderItem(4, new QStandardItem(QString("Reading")));
    this->model->setHorizontalHeaderItem(5, new QStandardItem(QString("Running?")));


    for(int i=0;i<16;i++)
    {
        QStandardItem *item = new QStandardItem(QString("Not Loaded"));
        this->model->setItem(i,0,item);
    }//End for loop

    ui->chanlist->setModel(this->model);
    this->setControlsEnabled(false);
}//End init

Controller::~Controller()
{
    for (std::vector<tWrap*>::iterator itr = this->task.begin() ; itr != this->task.end(); ++itr)
    {
        delete (*itr);
    }//end delele tWrap pointers
    this->task.clear();
    delete ui;
}//deconstructor

void Controller::on_startB_clicked()
{
    std::vector<tWrap*>::iterator itr;
    itr = std::find_if(this->task.begin(),this->task.end(),findChannel(ui->inChan->value()));
    if(itr==this->task.end())
    {
        tWrap *tmp=new tWrap(ui->inDur->value(),ui->inChan->value());
        tmp->start(this);
        //this->writeDataOut((*tmp));
        this->fillTable((*tmp));
        this->task.push_back(tmp);

        QFile f(tmp->getPath());
        f.open(QFile::WriteOnly|QFile::Truncate);
        f.close();
        this->arduinoMSG(WRITE,ON,tmp->chan);

        qDebug()<<"Timer Created:"<<tmp->sTimer->timerId();
        qDebug()<<"Wrapper ID:"<<tmp->ID;
    }else{
        this->sendUserMessage("Loaded","Channel Already Loaded. please unload and try again");
    }//
}//

void Controller::on_StopB_clicked()
{
    std::vector<tWrap*>::iterator itr;
    itr = std::find_if(this->task.begin(),this->task.end(),findChannel(ui->inChan->value()));
    if(itr != this->task.end())
    {
        if((*itr)->isRunning() && (*itr)->isLoaded())
        {
            QString msg="Stop channel "+QString::number((*itr)->chan)+"?";
            if(this->getUserResponce("Unload?",msg))
            {
                (*itr)->stop();
                this->model->setItem((*itr)->chan-1,5,new QStandardItem(QString("Done")));
                this->arduinoMSG(WRITE,OFF,(*itr)->chan);
            }//end check with user
        }else{
            this->sendUserMessage("Error","Channel is not running.  please unload if needed");
        }//End check if running/loaded
    }else{
        this->sendUserMessage("Not Found","Channel Not found. Please check selection");
    }//End looking for channel
}// End start procedure and checks

void Controller::timerEvent(QTimerEvent *e)
{
    std::vector<tWrap*>::iterator itr;
    itr = std::find_if(this->task.begin(),this->task.end(),findTimer(e->timerId()));
    if(itr != this->task.end())
    {
        (*itr)->addTime();
        if((*itr)->checkTime())
        {
           (*itr)->stop();
           this->model->setItem((*itr)->chan-1,5,new QStandardItem(QString("Done")));
           qDebug()<<"Triggered:"<<(*itr)->ID;
           this->arduinoMSG(WRITE,OFF,(*itr)->chan);
        }else{
            this->fillTable(*(*itr));
            this->arduinoMSG(READ,OFF,(*itr)->chan);
        }//End check if time is up
    }else{
        qDebug()<<"Timer Not Fond";
    }//End look for timer
}//End Timer Event

void Controller::writeDataOut(const tWrap &s)
{
    QFile out(s.path);
    if (!out.open(QIODevice::ReadOnly | QIODevice::Text | QIODevice::ReadWrite |QIODevice::Append))
    {
        qDebug() << "FAIL TO CREATE FILE / FILE NOT EXIT***";
    }else{
        QTextStream stream(&out);
        QDateTime temp=QDateTime::currentDateTime();
        stream <<s.lastRead<<"\t"<<temp.toString("dd/MM/yyyy")<<"\t"<<s.current.toString("hh:mm:ss")<<endl;
        out.close();
    }//End check if file
}//End write data out

bool Controller::getUserResponce(const QString &t,const QString &msg)
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

void Controller::sendUserMessage(const QString &t,const QString &msg)
{
    QMessageBox messageBox;
    messageBox.critical(0,t,msg);
    messageBox.setFixedSize(500,200);
}//End send message

void Controller::on_connectSerial_clicked()
{
    QSerialPortInfo port;
    bool found=false;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        //ui->serialPortList->addItem(info.portName());
        if(info.vendorIdentifier()==this->arduinoVendorID && info.productIdentifier()==this->megaProductID)
        {
            qDebug()<<info.manufacturer();
            qDebug()<<info.vendorIdentifier();
            qDebug()<<info.productIdentifier();
            port=info;
            found=true;
        }//end check for arduino



    }//end iteration of open ports    

    if(found)
    {

        this->arduino->setPortName(port.portName());
        this->arduino->setBaudRate(QSerialPort::Baud9600);
        this->arduino->setDataBits(QSerialPort::Data8);
        this->arduino->setParity(QSerialPort::NoParity);
        this->arduino->setStopBits(QSerialPort::OneStop);
        this->arduino->setFlowControl(QSerialPort::NoFlowControl);
        this->arduino->open(QIODevice::ReadWrite);
        this->arduino->setDataTerminalReady(true);

        connect(arduino, SIGNAL(readyRead()),this,SLOT(readData()));

        this->connected=this->arduino->isWritable() && this->arduino->isReadable();

        if(this->connected==true){
            ui->serialStatus->setText("Connected");
            this->setControlsEnabled(true);
            qDebug()<<"Success!";
        }//
    }else{
        this->sendUserMessage("Error","USB NOT FOUND!!");
    }//end
}//connect serial

void Controller::readData()
{
    if (!this->arduino)
    {
        qDebug()<<"Why am I here!?!";
        return;
    }
        qDebug()<<"Reading";
        QByteArray in;
        int buffSize=this->arduino->bytesAvailable();
        in.resize(buffSize);
        this->arduino->read(in.data(),in.size());
        this->recieved.append(in);
        qDebug()<<this->recieved;
        if(in.contains('\n'))
        {
            QString str = QString::fromUtf8(recieved.split('\n').first());
            int channel=str.split(':').first().toInt();
            std::vector<tWrap*>::iterator itr;
            itr = std::find_if(this->task.begin(),this->task.end(),findChannel(channel));
            if(itr != this->task.end())
            {
                (*itr)->lastRead=str.split(':').last().toDouble();
                this->writeDataOut(*(*itr));
                recieved.clear();
            }else{
                recieved.clear();
                qDebug()<<"Channel not found from read";
            }//End check for channel
        }//End terminator check
}//End serial read data

void Controller::on_unload_B_clicked()
{
    std::vector<tWrap*>::iterator itr;
    itr = std::find_if(this->task.begin(),this->task.end(),findChannel(ui->inChan->value()));
    if(itr!=this->task.end())
    {
        if(!(*itr)->isRunning())
        {
            int channel=ui->inChan->value();
            QString msg="Unload channel "+QString::number(channel)+"?";
            if(this->getUserResponce("Unload?",msg))
            {
             delete (*itr);
             qDebug()<<this->task.size();
             this->task.erase(itr);
             qDebug()<<this->task.size();
             this->clearSlot(channel-1);
            }//end check user response
        }else{
            this->sendUserMessage("Warning","Channel Running. Please Stop First");
        }//End check if slot as running board
    }else{
        this->sendUserMessage("Warning","Channel not found, Please check input");
    }//End check for channel
}//End unload

void Controller::clearTable()
{
    for(int i=0;i<16;i++)
    {
        this->model->setItem(i,0,new QStandardItem("Not Loaded"));
        this->model->setItem(i,1,new QStandardItem(" "));
        this->model->setItem(i,2,new QStandardItem(" "));
        this->model->setItem(i,3,new QStandardItem(" "));
        this->model->setItem(i,4,new QStandardItem(" "));
        this->model->setItem(i,5,new QStandardItem(" "));
    }//End for loop
}//End fill table

void Controller::clearSlot(int s)
{
    this->model->setItem(s,0,new QStandardItem("Not Loaded"));
    this->model->setItem(s,1,new QStandardItem(" "));
    this->model->setItem(s,2,new QStandardItem(" "));
    this->model->setItem(s,3,new QStandardItem(" "));
    this->model->setItem(s,4,new QStandardItem(" "));
    this->model->setItem(s,5,new QStandardItem(" "));
}//End fill table

void Controller::arduinoMSG(comType action,comType param,int c)
{
    QByteArray output;
    QString txt;
    if(action==WRITE)
    {
        if(c<10)
        {
            QTextStream(&txt)<<"P0"<<c<<" "<<param<<"\n";
            output =txt.toLocal8Bit();
            this->arduino->write(output);
            this->arduino->flush();
        }else
        {
            QTextStream(&txt)<<"P"<<c<<" "<<param<<"\n";
            output =txt.toLocal8Bit();
            this->arduino->write(output);
            this->arduino->flush();
        }

    }else if(action==READ){
        if(c<10)
        {
            //Request Current Reading
            QTextStream(&txt)<<"R0"<<c<<"\n";
            output =txt.toLocal8Bit();
            this->arduino->write(output);
            this->arduino->flush();

        }else{
            //Request Current Reading
            QTextStream(&txt)<<"R"<<c<<"\n";
            output =txt.toLocal8Bit();
            this->arduino->write(output);
            this->arduino->flush();
        }//

    }//end check what type
}//End arduinoMSG

void Controller::fillTable(const tWrap &s)
{
    this->model->setItem(s.chan-1,0,new QStandardItem(QString::number(s.chan)));
    this->model->setItem(s.chan-1,1,new QStandardItem(QString(s.start_t.toString("hh:mm:ss"))));
    this->model->setItem(s.chan-1,2,new QStandardItem(QString::number(s.duration)));
    this->model->setItem(s.chan-1,3,new QStandardItem(QString::number(s.elapsed)));
    if(s.running)
    {
        this->model->setItem(s.chan-1,5,new QStandardItem(QString("Running")));
    }else{
        this->model->setItem(s.chan-1,5,new QStandardItem(QString("Not Running")));
    }//End check for running
    this->model->setItem(s.chan-1,4,new QStandardItem(QString::number(s.lastRead)));
}//End fill table

void Controller::setControlsEnabled(bool enable)
{
    ui->startB->setEnabled(enable);
    ui->StopB->setEnabled(enable);
    ui->unload_B->setEnabled(enable);
}//End
