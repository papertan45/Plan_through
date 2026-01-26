#include "dayview.h"
#include "monthview.h"
#include "./datastruct.h"
#include "./appdatas.h"
#include "./utils/datehelper.h"
#include "./utils/widgetcontainer.h"

DayView::DayView(QWidget *parent)
    : QWidget{parent}
{
    widgetContainer("dayView",this);
    this->setObjectName("dayView");
    QVBoxLayout* pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(10);  // 日视图间距紧凑

    QHBoxLayout* funcBtnLayout = new QHBoxLayout;
    QPushButton* dateSelectBtn = new QPushButton("日期选择");
    QPushButton* todayBtn = new QPushButton("今日");
    QPushButton* setTargetBtn = new QPushButton("设置目标");
    QPushButton* clearBtn = new QPushButton("清除当日");
    m_targetHourShowLabel = new QLabel();
    m_targetHourShowLabel->setStyleSheet("font-size:12px; font-weight:bold; color:#27AE60; padding:0 6px;");
    m_targetHourShowLabel->setText(QString("每日目标：%1 小时").arg(appDatas.targetHour()));

    QString funcBtnStyle =
        "QPushButton{font-size:12px; font-weight:bold; padding:5px 12px; border-radius:6px; border:none; background-color:#FFFFFF; color:#333333;}"
        "QPushButton:hover{background-color:#F8F9FA;}"
        "QPushButton:pressed{background-color:#E9ECEF;}";
    clearBtn->setStyleSheet("QPushButton{font-size:12px; font-weight:bold; padding:5px 12px; border-radius:6px; border:none; background-color:#FF6B6B; color:#FFFFFF;}"
                            "QPushButton:hover{background-color:#FF5252;}"
                            "QPushButton:pressed{background-color:#FF3B3B;}");
    setTargetBtn->setStyleSheet("QPushButton{font-size:12px; font-weight:bold; padding:5px 12px; border-radius:6px; border:none; background-color:#27AE60; color:#FFFFFF;}"
                                "QPushButton:hover{background-color:#219653;}"
                                "QPushButton:pressed{background-color:#1E8845;}");

    funcBtnLayout->addWidget(dateSelectBtn);
    funcBtnLayout->addWidget(todayBtn);
    funcBtnLayout->addStretch();
    funcBtnLayout->addWidget(m_targetHourShowLabel);
    funcBtnLayout->addWidget(setTargetBtn);
    funcBtnLayout->addWidget(clearBtn);
    pageLayout->addLayout(funcBtnLayout);

    QGroupBox* progressGroup = new QGroupBox;
    progressGroup->setStyleSheet("QGroupBox{font-size:13px; font-weight:bold; color:#2D8CF0; border:2px solid #ECF5FF; border-radius:8px; padding:8px; margin:0;}");
    QVBoxLayout* progressLayout = new QVBoxLayout(progressGroup);

    QHBoxLayout* progressHeaderLayout = new QHBoxLayout;
    QLabel* progressTitle = new QLabel("📚 学习进度");
    progressTitle->setStyleSheet("font-size:13px; font-weight:bold; color:#2D8CF0;");
    m_selectedDateLabel = new QLabel();
    m_selectedDateLabel->setStyleSheet("font-size:12px; font-weight:bold; color:#2D8CF0; padding:0 3px;");
    m_selectedDateLabel->setText(QString("当前日期：%1").arg(DateHelper::currentDate().toString("yyyy年MM月dd日")));
    progressHeaderLayout->addWidget(progressTitle);
    progressHeaderLayout->addStretch();
    progressHeaderLayout->addWidget(m_selectedDateLabel);

    m_todayStudyHourLabel = new QLabel(QString("今日学习：0小时 / 目标%1小时").arg(appDatas[DateHelper::currentDate()].studyHours));
    m_todayStudyHourLabel->setStyleSheet("font-size:12px; font-weight:bold; color:#333333; padding:4px 0;");

    m_dayProgressBar = new QProgressBar;
    m_dayProgressBar->setAlignment(Qt::AlignCenter);
    m_dayProgressBar->setRange(0, appDatas.targetHour());
    m_dayProgressBar->setValue(0);

    progressLayout->addLayout(progressHeaderLayout);
    progressLayout->addWidget(m_todayStudyHourLabel);
    progressLayout->addWidget(m_dayProgressBar);
    pageLayout->addWidget(progressGroup);

    QGroupBox* statsGroup = new QGroupBox("📊 打卡统计");
    statsGroup->setStyleSheet("QGroupBox{font-size:13px; font-weight:bold; color:#2D8CF0; border:2px solid #ECF5FF; border-radius:8px; padding:8px;}");
    QGridLayout* statsLayout = new QGridLayout(statsGroup);
    statsLayout->setSpacing(10);  // 统计项间距紧凑
    m_continuousDaysLabel = new QLabel("当前连续天数：0");
    m_maxContinuousDaysLabel = new QLabel("最长连续天数：0");
    m_completedProjectsLabel = new QLabel("已完成项目：0/0");
    m_studyCheckLabel = new QLabel(QString("学习打卡：0/%1").arg(0));
    QString statLabelStyle = "font-size:12px; font-weight:bold; color:#555555; padding:2px;";
    m_continuousDaysLabel->setStyleSheet(statLabelStyle);
    m_maxContinuousDaysLabel->setStyleSheet(statLabelStyle);
    m_completedProjectsLabel->setStyleSheet(statLabelStyle);
    m_studyCheckLabel->setStyleSheet(statLabelStyle);
    statsLayout->addWidget(m_continuousDaysLabel, 0, 0);
    statsLayout->addWidget(m_maxContinuousDaysLabel, 0, 1);
    statsLayout->addWidget(m_completedProjectsLabel, 1, 0);
    statsLayout->addWidget(m_studyCheckLabel, 1, 1);
    pageLayout->addWidget(statsGroup);

    QScrollArea* timeAxisScroll = new QScrollArea(this);
    timeAxisScroll->setWidgetResizable(true);
    timeAxisScroll->setStyleSheet("QScrollArea{border:none; background-color:transparent;}"
                                  "QScrollBar:vertical{width:6px; background-color:#F5F7FA; border-radius:3px;}"
                                  "QScrollBar::handle:vertical{background-color:#C0C4CC; border-radius:3px;}"
                                  "QScrollBar::handle:vertical:hover{background-color:#909399;}");
    m_timeAxisWidget = new TimeAxis(this);
    timeAxisScroll->setWidget(m_timeAxisWidget);
    pageLayout->addWidget(timeAxisScroll);

    connect(dateSelectBtn, &QPushButton::clicked, this, &DayView::showDateSelectDialog);
    connect(todayBtn, &QPushButton::clicked, this, &DayView::setToTodayDate);
    connect(setTargetBtn, &QPushButton::clicked, this, &DayView::showSetTargetDialog);
    connect(clearBtn, &QPushButton::clicked, this, &DayView::clearCurrentData);
}

