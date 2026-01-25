#include "datastruct.h"
#include "utils/datehelper.h"
#include "utils/widgetcontainer.h"
#include "widgets/dayview.h"
#include "widgets/monthview.h"
#include "mainwindow.h"
#include "appdatas.h"
#include "clean.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    widgetContainer("main",this);
    initUI();
    applyTheme(appDatas.themeType());
    initSystemTray();
    initMemoryMonitorTimer();

    findChild<DayView*>("dayView")->loadDateData(DateHelper::currentDate());
    findChild<DayView*>("dayView")->updateDayViewStats();
    findChild<MonthView*>("monthView")->generateMonthCalendar();

    if (appDatas.isAutoStartup()) {
        this->hide();
    }
}

// 析构函数，释放所有动态分配的资源
MainWindow::~MainWindow()
{
    if (m_systemTrayIcon) {
        m_systemTrayIcon->hide();
        delete m_systemTrayIcon;
        m_systemTrayIcon = nullptr;
    }
    if (m_trayMenu) {
        delete m_trayMenu;
        m_trayMenu = nullptr;
    }
}

// 应用主题样式
// 参数1：主题类型，0表示默认主题，1表示深色主题等
void MainWindow::applyTheme(int themeType)
{
    appDatas.setTheme(themeType);
    QString mainStyle, progressStyle, btnStyle;
    if(themeType == 0)
    {
        mainStyle = "QMainWindow{background-color: #F5F7FA;border: none;}"
                    "*{color:#333333;}"
                    "QLabel{color:#333333;}"
                    "QGroupBox{color:#333333; font-weight:bold;}";
        progressStyle = "QProgressBar{border:none; border-radius:6px; height:22px; background-color:#ECF5FF; font-size:12px; font-weight:bold; color:#333333;}"
                        "QProgressBar::chunk{background-color:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #2D8CF0,stop:1 #1D7CE0); border-radius:6px;}";
    }
    else if(themeType ==1)
    {
        mainStyle = "QMainWindow{background-color: #FFFFFF;border: none;}"
                    "*{color:#333333;}"
                    "QLabel{color:#333333;}"
                    "QGroupBox{color:#333333; font-weight:bold;}";
        progressStyle = "QProgressBar{border:none; border-radius:6px; height:22px; background-color:#F0F0F0; font-size:12px; font-weight:bold; color:#333333;}"
                        "QProgressBar::chunk{background-color:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #2D8CF0,stop:1 #1D7CE0); border-radius:6px;}";
    }
    this->setStyleSheet(mainStyle);
    findChild<DayView*>("dayView")->setProgressStyle(progressStyle);
    findChild<DayView*>("dayView")->updateDayViewStats();
}

// 初始化系统托盘
void MainWindow::initSystemTray()
{
    m_systemTrayIcon = new QSystemTrayIcon(this);
    m_systemTrayIcon->setIcon(QIcon(":/16.ico"));
    m_systemTrayIcon->setToolTip("学习计划打卡");

    m_trayMenu = new QMenu(this);
    m_trayMenu->setStyleSheet(
        "QMenu{background-color:#FFFFFF; border:1px solid #EEEEEE; border-radius:6px; padding:3px 0px;}"
        "QMenu::item{color:#000000; font-size:12px; padding:4px 30px 4px 15px; margin:1px 3px; border-radius:3px;}"
        "QMenu::item:selected{background-color:#ECF5FF; color:#000000;}"
        "QMenu::separator{height:1px; background-color:#EEEEEE; margin:3px 0px;}"
        );

    QAction *showAct = new QAction("显示窗口", this);
    QAction *exitAct = new QAction("退出程序", this);

    connect(showAct, &QAction::triggered, this, &MainWindow::showWindowFromTray);
    connect(exitAct, &QAction::triggered, qApp, &QApplication::quit);
    m_trayMenu->addAction(showAct);
    m_trayMenu->addAction(exitAct);

    m_systemTrayIcon->setContextMenu(m_trayMenu);
    m_systemTrayIcon->show();
    connect(m_systemTrayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconClicked);
}

