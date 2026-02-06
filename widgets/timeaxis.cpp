#include "timeaxis.h"
#include "dayview.h"
#include "./datastruct.h"
#include "./appdatas.h"
#include "./utils/datehelper.h"
#include "./utils/widgetcontainer.h"
#include "monthview.h"

TimeAxis::TimeAxis(QWidget *parent)
    : QWidget{parent}
{
    this->setObjectName("timeAxis");
    widgetContainer("timeAxis",this);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(6);   // 时间轴小时项间距紧凑
    layout->setContentsMargins(3, 6, 3, 6);

    QList<int> hours = {8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
    for (int hour : hours) {
        QHBoxLayout* hourLayout = new QHBoxLayout;
        hourLayout->setSpacing(5);

        QLabel* timeLabel = new QLabel(QString("%1:00").arg(hour));
        timeLabel->setMinimumWidth(45);
        timeLabel->setFixedWidth(50);
        timeLabel->setStyleSheet("font-size:12px; font-weight:bold; color:#2D8CF0; text-align:center;");
        timeLabel->setAlignment(Qt::AlignCenter);

        QPushButton* axisBtn = new QPushButton("未安排");
        axisBtn->setObjectName(QString::number(hour));
        axisBtn->setEnabled(true);
        axisBtn->setStyleSheet(
            "QPushButton{font-size:12px; padding:6px 3px; border-radius:10px; border:none; background-color:#FFFFFF; color:#909399;}"
            "QPushButton:hover{background-color:#F8F9FA; color:#606266;}"
            "QPushButton:pressed{background-color:#F0F0F0;}"
            "QPushButton[text!=\"未安排\"]{background-color:#ECF5FF; color:#2D8CF0; font-weight:bold;}");
        axisBtn->setMinimumHeight(30);  // 时间轴按钮高度压缩
        axisBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        m_timeAxisBtnMap.insert(hour, axisBtn);

        hourLayout->addWidget(timeLabel);
        hourLayout->addWidget(axisBtn);
        hourLayout->setStretchFactor(axisBtn, 1);

        layout->addLayout(hourLayout);

        connect(axisBtn, &QPushButton::clicked, [=](){ onTimeAxisBtnClicked(hour); });
    }
}

void TimeAxis::onTimeAxisBtnClicked(int hour)
{
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("选择事项");
    dialog->setModal(true);
    dialog->resize(240, 260);
    dialog->setStyleSheet("QDialog{background-color:#F5F7FA;border-radius:10px;border:none;}"
                          "QLabel{font-size:13px;font-weight:bold;color:#2D8CF0;padding:6px 0;text-align:center;}");

    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(15,15,15,15);
    layout->setSpacing(8);

    QLabel* titleLabel = new QLabel("请选择事项类型");
    layout->addWidget(titleLabel);

    QStringList types = {"学习", "吃饭", "睡觉", "洗澡", "游戏", "杂事"};
    for (const QString& type : types) {
        QPushButton* typeBtn = new QPushButton(type);
        typeBtn->setStyleSheet(
            "QPushButton{font-size:12px;padding:6px 0;border-radius:6px;border:none;background-color:#FFFFFF;color:#333333;}"
            "QPushButton:hover{background-color:#ECF5FF;color:#2D8CF0;}"
            "QPushButton:pressed{background-color:#2D8CF0;color:#FFFFFF;}");
        layout->addWidget(typeBtn);

        connect(typeBtn, &QPushButton::clicked, [=](){
            confirmTimeAxisItem(hour, type);
            dialog->close();
        });
    }

    QHBoxLayout* btnGroupLayout = new QHBoxLayout;
    btnGroupLayout->setSpacing(8);
    QPushButton* clearBtn = new QPushButton("清除");
    QPushButton* cancelBtn = new QPushButton("取消");
    clearBtn->setStyleSheet(
        "QPushButton{font-size:12px;font-weight:bold;padding:5px 0;border-radius:6px;border:none;background-color:#FF6B6B;color:#FFFFFF;width:80px;}"
        "QPushButton:hover{background-color:#FF5252;}"
        "QPushButton:pressed{background-color:#FF3B3B;}");
    cancelBtn->setStyleSheet(
        "QPushButton{font-size:12px;font-weight:bold;padding:5px 0;border-radius:6px;border:none;background-color:#C0C4CC;color:#FFFFFF;width:80px;}"
        "QPushButton:hover{background-color:#909399;}"
        "QPushButton:pressed{background-color:#606266;}");

    btnGroupLayout->addStretch();
    btnGroupLayout->addWidget(clearBtn);
    btnGroupLayout->addWidget(cancelBtn);
    btnGroupLayout->addStretch();
    layout->addLayout(btnGroupLayout);

    connect(clearBtn, &QPushButton::clicked, this, [=](){
        clearCurrentHourItem(hour);
        dialog->close();
    });
    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::close);

    dialog->exec();
}

