#include "datehelper.h"

QDate DateHelper::currentDate(){return m_currentDate;}

int DateHelper::currentYear(){return m_currentDate.year();}

int DateHelper::currentMonth(){return m_currentDate.month();}

QDate DateHelper::caleDate(){return m_caleDate;}

int DateHelper::caleYear(){return m_caleDate.year();}

int DateHelper::caleMonth(){return m_caleDate.month();}

void DateHelper::setCurrentDate(const QDate& date){
    m_currentDate = date;
}

void DateHelper::setCaleDate(const QDate& date){
    m_caleDate = date;
}

void DateHelper::addMonth(const int diff){
    QDate firstDay = m_currentDate.addDays(1-m_currentDate.day());
    QDate lastDay = firstDay.addMonths(diff).addDays(-1);
    QDate targetDay = m_currentDate.addMonths(diff);
    setCurrentDate(lastDay.day()<m_currentDate.day()?lastDay:targetDay);
}

void DateHelper::addCaleMonth(const int diff){
    QDate firstDay = m_caleDate.addDays(1-m_currentDate.day());
    QDate lastDay = firstDay.addMonths(diff).addDays(-1);
    QDate targetDay = m_caleDate.addMonths(diff);
    setCaleDate(lastDay.day()<m_caleDate.day()?lastDay:targetDay);
}

int DateHelper::calcMonthDiff(const QDate& date){
    const int year = date.year();
    const int yearDiff = year - currentYear();
    return 12*yearDiff + date.month() - currentMonth();
}

int DateHelper::calcCaleMonthDiff(const QDate& date){
    const int year = date.year();
    const int yearDiff = year - caleYear();
    return 12*yearDiff + date.month() - caleMonth();
}

void DateHelper::resetDate(){
    m_currentDate = QDate::currentDate();
    m_caleDate = QDate::currentDate();
}

QDate DateHelper::m_currentDate = QDate::currentDate();
QDate DateHelper::m_caleDate = QDate::currentDate();