// 系统托盘图标点击事件处理
// 参数1：激活原因
void MainWindow::onTrayIconClicked(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::DoubleClick)
    {
        this->showWindowFromTray();
    }
}

// 从系统托盘显示窗口
void MainWindow::showWindowFromTray()
{
    this->show();
    this->activateWindow();
    this->raise();
}

// 切换到日视图
void MainWindow::switchToDayView()
{
    m_mainStackedWidget->setCurrentIndex(0);
    m_dayViewBtn->setChecked(true);
    m_monthViewBtn->setChecked(false);
    findChild<DayView*>("dayView")->updateDayViewStats();
}

// 切换到月视图
void MainWindow::switchToMonthView()
{
    m_mainStackedWidget->setCurrentIndex(1);
    m_monthViewBtn->setChecked(true);
    m_dayViewBtn->setChecked(false);
    findChild<MonthView*>("monthView")->generateMonthCalendar();
}

// 显示设置窗口
void MainWindow::showSettingsWindow()
{
    QDialog *settingsDlg = new QDialog(this);
    settingsDlg->setWindowTitle("软件设置");
    settingsDlg->setFixedSize(500, 380);
    settingsDlg->setModal(true);

    settingsDlg->setStyleSheet(
        "QDialog{background-color:#FFFFFF; border-radius:10px; border:1px solid #EEEEEE;}"
        "QLabel{font-size:12px; color:#000000; font-weight:normal;}"
        "QCheckBox{font-size:12px; color:#000000; padding:3px; background-color:transparent;}"
        "QCheckBox::indicator{width:14px;height:14px;border:1px solid #CCCCCC;border-radius:2px;background-color:#FFFFFF;}"
        "QCheckBox::indicator:checked{background-color:#2D8CF0;border-color:#2D8CF0;}"
        "QComboBox{font-size:12px; color:#000000; height:26px; padding:0 6px; border:1px solid #DDDDDD; border-radius:4px; background-color:#FFFFFF;}"
        "QComboBox::drop-down{border:none;}"
        "QComboBox::down-arrow{width:10px;height:10px;}"
        "QComboBox QAbstractItemView{background-color:#FFFFFF; color:#000000; border:1px solid #DDDDDD; selection-background-color:#ECF5FF; selection-color:#000000;}"
        "QPushButton{font-size:12px; padding:4px 10px; border-radius:4px; border:none; color:#FFFFFF;}"
        "QPushButton:hover{opacity:0.9;}"
        "QPushButton:pressed{opacity:0.8;}"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(settingsDlg);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(15,15,15,15);

    QHBoxLayout *autoStartLayout = new QHBoxLayout;
    QCheckBox *autoStartCb = new QCheckBox("开机自动启动");
    autoStartCb->setChecked(appDatas.isAutoStartup());
    autoStartLayout->addWidget(autoStartCb);
    autoStartLayout->addStretch();
    connect(autoStartCb, &QCheckBox::checkStateChanged, this, &MainWindow::onAutoStartupChanged);

    QHBoxLayout *minTrayLayout = new QHBoxLayout;
    QCheckBox *minTrayCb = new QCheckBox("关闭窗口后最小化到托盘");
    minTrayCb->setChecked(appDatas.isMinToTray());
    minTrayLayout->addWidget(minTrayCb);
    minTrayLayout->addStretch();
    connect(minTrayCb, &QCheckBox::checkStateChanged, this, &MainWindow::onMinToTrayChanged);

    QHBoxLayout *themeLayout = new QHBoxLayout;
    QLabel *themeLab = new QLabel("软件主题：");
    QComboBox *themeCbx = new QComboBox;
    themeCbx->addItems({"简约灰", "纯净白"});
    themeCbx->setCurrentIndex(appDatas.themeType());
    themeLayout->addWidget(themeLab);
    themeLayout->addWidget(themeCbx);
    themeLayout->addStretch();
    connect(themeCbx, &QComboBox::currentIndexChanged, this, &MainWindow::onThemeChanged);

    QHBoxLayout *pathLayout = new QHBoxLayout;
    QPushButton *pathBtn = new QPushButton("打开存档文件位置");
    pathBtn->setStyleSheet("background-color:#2D8CF0;");
    pathLayout->addWidget(pathBtn);
    pathLayout->addStretch();
    connect(pathBtn, &QPushButton::clicked, this, &MainWindow::openSavePath);

    QHBoxLayout *logLayout = new QHBoxLayout;
    QPushButton *logBtn = new QPushButton("打开日志文件位置");
    logBtn->setStyleSheet("background-color:#F59E0B;");
    logLayout->addWidget(logBtn);
    logLayout->addStretch();
    connect(logBtn, &QPushButton::clicked, this, &MainWindow::openLogPath);

    QHBoxLayout *rateLayout = new QHBoxLayout;
    QPushButton *rateBtn = new QPushButton("微软商店好评支持一下吧 ❤️");
    rateBtn->setStyleSheet("background-color:#27AE60;");
    rateLayout->addWidget(rateBtn);
    rateLayout->addStretch();
    connect(rateBtn, &QPushButton::clicked, this, &MainWindow::goToMsStoreRate);

    // 添加内存使用情况显示
    QHBoxLayout *memoryInfoLayout = new QHBoxLayout;
    QLabel *memoryInfoLabel = new QLabel(MemoryCleaner::getMemoryUsage());
    memoryInfoLabel->setStyleSheet("font-size:10px; color:#666666;");
    memoryInfoLayout->addWidget(memoryInfoLabel);
    memoryInfoLayout->addStretch();
    
    // 创建定时器，每秒更新一次内存使用情况
    QTimer *memoryUpdateTimer = new QTimer(settingsDlg);
    connect(memoryUpdateTimer, &QTimer::timeout, [=]() {
        memoryInfoLabel->setText(MemoryCleaner::getMemoryUsage());
    });
    memoryUpdateTimer->start(1000); // 1000毫秒 = 1秒

    // 添加内存清理按钮
    QHBoxLayout *cleanMemoryLayout = new QHBoxLayout;
    
    // 深度清理按钮
    QPushButton *deepCleanMemoryBtn = new QPushButton("清理C盘缓存文件");
    deepCleanMemoryBtn->setStyleSheet("background-color:#DC2626; color:white;");
    cleanMemoryLayout->addWidget(deepCleanMemoryBtn);
    
    cleanMemoryLayout->addStretch();
    
    // 深度清理按钮点击事件
    connect(deepCleanMemoryBtn, &QPushButton::clicked, [=]() {
        // 显示警告对话框，确认是否要强制关闭进程
        QMessageBox msgBox;
        msgBox.setParent(this);
        msgBox.setWindowTitle("警告");
        msgBox.setText("深度清理将强制关闭部分应用程序，可能导致未保存的数据丢失。");
        msgBox.setInformativeText("关闭的应用包括：浏览器、聊天软件、媒体播放器、办公软件等。\n\n是否继续执行深度清理？");
        msgBox.setIcon(QMessageBox::Warning);
        
        // 明确添加按钮
        QPushButton *yesBtn = msgBox.addButton("继续", QMessageBox::YesRole);
        QPushButton *noBtn = msgBox.addButton("取消", QMessageBox::NoRole);
        
        // 设置默认按钮为取消
        msgBox.setDefaultButton(noBtn);
        
        // 执行对话框
        msgBox.exec();
        
        // 检查用户选择
        if (msgBox.clickedButton() == yesBtn) {
            // 执行深度内存清理（强制关闭进程）
            MemoryCleaner::performFastSystemCleaning(true);
            
            // 更新内存使用情况
            QString newMemoryInfo = MemoryCleaner::getMemoryUsage();
            memoryInfoLabel->setText(newMemoryInfo);
            
            // 显示清理完成提示
            QMessageBox::information(this, "提示", "系统内存深度清理完成\n" + newMemoryInfo);
        }
    });

    // 添加自动清理设置
    QHBoxLayout *autoCleanLayout = new QHBoxLayout;
    QLabel *autoCleanLabel = new QLabel("自动清理：");
    QCheckBox *autoCleanCheck = new QCheckBox("启用");
    autoCleanCheck->setChecked(appDatas.isAutoCleanMemoryEnabled());
    
    QLabel *thresholdLabel = new QLabel("阈值：");
    QComboBox *thresholdCombo = new QComboBox;
    
    // 添加阈值选项：30%, 40%, 50%, 60%, 70%, 80%, 90%
    thresholdCombo->addItem("30%", 30);
    thresholdCombo->addItem("40%", 40);
    thresholdCombo->addItem("50%", 50);
    thresholdCombo->addItem("60%", 60);
    thresholdCombo->addItem("70%", 70);
    thresholdCombo->addItem("80%", 80);
    thresholdCombo->addItem("90%", 90);
    
    // 设置当前选中的阈值
    int currentThreshold = appDatas.autoCleanMemoryThreshold();
    int index = thresholdCombo->findData(currentThreshold);
    if (index >= 0) {
        thresholdCombo->setCurrentIndex(index);
    } else {
        thresholdCombo->setCurrentIndex(5); // 默认选中80%
    }
    
    autoCleanLayout->addWidget(autoCleanLabel);
    autoCleanLayout->addWidget(autoCleanCheck);
    autoCleanLayout->addSpacing(10);
    autoCleanLayout->addWidget(thresholdLabel);
    autoCleanLayout->addWidget(thresholdCombo);
    autoCleanLayout->addStretch();
    
    // 连接信号槽
    connect(autoCleanCheck, &QCheckBox::checkStateChanged, this, &MainWindow::onAutoCleanEnabledChanged);
    connect(thresholdCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        int threshold = thresholdCombo->itemData(index).toInt();
        onAutoCleanThresholdChanged(threshold);
    });

    // 数据备份和恢复
    QHBoxLayout *backupLayout = new QHBoxLayout;
    QPushButton *createBackupBtn = new QPushButton("创建数据备份");
    createBackupBtn->setStyleSheet("background-color:#34B7F1;");
    QPushButton *restoreBackupBtn = new QPushButton("从备份恢复");
    restoreBackupBtn->setStyleSheet("background-color:#9370DB;");
    QPushButton *statisticsBtn = new QPushButton("学习统计");
    statisticsBtn->setStyleSheet("background-color:#52C41A;");
    
    backupLayout->addWidget(createBackupBtn);
    backupLayout->addWidget(restoreBackupBtn);
    backupLayout->addWidget(statisticsBtn);
    backupLayout->addStretch();
    
    // 学习统计对话框
    connect(statisticsBtn, &QPushButton::clicked, [=]() {
        QDialog *statsDlg = new QDialog(settingsDlg);
        statsDlg->setWindowTitle("学习统计");
        statsDlg->setFixedSize(500, 400);
        statsDlg->setModal(true);
        
        statsDlg->setStyleSheet(
            "QDialog{background-color:#FFFFFF; border-radius:10px; border:1px solid #EEEEEE;}"
            "QLabel{font-size:12px; color:#333333;}"
            ".statLabel{font-size:14px; font-weight:bold; color:#2D8CF0;}"
            ".statValue{font-size:16px; font-weight:bold; color:#34B7F1;}"
            "QGroupBox{font-size:14px; font-weight:bold; color:#333333; border:1px solid #DDDDDD; border-radius:6px; margin-top:10px; padding-top:15px;}"
            "QGroupBox::title{subcontrol-origin:margin; left:10px; padding:0 5px 0 5px;}"
        );
        
        QVBoxLayout *statsLayout = new QVBoxLayout(statsDlg);
        statsLayout->setSpacing(15);
        statsLayout->setContentsMargins(20, 20, 20, 20);
        
        // 学习时长统计
        QGroupBox *studyHoursGroup = new QGroupBox("学习时长统计");
        QGridLayout *studyHoursLayout = new QGridLayout(studyHoursGroup);
        studyHoursLayout->setSpacing(10);
        studyHoursLayout->setContentsMargins(15, 15, 15, 15);
        
        studyHoursLayout->addWidget(new QLabel("总学习天数："), 0, 0, 1, 1, Qt::AlignRight);
        studyHoursLayout->addWidget(new QLabel(QString::number(appDatas.getTotalStudyDays()) + " 天"), 0, 1, 1, 1, Qt::AlignLeft);
        studyHoursLayout->addWidget(new QLabel("总学习时长："), 1, 0, 1, 1, Qt::AlignRight);
        studyHoursLayout->addWidget(new QLabel(QString::number(appDatas.getTotalStudyHours()) + " 小时"), 1, 1, 1, 1, Qt::AlignLeft);
        studyHoursLayout->addWidget(new QLabel("平均每天学习时长："), 2, 0, 1, 1, Qt::AlignRight);
        studyHoursLayout->addWidget(new QLabel(QString::number(appDatas.getAverageStudyHoursPerDay(), 'f', 1) + " 小时"), 2, 1, 1, 1, Qt::AlignLeft);
        
        // 项目完成情况统计
        QGroupBox *projectsGroup = new QGroupBox("项目完成情况");
        QGridLayout *projectsLayout = new QGridLayout(projectsGroup);
        projectsLayout->setSpacing(10);
        projectsLayout->setContentsMargins(15, 15, 15, 15);
        
        projectsLayout->addWidget(new QLabel("总项目数："), 0, 0, 1, 1, Qt::AlignRight);
        projectsLayout->addWidget(new QLabel(QString::number(appDatas.getTotalProjects()) + " 个"), 0, 1, 1, 1, Qt::AlignLeft);
        projectsLayout->addWidget(new QLabel("完成项目数："), 1, 0, 1, 1, Qt::AlignRight);
        projectsLayout->addWidget(new QLabel(QString::number(appDatas.getCompletedProjects()) + " 个"), 1, 1, 1, 1, Qt::AlignLeft);
        projectsLayout->addWidget(new QLabel("项目完成率："), 2, 0, 1, 1, Qt::AlignRight);
        projectsLayout->addWidget(new QLabel(QString::number(appDatas.getProjectCompletionRate(), 'f', 1) + "%"), 2, 1, 1, 1, Qt::AlignLeft);
        
        // 最大连续天数
        QGroupBox *continuousGroup = new QGroupBox("连续学习");
        QGridLayout *continuousLayout = new QGridLayout(continuousGroup);
        continuousLayout->setSpacing(10);
        continuousLayout->setContentsMargins(15, 15, 15, 15);
        
        continuousLayout->addWidget(new QLabel("最大连续学习天数："), 0, 0, 1, 1, Qt::AlignRight);
        continuousLayout->addWidget(new QLabel(QString::number(appDatas.maxContinDays()) + " 天"), 0, 1, 1, 1, Qt::AlignLeft);
        
        statsLayout->addWidget(studyHoursGroup);
        statsLayout->addWidget(projectsGroup);
        statsLayout->addWidget(continuousGroup);
        
        // 关闭按钮
        QHBoxLayout *closeLayout = new QHBoxLayout;
        QPushButton *closeBtn = new QPushButton("关闭");
        closeBtn->setStyleSheet("background-color:#2D8CF0; color:#FFFFFF; font-size:12px; padding:4px 20px; border-radius:4px; border:none;");
        closeLayout->addStretch();
        closeLayout->addWidget(closeBtn);
        closeLayout->addStretch();
        
        statsLayout->addLayout(closeLayout);
        
        connect(closeBtn, &QPushButton::clicked, statsDlg, &QDialog::close);
        
        statsDlg->exec();
    });
    
    // 连接备份和恢复按钮的信号槽
    connect(createBackupBtn, &QPushButton::clicked, [=]() {
        // 获取当前日期时间作为备份文件名
        QString backupFileName = "study_data_backup_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".json";
        QString backupPath = QFileDialog::getSaveFileName(settingsDlg, "保存数据备份", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + backupFileName, "JSON Files (*.json)");
        
        if (!backupPath.isEmpty()) {
            if (appDatas.createBackup(backupPath)) {
                QMessageBox::information(settingsDlg, "成功", "数据备份创建成功！\n" + backupPath);
            } else {
                QMessageBox::critical(settingsDlg, "失败", "数据备份创建失败！");
            }
        }
    });
    
    connect(restoreBackupBtn, &QPushButton::clicked, [=]() {
        QString backupPath = QFileDialog::getOpenFileName(settingsDlg, "选择数据备份文件", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), "JSON Files (*.json)");
        
        if (!backupPath.isEmpty()) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::warning(settingsDlg, "警告", "从备份恢复将覆盖当前所有数据，是否继续？", 
                                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            
            if (reply == QMessageBox::Yes) {
                if (appDatas.restoreFromBackup(backupPath)) {
                    QMessageBox::information(settingsDlg, "成功", "数据恢复成功！");
                    // 刷新界面数据
                    switchToDayView();
                } else {
                    QMessageBox::critical(settingsDlg, "失败", "数据恢复失败！");
                }
            }
        }
    });
    
    mainLayout->addLayout(autoStartLayout);
    mainLayout->addLayout(minTrayLayout);
    mainLayout->addLayout(themeLayout);
    mainLayout->addLayout(pathLayout);
    mainLayout->addLayout(logLayout);
    mainLayout->addLayout(backupLayout); // 添加备份恢复布局
    mainLayout->addLayout(memoryInfoLayout);
    mainLayout->addLayout(cleanMemoryLayout);
    mainLayout->addLayout(autoCleanLayout);
    mainLayout->addLayout(rateLayout);
    mainLayout->addStretch();

    settingsDlg->exec();
    appDatas.saveSettings();
}

