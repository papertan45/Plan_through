#ifndef MONTHVIEW_H
#define MONTHVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>

/**
 * @brief The MonthView class
 * 月份视窗所对应的QWidget派生类。
 */
class MonthView : public QWidget
{
    Q_OBJECT
public:
    explicit MonthView(QWidget *parent = nullptr);
    void switchMonth(int offset);
    void generateMonthCalendar();
    void setToCurrentMonth();

private:
    QGridLayout *m_monthCalendarLayout = nullptr;
    QLabel* m_monthTitleLabel = nullptr;

signals:
};

#endif // MONTHVIEW_H