DayView::~DayView(){

}

void DayView::updateDayViewStats()
{
    DateStudyData data = appDatas[DateHelper::currentDate()];
    int continuousDays = appDatas.calculateContinuousDays();
    appDatas.setMaxContinDays(qMax(appDatas.maxContinDays(), continuousDays));
    m_todayStudyHourLabel->setText(QString("今日学习：%1小时 / 目标%2小时").arg(data.studyHours).arg(appDatas.targetHour()));
    if(data.studyHours >= appDatas.targetHour())
    {
        m_dayProgressBar->setValue(appDatas.targetHour());
    }
    else
    {
        m_dayProgressBar->setValue(data.studyHours);
    }

    m_continuousDaysLabel->setText(QString("当前连续天数：%1").arg(continuousDays));
    m_maxContinuousDaysLabel->setText(QString("最长连续天数：%1").arg(appDatas.maxContinDays()));
    m_completedProjectsLabel->setText(QString("已完成项目：%1/%2").arg(data.completedProjects).arg(data.totalProjects));
    m_studyCheckLabel->setText(QString("学习打卡：%1/%2").arg(data.studyHours).arg(appDatas.targetHour()));
}

void DayView::showDateSelectDialog()
{
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("选择日期");
    dialog->setModal(true);
    dialog->resize(260, 200);  // 弹窗尺寸紧凑
    dialog->setStyleSheet("QDialog{background-color:#F5F7FA;border-radius:10px;border:none;}"
                          "QCalendarWidget{background-color:#FFFFFF;border-radius:6px;border:1px solid #ECF5FF;}"
                          "QPushButton{font-size:12px;font-weight:bold;padding:5px 15px;border-radius:6px;border:none;background-color:#2D8CF0;color:#FFFFFF;margin-top:8px;}"
                          "QPushButton:hover{background-color:#1D7AD9;}");
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(15,15,15,15);
    layout->setSpacing(8);
    QCalendarWidget* calendar = new QCalendarWidget;
    calendar->setSelectedDate(DateHelper::currentDate());
    calendar->setStyleSheet("QCalendarWidget{font-size:10px;}");
    layout->addWidget(calendar);

    QPushButton* confirmBtn = new QPushButton("确定");
    layout->addWidget(confirmBtn, 0, Qt::AlignCenter);
    connect(confirmBtn, &QPushButton::clicked, [=](){
        const QDate date = calendar->selectedDate();
        DateHelper::setCurrentDate(date);
        m_selectedDateLabel->setText(QString("当前日期：%1").arg(date.toString("yyyy年MM月dd日")));
        loadDateData(DateHelper::currentDate());
        updateDayViewStats();
        qobject_cast<MonthView*>(widgetContainer("monthView"))->switchMonth(DateHelper::calcCaleMonthDiff(date));
        dialog->close();
    });

    dialog->exec();
}