// 自动启动设置改变事件处理
// 参数1：复选框状态
void MainWindow::onAutoStartupChanged(Qt::CheckState state)
{
    appDatas.setAutoStartup(state == Qt::Checked);
}

// 最小化到托盘设置改变事件处理
// 参数1：复选框状态
void MainWindow::onMinToTrayChanged(Qt::CheckState state)
{
    appDatas.setMinToTray(state == Qt::Checked);
}

// 主题设置改变事件处理
// 参数1：主题索引
void MainWindow::onThemeChanged(int index)
{
    applyTheme(index);
}

// 打开存档文件位置
void MainWindow::openSavePath()
{
    QString savePath = appDatas.path("Save");
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(savePath).absolutePath()));
}

// 打开日志文件位置
void MainWindow::openLogPath()
{
    QString logPath = appDatas.path("Log");
    QDesktopServices::openUrl(QUrl::fromLocalFile(logPath));
}

// 跳转到微软商店评分页面
// 参数1：无
void MainWindow::goToMsStoreRate()
{
    QDesktopServices::openUrl(QUrl("https://apps.microsoft.com/detail/9P7X9B7RKXDB?hl=neutral&gl=CN&ocid=pdpshare"));
}

// 初始化内存监控定时器
void MainWindow::initMemoryMonitorTimer()
{
    m_memoryMonitorTimer = new QTimer(this);
    
    // 每隔5分钟检查一次内存使用情况
    connect(m_memoryMonitorTimer, &QTimer::timeout, this, &MainWindow::checkMemoryUsage);
    
    // 启动定时器
    m_memoryMonitorTimer->start(5 * 60 * 1000);
    
    // 立即执行一次检查
    checkMemoryUsage();
}

