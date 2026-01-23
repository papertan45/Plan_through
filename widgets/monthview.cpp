#include "monthview.h"
#include "./utils/datehelper.h"
#include "./appdatas.h"
#include "./utils/widgetcontainer.h"
#include "dayview.h"

MonthView::MonthView(QWidget *parent)
    : QWidget{parent}
{
    widgetContainer("monthView",this);
    this->setObjectName("monthView");
    QVBoxLayout* pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0,0,0,0);
    pageLayout->setSpacing(10);  // 月视图间距紧凑

    QHBoxLayout* monthLayout = new QHBoxLayout;
    QPushButton* prevMonthBtn = new QPushButton("◀ 上月");
    QPushButton* nextMonthBtn = new QPushButton("下月 ▶");
    QPushButton* currentMonthBtn = new QPushButton("当月");
    m_monthTitleLabel = new QLabel(QString("%1年%2月").arg(DateHelper::currentYear()).arg(DateHelper::currentMonth()));
    m_monthTitleLabel->setAlignment(Qt::AlignCenter);
    m_monthTitleLabel->setStyleSheet("font-size:15px; font-weight:bold; color:#2D8CF0; padding:0 10px;");
    QString monthBtnStyle =
        "QPushButton{font-size:12px; font-weight:bold; padding:5px 10px; border-radius:6px; border:none; background-color:#FFFFFF; color:#333333;}"
        "QPushButton:hover{background-color:#F0F0F0;}"
        "QPushButton:pressed{background-color:#E0E0E0;}";
    prevMonthBtn->setStyleSheet(monthBtnStyle);
    nextMonthBtn->setStyleSheet(monthBtnStyle);
    currentMonthBtn->setStyleSheet("QPushButton{font-size:12px; font-weight:bold; padding:5px 10px; border-radius:6px; border:none; background-color:#2D8CF0; color:#FFFFFF;}"
                                   "QPushButton:hover{background-color:#1D7AD9;}");
    monthLayout->addWidget(prevMonthBtn);
    monthLayout->addWidget(m_monthTitleLabel);
    monthLayout->addWidget(nextMonthBtn);
    monthLayout->addStretch();
    monthLayout->addWidget(currentMonthBtn);
    pageLayout->addLayout(monthLayout);

    QGroupBox* calendarGroup = new QGroupBox("📅 月度学习记录");
    calendarGroup->setStyleSheet("QGroupBox{font-size:13px; font-weight:bold; color:#2D8CF0; border:2px solid #ECF5FF; border-radius:8px; padding:8px;}");
    m_monthCalendarLayout = new QGridLayout(calendarGroup);
    m_monthCalendarLayout->setSpacing(4);  // 日历单元格间距极致紧凑
    QStringList weeks = {"日", "一", "二", "三", "四", "五", "六"};
    for (int i = 0; i < 7; ++i) {
        QLabel* weekLab = new QLabel(weeks[i]);
        weekLab->setStyleSheet("font-size:12px; font-weight:bold; color:#2D8CF0; text-align:center;");
        weekLab->setAlignment(Qt::AlignCenter);
        m_monthCalendarLayout->addWidget(weekLab, 0, i, Qt::AlignCenter);
    }
    calendarGroup->setLayout(m_monthCalendarLayout);
    pageLayout->addWidget(calendarGroup);

    connect(prevMonthBtn, &QPushButton::clicked, [=](){ switchMonth(-1);/*qobject_cast<DayView*>(widgetContainer("dayView"))->updateDayViewStats();*/ });
    connect(nextMonthBtn, &QPushButton::clicked, [=](){ switchMonth(1);/*qobject_cast<DayView*>(widgetContainer("dayView"))->updateDayViewStats();*/ });
    connect(currentMonthBtn, &QPushButton::clicked, this, &MonthView::setToCurrentMonth);
}

void MonthView::switchMonth(int offset)
{
    DateHelper::addCaleMonth(offset);
    m_monthTitleLabel->setText(QString("%1年%2月").arg(DateHelper::caleYear()).arg(DateHelper::caleMonth()));
    generateMonthCalendar();
}

void MonthView::generateMonthCalendar(){
    const int year = DateHelper::caleYear(),month = DateHelper::caleMonth();
    QLayoutItem* item;
    while ((item = m_monthCalendarLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    QStringList weeks = {"日", "一", "二", "三", "四", "五", "六"};
    for (int i = 0; i < 7; ++i) {
        QLabel* weekLab = new QLabel(weeks[i]);
        weekLab->setStyleSheet("font-size:12px;font-weight:bold;color:#2D8CF0;text-align:center;");
        weekLab->setAlignment(Qt::AlignCenter);
        m_monthCalendarLayout->addWidget(weekLab, 0, i, Qt::AlignCenter);
    }

    QDate firstDay(year, month, 1);
    int startWeek = firstDay.dayOfWeek();
    startWeek = (startWeek == 7) ? 0 : startWeek;

    int daysInMonth = firstDay.daysInMonth();
    int row = 1;
    int col = startWeek;

    for (int day = 1; day <= daysInMonth; ++day) {
        QDate currentDate(year, month, day);
        DateStudyData data = appDatas.value(currentDate);

        QLabel* dayLabel = new QLabel(QString("%1\n%2h").arg(day).arg(data.studyHours));
        dayLabel->setAlignment(Qt::AlignCenter);
        dayLabel->setFixedSize(48, 48);  // 日历单元格尺寸紧凑压缩
        if (data.studyHours == 0) {
            dayLabel->setStyleSheet("background-color:#FFFFFF;border:1px solid #F0F0F0;border-radius:8px;font-size:11px;color:#909399;");
        } else if (data.studyHours >= appDatas.targetHour()) {
            dayLabel->setStyleSheet("background-color:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #27AE60,stop:1 #219653);color:white;border-radius:8px;font-size:11px;font-weight:bold;");
        } else {
            dayLabel->setStyleSheet("background-color:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #2D8CF0,stop:1 #1D7AD9);color:white;border-radius:8px;font-size:11px;font-weight:bold;");
        }

        m_monthCalendarLayout->addWidget(dayLabel, row, col, Qt::AlignCenter);
        col++;
        if (col >= 7) {
            col = 0;
            row++;
        }
    }
}

void MonthView::setToCurrentMonth()
{
    DateHelper::resetDate();
    m_monthTitleLabel->setText(QString("%1年%2月").arg(DateHelper::currentYear()).arg(DateHelper::currentMonth()));
    generateMonthCalendar();
}