void TimeAxis::confirmTimeAxisItem(int hour, const QString& type)
{
    bool isCompleted = true;
    DateStudyData& data = appDatas[DateHelper::currentDate()];

    if (data.timeAxisData.contains(hour))
    {
        TimeAxisItem oldItem = data.timeAxisData[hour];
        if (oldItem.type == "学习" && oldItem.isCompleted) data.studyHours -= 1;
        if (oldItem.isCompleted) data.completedProjects -= 1;
    }

    data.timeAxisData[hour] = {type, isCompleted};

    if(m_timeAxisBtnMap.contains(hour)){
        QPushButton* btn = m_timeAxisBtnMap[hour];
        btn->setText(type);
        
        // 根据任务类型设置不同的颜色背景
        QString styleSheet;
        if (type == "学习") {
            styleSheet = "QPushButton{font-size:12px;padding:6px 3px;border-radius:10px;border:none;background-color:#ECF5FF;color:#2D8CF0;font-weight:bold;}"
                         "QPushButton:hover{background-color:#E6F0FF;}"
                         "QPushButton:pressed{background-color:#D9E8FF;}";
        } else if (type == "吃饭") {
            styleSheet = "QPushButton{font-size:12px;padding:6px 3px;border-radius:10px;border:none;background-color:#E8F5E9;color:#2E7D32;font-weight:bold;}"
                         "QPushButton:hover{background-color:#D4EDDA;color:#155724;}"
                         "QPushButton:pressed{background-color:#C3E6CB;color:#155724;}";
        } else if (type == "睡觉") {
            styleSheet = "QPushButton{font-size:12px;padding:6px 3px;border-radius:10px;border:none;background-color:#F3E5F5;color:#6A1B9A;font-weight:bold;}"
                         "QPushButton:hover{background-color:#E1BEE7;color:#4A148C;}"
                         "QPushButton:pressed{background-color:#CE93D8;color:#4A148C;}";
        } else if (type == "洗澡") {
            styleSheet = "QPushButton{font-size:12px;padding:6px 3px;border-radius:10px;border:none;background-color:#E0F7FA;color:#006064;font-weight:bold;}"
                         "QPushButton:hover{background-color:#B2EBF2;color:#004D40;}"
                         "QPushButton:pressed{background-color:#80DEEA;color:#004D40;}";
        } else if (type == "游戏") {
            styleSheet = "QPushButton{font-size:12px;padding:6px 3px;border-radius:10px;border:none;background-color:#FFEBEE;color:#C62828;font-weight:bold;}"
                         "QPushButton:hover{background-color:#FFCDD2;color:#B71C1C;}"
                         "QPushButton:pressed{background-color:#EF9A9A;color:#B71C1C;}";
        } else if (type == "杂事") {
            styleSheet = "QPushButton{font-size:12px;padding:6px 3px;border-radius:10px;border:none;background-color:#FFF8E1;color:#E65100;font-weight:bold;}"
                         "QPushButton:hover{background-color:#FFECB3;color:#E65100;}"
                         "QPushButton:pressed{background-color:#FFE082;color:#E65100;}";
        } else {
            styleSheet = "QPushButton{font-size:12px;padding:6px 3px;border-radius:10px;border:none;background-color:#ECF5FF;color:#2D8CF0;font-weight:bold;}"
                         "QPushButton:hover{background-color:#E6F0FF;}"
                         "QPushButton:pressed{background-color:#D9E8FF;}";
        }
        
        btn->setStyleSheet(styleSheet);
    }

    data.totalProjects = data.timeAxisData.count();
    if (type == "学习" && isCompleted) data.studyHours += 1;
    if (isCompleted) data.completedProjects += 1;

    appDatas.saveDataToFile();
    qobject_cast<DayView*>(widgetContainer("dayView"))->updateDayViewStats();
    qobject_cast<MonthView*>(widgetContainer("monthView"))->generateMonthCalendar();
}

void TimeAxis::clearCurrentHourItem(int hour)
{
    DateStudyData& data = appDatas[DateHelper::currentDate()];
    if (data.timeAxisData.contains(hour))
    {
        TimeAxisItem oldItem = data.timeAxisData[hour];
        if (oldItem.type == "学习" && oldItem.isCompleted) data.studyHours -= 1;
        if (oldItem.isCompleted) data.completedProjects -= 1;
        data.timeAxisData.remove(hour);
        data.totalProjects = data.timeAxisData.count();
    }

    QPushButton* btn = m_timeAxisBtnMap[hour];
    btn->setText("未安排");
    btn->setStyleSheet(
        "QPushButton{font-size:12px;padding:6px 3px;border-radius:10px;border:none;background-color:#FFFFFF;color:#909399;}"
        "QPushButton:hover{background-color:#F8F9FA;color:#606266;}"
        "QPushButton:pressed{background-color:#F0F0F0;}");

    appDatas.saveDataToFile();
    qobject_cast<DayView*>(widgetContainer("dayView"))->updateDayViewStats();
    qobject_cast<MonthView*>(widgetContainer("monthView"))->generateMonthCalendar();
}

QPushButton* TimeAxis::operator[](int hour){
    return m_timeAxisBtnMap[hour];
}