// 定期检查内存使用情况
void MainWindow::checkMemoryUsage()
{
    // 获取当前内存使用率
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    
    if (GlobalMemoryStatusEx(&memInfo)) {
        int currentUsage = memInfo.dwMemoryLoad;
        int threshold = appDatas.autoCleanMemoryThreshold();
        bool autoCleanEnabled = appDatas.isAutoCleanMemoryEnabled();
        
        // 记录当前内存使用情况
        qDebug() << "定期内存检查：";
        qDebug() << "  - 当前内存使用率：" << currentUsage << "%";
        qDebug() << "  - 自动清理阈值：" << threshold << "%";
        qDebug() << "  - 自动清理状态：" << (autoCleanEnabled ? "已启用" : "已禁用");
        
        // 如果内存使用率超过阈值且自动清理功能已启用，则执行深度清理
        if (autoCleanEnabled && currentUsage >= threshold) {
            qDebug() << "内存使用率超过阈值，执行自动深度清理...";
            
            // 记录清理前的内存状态
            QString beforeCleanup = MemoryCleaner::getMemoryUsage();
            qDebug() << "清理前内存状态：" << beforeCleanup;
            
            // 执行深度内存清理，使用强制关闭进程功能
            bool cleanupSuccess = MemoryCleaner::performFastSystemCleaning(true);
            
            // 记录清理后的内存状态
            QString afterCleanup = MemoryCleaner::getMemoryUsage();
            qDebug() << "清理后内存状态：" << afterCleanup;
            
            qDebug() << "自动内存深度清理" << (cleanupSuccess ? "成功" : "失败");
        }
    } else {
        DWORD errorCode = GetLastError();
        qCritical() << "获取内存使用情况失败，错误码：" << errorCode;
    }
}

