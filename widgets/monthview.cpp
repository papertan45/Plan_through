#include "monthview.h"
#include "./utils/datehelper.h"
#include "./appdatas.h"
#include "./utils/widgetcontainer.h"
#include "dayview.h"

MonthView::MonthView(QWidget *parent)
    : QWidget{parent}
{
    widgetContainer("monthView", this);
    this->setObjectName("monthView");
    QVBoxLayout* pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(10);  // 月视图间距紧凑

    // 月份切换和标题布局
    QHBoxLayout* monthLayout = new QHBoxLayout;
    QPushButton* prevMonthBtn = new QPushButton("◀ 上月");
    QPushButton* nextMonthBtn = new QPushButton("下月 ▶");
    QPushButton* currentMonthBtn = new QPushButton("当月");
    m_monthTitleLabel = new QLabel(QString("%1年%2月").arg(DateHelper::currentYear()).arg(DateHelper::currentMonth()));
    m_monthTitleLabel->setAlignment(Qt::AlignCenter);
    m_monthTitleLabel->setStyleSheet("font-size:15px; font-weight:bold; color:#2D8CF0; padding:0 10px;");
    
    // 按钮样式
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

    // 日历主体
    QGroupBox* calendarGroup = new QGroupBox("📅 月度学习记录");
    calendarGroup->setStyleSheet("QGroupBox{font-size:13px; font-weight:bold; color:#2D8CF0; border:2px solid #ECF5FF; border-radius:8px; padding:8px;}");
    m_monthCalendarLayout = new QGridLayout(calendarGroup);
    m_monthCalendarLayout->setSpacing(4);  // 日历单元格间距极致紧凑
    
    // 星期标题
    QStringList weeks = {"日", "一", "二", "三", "四", "五", "六"};
    for (int i = 0; i < 7; ++i) {
        QLabel* weekLab = new QLabel(weeks[i]);
        weekLab->setStyleSheet("font-size:12px; font-weight:bold; color:#2D8CF0; text-align:center;");
        weekLab->setAlignment(Qt::AlignCenter);
        m_monthCalendarLayout->addWidget(weekLab, 0, i, Qt::AlignCenter);
    }
    calendarGroup->setLayout(m_monthCalendarLayout);
    pageLayout->addWidget(calendarGroup);

    // 连接信号槽
    connect(prevMonthBtn, &QPushButton::clicked, [=](){ switchMonth(-1); });
    connect(nextMonthBtn, &QPushButton::clicked, [=](){ switchMonth(1); });
    connect(currentMonthBtn, &QPushButton::clicked, this, &MonthView::setToCurrentMonth);
}

// 切换月份
// @param offset 月份偏移量，正数为下一个月，负数为上一个月
void MonthView::switchMonth(int offset)
{
    DateHelper::addCaleMonth(offset);
    m_monthTitleLabel->setText(QString("%1年%2月").arg(DateHelper::caleYear()).arg(DateHelper::caleMonth()));
    generateMonthCalendar();
}

// 生成月历
void MonthView::generateMonthCalendar()
{
    const int year = DateHelper::caleYear(), month = DateHelper::caleMonth();
    
    // 清空现有日历项
    QLayoutItem* item;
    while ((item = m_monthCalendarLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    // 清空日期标签映射
    m_dateLabelMap.clear();

    // 重新添加星期标题
    QStringList weeks = {"日", "一", "二", "三", "四", "五", "六"};
    for (int i = 0; i < 7; ++i) {
        QLabel* weekLab = new QLabel(weeks[i]);
        weekLab->setStyleSheet("font-size:12px;font-weight:bold;color:#2D8CF0;text-align:center;");
        weekLab->setAlignment(Qt::AlignCenter);
        m_monthCalendarLayout->addWidget(weekLab, 0, i, Qt::AlignCenter);
    }

    // 获取当月第一天和起始星期
    QDate firstDay(year, month, 1);
    int startWeek = firstDay.dayOfWeek();
    startWeek = (startWeek == 7) ? 0 : startWeek;

    int daysInMonth = firstDay.daysInMonth();
    int row = 1;
    int col = startWeek;

    // 生成日期标签
    for (int day = 1; day <= daysInMonth; ++day) {
        QDate currentDate(year, month, day);
        DateStudyData data = appDatas.value(currentDate);

        QLabel* dayLabel = new QLabel(QString("%1\n%2h").arg(day).arg(data.studyHours));
        dayLabel->setAlignment(Qt::AlignCenter);
        dayLabel->setFixedSize(48, 48);  // 日历单元格尺寸紧凑压缩
        dayLabel->setCursor(Qt::PointingHandCursor); // 设置鼠标指针为手型
        
        // 根据学习时长设置不同的背景色
        if (data.studyHours == 0) {
            dayLabel->setStyleSheet("background-color:#FFFFFF;border:1px solid #F0F0F0;border-radius:8px;font-size:11px;color:#909399;");
        } else if (data.studyHours >= appDatas.targetHour()) {
            dayLabel->setStyleSheet("background-color:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #27AE60,stop:1 #219653);color:white;border-radius:8px;font-size:11px;font-weight:bold;");
        } else {
            dayLabel->setStyleSheet("background-color:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #2D8CF0,stop:1 #1D7AD9);color:white;border-radius:8px;font-size:11px;font-weight:bold;");
        }

        // 为日期标签安装事件过滤器
        dayLabel->installEventFilter(this);
        // 存储日期和标签的映射关系
        m_dateLabelMap[dayLabel] = currentDate;

        m_monthCalendarLayout->addWidget(dayLabel, row, col, Qt::AlignCenter);
        col++;
        if (col >= 7) {
            col = 0;
            row++;
        }
    }
}

// 设置为当前月份
void MonthView::setToCurrentMonth()
{
    DateHelper::resetDate();
    m_monthTitleLabel->setText(QString("%1年%2月").arg(DateHelper::currentYear()).arg(DateHelper::currentMonth()));
    generateMonthCalendar();
}

// 事件过滤器，用于处理日期标签的点击事件
// @param watched 被监视的对象
// @param event 事件对象
// @return 是否处理了该事件
bool MonthView::eventFilter(QObject *watched, QEvent *event)
{
    // 检查事件类型是否为鼠标按下事件
    if (event->type() == QEvent::MouseButtonPress) {
        // 检查被点击的对象是否是QLabel，并且在我们的日期标签映射中
        QLabel *label = qobject_cast<QLabel*>(watched);
        if (label && m_dateLabelMap.contains(label)) {
            // 获取对应的日期
            QDate clickedDate = m_dateLabelMap[label];
            
            // 设置当前日期
            DateHelper::setCurrentDate(clickedDate);
            
            // 直接调用widgetContainer获取主窗口对象，通过QMetaObject::invokeMethod调用switchToDayView
            QObject *mainWindow = widgetContainer("main");
            if (mainWindow) {
                QMetaObject::invokeMethod(mainWindow, "switchToDayView");
            }
            
            // 更新日视图数据
            DayView *dayView = qobject_cast<DayView*>(widgetContainer("dayView"));
            if (dayView) {
                dayView->loadDateData(clickedDate);
                dayView->updateDayViewStats();
            }
            
            return true; // 事件已处理
        }
    }
    
    // 否则，继续传递事件
    return QWidget::eventFilter(watched, event);
}