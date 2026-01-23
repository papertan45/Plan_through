#ifndef DATEHELPER_H
#define DATEHELPER_H

#include <QDate>

/**
 * @brief The DateHelper class
 * 用于全日期视窗和月份视窗的日期管理。
 * 对于日期视窗，其相关方法会带有current这样的字符串。
 * 对于月份视窗，其相关方法会带有cale这样的字符串。
 */
class DateHelper{
public:
    QDate static currentDate();
    int static currentYear();
    int static currentMonth();
    QDate static caleDate();
    int static caleYear();
    int static caleMonth();

public:
    void static setCurrentDate(const QDate& date);
    void static setCaleDate(const QDate& date);
    void static resetDate();
    void static addMonth(const int diff);
    void static addCaleMonth(const int diff);
    int static calcMonthDiff(const QDate& date);
    int static calcCaleMonthDiff(const QDate& date);

private:
    QDate static m_currentDate;
    QDate static m_caleDate;
};

#endif // DATEHELPER_H
