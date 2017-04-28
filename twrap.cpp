#include "twrap.h"

tWrap::tWrap(double dur,int c)
{
    this->running=false;
    this->loaded=true;
    this->chan=c;
    this->duration=dur;
    this->elapsed=0;
    this->lastRead=0;
    this->sTimer=new QBasicTimer();
    this->buildPath();
}//End Constructor

//REIMPLAMENT
/* tWrap(const tWrap &t)
{
    this->duration=t.duration;
    this->elapsed=t.elapsed;
    this->increment=t.increment;
    this->sTimer=new QBasicTimer();
    *this->sTimer=*t.sTimer;
    this->ID=sTimer->timerId();
}*/

tWrap::tWrap(int id)
{
    this->duration=0;
    this->elapsed=0;
    this->ID=id;
    this->sTimer=new QBasicTimer();
}//end constructor for search

tWrap::~tWrap()
{
    this->sTimer->stop();
    delete this->sTimer;
}//end

void tWrap::addTime()
{
    this->elapsed+=(double)this->increment/60.00;
    this->setCurrentT(QDateTime::currentDateTime());
}//end add time

void tWrap::start(QObject *obj)
{
    this->sTimer->start(this->increment*this->minsToMilli,obj);
    this->ID=this->sTimer->timerId();
    this->start_t=QDateTime::currentDateTime();
    this->setCurrentT(this->start_t);
    this->running=true;
}//end timer start

void tWrap::stop()
{
    this->sTimer->stop();
    this->running=false;
    this->setCurrentT(QDateTime::currentDateTime());
}//end stop timer

bool tWrap::isRunning()
{
    return this->sTimer->isActive();
}//

bool tWrap::checkTime()
{
    return this->elapsed>=this->duration;
}//

void tWrap::setCurrentT(const QDateTime& t)
{
    this->current=QDateTime(t);
}//

const QString& tWrap::getPath()
{
    return this->path;
}//

int tWrap::getIncrement()
{
    return this->increment/this->minsToMilli;
}//get increment

double tWrap::getElapsed()
{
    return this->elapsed;
}//End get Elapsed time

void tWrap::buildPath()
{
    QTextStream(&this->path)<<"/home/pi/Monitor"<<"/Channel "<<this->chan<<".txt";
    //stream()
}//End build path

bool tWrap::isLoaded()
{
    return this->loaded;
}//end loaded check

void tWrap::reset()
{
    this->chan=0;
    this->loaded=false;
    this->running=false;
    this->elapsed=0;
    this->duration=0;
    this->ID=0;
    this->path="";
    this->lastRead=0.0;
}//end reset