void DayView::setToTodayDate()
{
    DateHelper::resetDate();
    m_selectedDateLabel->setText(QString("当前日期：%1").arg(DateHelper::currentDate().toString("yyyy年MM月dd日")));
    loadDateData(DateHelper::currentDate());
    updateDayViewStats();
    qobject_cast<MonthView*>(widgetContainer("monthView"))->switchMonth(0);
}

void DayView::showSetTargetDialog()
{
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("设置每日学习目标");
    dialog->setModal(true);
    dialog->resize(240, 280);  // 弹窗尺寸紧凑
    dialog->setStyleSheet("QDialog{background-color:#F5F7FA;border-radius:10px;border:none;}"
                          "QLabel{font-size:13px;font-weight:bold;color:#27AE60;padding:6px 0;text-align:center;}");

    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(15,15,15,15);
    layout->setSpacing(8);

    QLabel* titleLabel = new QLabel("请选择每日学习小时数");
    layout->addWidget(titleLabel);

    QList<int> targetHours = {1,2,3,4,5,6,7,8};
    for (int hour : targetHours) {
        QPushButton* hourBtn = new QPushButton(QString("%1 小时").arg(hour));
        hourBtn->setStyleSheet(
            "QPushButton{font-size:12px;padding:6px 0;border-radius:6px;border:none;background-color:#FFFFFF;color:#333333;}"
            "QPushButton:hover{background-color:#F0F9F0;color:#27AE60;}"
            "QPushButton:pressed{background-color:#27AE60;color:#FFFFFF;font-weight:bold;}");
        layout->addWidget(hourBtn);

        connect(hourBtn, &QPushButton::clicked, [=](){
            appDatas.setTargetHour(hour);
            m_targetHourShowLabel->setText(QString("每日目标：%1 小时").arg(appDatas.targetHour()));
            m_todayStudyHourLabel->setText(QString("今日学习：%1小时 / 目标%2小时").arg(appDatas[DateHelper::currentDate()].studyHours).arg(appDatas.targetHour()));
            m_dayProgressBar->setRange(0,appDatas.targetHour());
            if(appDatas[DateHelper::currentDate()].studyHours >= appDatas.targetHour())
                m_dayProgressBar->setValue(appDatas.targetHour());
            else
                m_dayProgressBar->setValue(appDatas[DateHelper::currentDate()].studyHours);
            m_dayProgressBar->update();
            dialog->close();
        });
    }

    QPushButton* cancelBtn = new QPushButton("取消");
    cancelBtn->setStyleSheet(
        "QPushButton{font-size:12px;font-weight:bold;padding:5px 0;border-radius:6px;border:none;background-color:#C0C4CC;color:#FFFFFF;width:80px;margin-top:3px;}"
        "QPushButton:hover{background-color:#909399;}"
        "QPushButton:pressed{background-color:#606266;}");
    QHBoxLayout* btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::close);

    dialog->exec();
}

void DayView::clearCurrentData()
{
    appDatas[DateHelper::currentDate()] = DateStudyData();
    QList<int> hours = {8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
    for(int hour : hours)
    {
        QPushButton* btn = m_timeAxisWidget->operator[](hour);
        btn->setText("未安排");
        btn->setStyleSheet(
            "QPushButton{font-size:12px;padding:6px 3px;border-radius:10px;border:none;background-color:#FFFFFF;color:#909399;}"
            "QPushButton:hover{background-color:#F8F9FA;color:#606266;}"
            "QPushButton:pressed{background-color:#F0F0F0;}");
    }
    appDatas.saveDataToFile();
    loadDateData(DateHelper::currentDate());
    updateDayViewStats();
    qobject_cast<MonthView*>(widgetContainer("monthView"))->switchMonth(0);
    QMessageBox::information(this, "提示", "当日数据已清除！");
}

void DayView::loadDateData(const QDate& date)
{
    if (!appDatas.contains(date)) {
        appDatas[date] = DateStudyData();
    }
    DateStudyData data = appDatas[date];

    QList<int> hours = {8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
    for(int hour : hours)
    {
        QPushButton* btn = m_timeAxisWidget->operator[](hour);
        if(data.timeAxisData.contains(hour))
        {
            TimeAxisItem item = data.timeAxisData[hour];
            btn->setText(item.type);
            btn->setStyleSheet(
                "QPushButton{font-size:12px;padding:6px 3px;border-radius:10px;border:none;background-color:#ECF5FF;color:#2D8CF0;font-weight:bold;}"
                "QPushButton:hover{background-color:#E6F0FF;}"
                "QPushButton:pressed{background-color:#D9E8FF;}");
        }
        else
        {
            btn->setText("未安排");
            btn->setStyleSheet(
                "QPushButton{font-size:12px;padding:6px 3px;border-radius:10px;border:none;background-color:#FFFFFF;color:#909399;}"
                "QPushButton:hover{background-color:#F8F9FA;color:#606266;}"
                "QPushButton:pressed{background-color:#F0F0F0;}");
        }
    }
}

void DayView::setProgress(int hour){
    m_dayProgressBar->setValue(hour);
}

void DayView::setProgressStyle(QString style){
    m_dayProgressBar->setStyleSheet(style);
}