// 自动清理内存阈值改变事件处理
// 参数1：新的阈值
void MainWindow::onAutoCleanThresholdChanged(int threshold)
{
    appDatas.setAutoCleanMemoryThreshold(threshold);
}

// 自动清理开关改变事件处理
// 参数1：检查状态
void MainWindow::onAutoCleanEnabledChanged(Qt::CheckState state)
{
    appDatas.setAutoCleanMemoryEnabled(state == Qt::Checked);
}

// 窗口关闭事件处理
// 参数1：关闭事件对象
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (appDatas.isMinToTray()) {
        event->ignore();
        this->hide();
    } else {
        event->accept();
    }
}

// 初始化用户界面
void MainWindow::initUI()
{
    this->resize(500, 600);

    QWidget* centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    QHBoxLayout* topTabLayout = new QHBoxLayout;
    m_dayViewBtn = new QPushButton("日视图");
    m_monthViewBtn = new QPushButton("月视图");
    m_settingsBtn = new QPushButton("设置");
    QString topBtnStyle =
        "QPushButton{font-size:15px; font-weight:bold; padding:8px 25px; margin-right:8px; border-radius:8px; border:none; background-color:#FFFFFF; color:#2D8CF0;}"
        "QPushButton:checked{background-color:#2D8CF0; color:#FFFFFF;}"
        "QPushButton:hover{background-color:#ECF5FF; color:#1D7AD9;}"
        "QPushButton:pressed{background-color:#1D7AD9; color:#FFFFFF;}";
    QString settingBtnStyle = "QPushButton{font-size:12px; padding:6px 12px; border-radius:6px; border:none; background-color:#2D8CF0; color:#FFFFFF; margin-left:8px;}"
                              "QPushButton:hover{background-color:#1D7AD9;}";
    m_dayViewBtn->setStyleSheet(topBtnStyle);
    m_monthViewBtn->setStyleSheet(topBtnStyle);
    m_settingsBtn->setStyleSheet(settingBtnStyle);
    m_dayViewBtn->setCheckable(true);
    m_monthViewBtn->setCheckable(true);
    m_dayViewBtn->setChecked(true);

    topTabLayout->addWidget(m_dayViewBtn);
    topTabLayout->addWidget(m_monthViewBtn);
    topTabLayout->addStretch();
    topTabLayout->addWidget(m_settingsBtn);

    mainLayout->addLayout(topTabLayout);
    connect(m_settingsBtn, &QPushButton::clicked, this, &MainWindow::showSettingsWindow);

    m_mainStackedWidget = new QStackedWidget;
    m_dayView = new DayView(this);
    m_monthView = new MonthView(this);
    m_mainStackedWidget->addWidget(m_dayView);
    m_mainStackedWidget->addWidget(m_monthView);
    mainLayout->addWidget(m_mainStackedWidget);

    connect(m_dayViewBtn, &QPushButton::clicked, this, &MainWindow::switchToDayView);
    connect(m_monthViewBtn, &QPushButton::clicked, this, &MainWindow::switchToMonthView);
}
