#include "datastruct.h"
#include "utils/datehelper.h"
#include "utils/widgetcontainer.h"
#include "widgets/dayview.h"
#include "widgets/monthview.h"
#include "mainwindow.h"
#include "appdatas.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    widgetContainer("main",this);
    initUI();
    applyTheme(appDatas.themeType());
    initSystemTray();

    findChild<DayView*>("dayView")->loadDateData(DateHelper::currentDate());
    findChild<DayView*>("dayView")->updateDayViewStats();
    findChild<MonthView*>("monthView")->generateMonthCalendar();

    if (appDatas.isAutoStartup()) {
        this->hide();  // 隐藏主窗口
    }
}

MainWindow::~MainWindow()
{
}

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

void MainWindow::onTrayIconClicked(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::DoubleClick) showWindowFromTray();
}

void MainWindow::showWindowFromTray()
{
    this->showNormal();        // 从最小化/隐藏状态恢复正常窗口
    this->raise();             // 窗口置顶
    this->activateWindow();    // 激活窗口获取焦点
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->availableGeometry();
    this->move(screenRect.center() - this->rect().center()); // 窗口居中屏幕
}

void MainWindow::openSavePath()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(appDatas.path()));
}

void MainWindow::goToMsStoreRate()
{
    QDesktopServices::openUrl(QUrl("https://apps.microsoft.com/detail/9P7X9B7RKXDB?hl=neutral&gl=CN&ocid=pdpshare"));
}

void MainWindow::showSettingsWindow()
{
    QDialog *settingsDlg = new QDialog(this);
    settingsDlg->setWindowTitle("软件设置");
    settingsDlg->setFixedSize(380, 280); // 高度从220改为280，容纳主题选项
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
    connect(autoStartCb, &QCheckBox::stateChanged, this, &MainWindow::onAutoStartupChanged);

    QHBoxLayout *minTrayLayout = new QHBoxLayout;
    QCheckBox *minTrayCb = new QCheckBox("关闭窗口后最小化到托盘");
    minTrayCb->setChecked(appDatas.isMinToTray());
    minTrayLayout->addWidget(minTrayCb);
    minTrayLayout->addStretch();
    connect(minTrayCb, &QCheckBox::stateChanged, this, &MainWindow::onMinToTrayChanged);

    // ======================== 新增：主题切换选项 开始 ========================
    QHBoxLayout *themeLayout = new QHBoxLayout;
    QLabel *themeLab = new QLabel("软件主题：");
    QComboBox *themeCbx = new QComboBox;
    themeCbx->addItems({"简约灰（推荐）", "纯净白"});
    themeCbx->setCurrentIndex(appDatas.themeType());
    themeLayout->addWidget(themeLab);
    themeLayout->addWidget(themeCbx);
    themeLayout->addStretch();
    connect(themeCbx, &QComboBox::currentIndexChanged, this, &MainWindow::onThemeChanged);
    mainLayout->addLayout(themeLayout);
    // ======================== 新增：主题切换选项 结束 ========================

    QHBoxLayout *pathLayout = new QHBoxLayout;
    QPushButton *pathBtn = new QPushButton("打开存档文件位置");
    pathBtn->setStyleSheet("background-color:#2D8CF0;");
    pathLayout->addWidget(pathBtn);
    pathLayout->addStretch();
    connect(pathBtn, &QPushButton::clicked, this, &MainWindow::openSavePath);

    QHBoxLayout *rateLayout = new QHBoxLayout;
    QPushButton *rateBtn = new QPushButton("微软商店好评支持一下吧 ❤️");
    rateBtn->setStyleSheet("background-color:#27AE60;");
    rateLayout->addWidget(rateBtn);
    rateLayout->addStretch();
    connect(rateBtn, &QPushButton::clicked, this, &MainWindow::goToMsStoreRate);

    mainLayout->addLayout(autoStartLayout);
    mainLayout->addLayout(minTrayLayout);
    mainLayout->addLayout(pathLayout);
    mainLayout->addLayout(rateLayout);
    mainLayout->addStretch();

    settingsDlg->exec();
    appDatas.saveSettings();
}

void MainWindow::onAutoStartupChanged(int state)
{
    appDatas.setAutoStartup(state == Qt::Checked);
}

void MainWindow::onMinToTrayChanged(int state)
{
    appDatas.setMinToTray(state == Qt::Checked);
}

void MainWindow::onThemeChanged(int index)
{
    applyTheme(index);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(appDatas.isMinToTray())
    {
        this->hide();
        event->ignore();
    }
    else
    {
        event->accept();
    }
}

void MainWindow::initUI()
{
    this->resize(500, 600);  // 主窗口尺寸紧凑缩小

    QWidget* centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(12, 12, 12, 12);  // 全局内边距大幅缩减
    mainLayout->setSpacing(10);                      // 全局主间距紧凑

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

void MainWindow::switchToDayView()
{
    m_mainStackedWidget->setCurrentIndex(0);
    m_dayViewBtn->setChecked(true);
    m_monthViewBtn->setChecked(false);
    findChild<DayView*>("dayView")->updateDayViewStats();
}

void MainWindow::switchToMonthView()
{
    m_mainStackedWidget->setCurrentIndex(1);
    m_monthViewBtn->setChecked(true);
    m_dayViewBtn->setChecked(false);
    findChild<MonthView*>("monthView")->generateMonthCalendar();
}
