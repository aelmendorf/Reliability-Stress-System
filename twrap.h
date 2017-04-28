#ifndef TWRAP_H
#define TWRAP_H

#include <QMainWindow>
#include <string>
#include <QBasicTimer>
#include <vector>
#include <QObject>
#include <functional>
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <QTextStream>
#include <sys/types.h>
#include <sys/stat.h>

//#define hPath "/home/pi/Monitor/";

class tWrap
{
public:
    QBasicTimer *sTimer;
    int ID,chan;
    double elapsed,lastRead,duration;
    bool loaded,running;
    QString path;
    QDateTime current,start_t;

 //Function declarations
    tWrap(double dur,int c);
    tWrap(const tWrap &t);
    tWrap(int id);
    ~tWrap();

    void addTime();
    void reset();
    void setCurrentT(const QDateTime& t);
    double getElapsed();
    void start(QObject *obj);
    void stop();
    int getIncrement();
    bool isRunning();
    bool isLoaded();
    bool checkTime();
    void buildPath();
    bool writeDataOut();
    const QString& getPath();
private:
    static const int minsToMilli=60000;
    static const int increment=2;
};

struct findTimer
{
    public:
        explicit findTimer(int id) : ID(id) {}
        bool operator() (const tWrap *rhs) const { return ID == rhs->ID; }
    private:
        int ID;
};

struct findChannel
{
    public:
        explicit findChannel(int c) : chan(c) {}
        bool operator() (const tWrap *rhs) const { return chan == rhs->chan; }
    private:
        int chan;
};

#endif // TWRAP_H
