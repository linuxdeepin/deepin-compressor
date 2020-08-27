#include "structs.h"

ProgressAssistant::ProgressAssistant(QObject *parent)
    : QObject(parent)
{
}

void ProgressAssistant::startTimer()
{
    m_timer.start();
}

void ProgressAssistant::restartTimer()
{
    m_timer.restart();
}

void ProgressAssistant::resetProgress()
{
    m_totalFileSize = 0;
    m_lastPercent = 0;
    consumeTime = 0;
    m_timer.elapsed();
}

void ProgressAssistant::setTotalSize(qint64 size)
{
    m_totalFileSize = size;
}

qint64 &ProgressAssistant::getTotalSize()
{
    return m_totalFileSize;
}

double ProgressAssistant::getSpeed(unsigned long percent, bool isConvert)
{
    //qDebug() << "size" << m_totalFileSize;
    if (consumeTime < 0) {
        m_timer.start();
    }

    consumeTime += m_timer.elapsed();
    //qDebug() << "compresstime" << consumeTime;
    double speed;
    if (consumeTime == 0) {
        speed = 0; //处理速度
    } else {
        if (isConvert) {
            speed = ((m_totalFileSize * 2 / 1024.0) * (percent / 100.0)) / consumeTime * 1000; //处理速度
        } else {
            speed = ((m_totalFileSize / 1024.0) * (percent / 100.0)) / consumeTime * 1000; //处理速度
        }
    }

    //qDebug() << "------speed:" << speed;
    return speed;
}

qint64 ProgressAssistant::getLeftTime(unsigned long percent, bool isConvert)
{
    //qDebug() << "size" << m_totalFileSize;
    consumeTime += m_timer.elapsed();
    //qDebug() << "compresstime" << consumeTime;
    double speed = 0;

    if (consumeTime != 0) {
        if (isConvert) {
            speed = ((m_totalFileSize * 2 / 1024.0) * (percent / 100.0)) / consumeTime * 1000; //处理速度
        } else {
            speed = ((m_totalFileSize / 1024.0) * (percent / 100.0)) / consumeTime * 1000; //处理速度
        }
    }

    double sizeLeft = 0;
    if (isConvert) {
        sizeLeft = (m_totalFileSize * 2 / 1024.0) * (100 - percent) / 100; //剩余大小
    } else {
        sizeLeft = (m_totalFileSize / 1024.0) * (100 - percent) / 100; //剩余大小
    }

    qint64 timeLeft = 0;
    if (speed != 0) {
        timeLeft = (qint64)(sizeLeft / speed); //剩余时间
    }

    //qDebug() << "m_sizeLeft" << sizeLeft;
    //qDebug() << "m_compressSpeed" << speed;
    //qDebug() << "m_timeLeft" << timeLeft;
    //qDebug() << "timeLeft:" << timeLeft;
    if (timeLeft != 100 && timeLeft == 0) {
        timeLeft = 1;
    }

    return timeLeft;
}
