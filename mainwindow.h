#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QGroupBox>
#include <QScrollArea>
#include <QMap>
#include <QDate>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDialog>
#include <QCalendarWidget>
#include <QGridLayout>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QProcessEnvironment>
#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDesktopServices>
#include <QUrl>
#include <QSettings>
#include <QColor>
#include <QPalette>
#include <QCloseEvent>
#include <QGuiApplication>
#include <QScreen>

struct TimeAxisItem
{
    QString type;
    bool isCompleted;
};

struct DateStudyData
{
    int studyHours = 0;
    int completedProjects = 0;
    int totalProjects = 0;
    QMap<int, TimeAxisItem> timeAxisData;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void initSettings();
    void saveSettings();
    void applyTheme(int themeType);
    void setAutoStartup(bool isAuto);

private slots:
    void switchToDayView();
    void switchToMonthView();
    void onTimeAxisBtnClicked(int hour);
    void confirmTimeAxisItem(int hour, QString type);
    void clearCurrentHourItem(int hour);
    void showDateSelectDialog();
    void setToTodayDate();
    void clearCurrentData();
    void switchMonth(int offset);
    void setToCurrentMonth();
    void showSetTargetDialog();
    void setStudyTargetHour(int targetHour);
    void showSettingsWindow();
    void onTrayIconClicked(QSystemTrayIcon::ActivationReason reason);
    void showWindowFromTray();
    void onAutoStartupChanged(int state);
    void onMinToTrayChanged(int state);
    void onThemeChanged(int index);
    void openSavePath();
    void goToMsStoreRate();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void initSavePath();
    void saveDataToFile();
    void loadDataFromFile();
    void initConfigFile();
    void saveConfigToFile();
    void loadConfigFromFile();
    void initUI();
    QWidget* createDayViewPage();
    QWidget* createMonthViewPage();
    QWidget* createTimeAxis();
    void loadDateData(const QDate& date);
    void updateDayViewStats();
    void generateMonthCalendar(int year, int month);
    int calculateContinuousDays();
    void initSystemTray();

    QPushButton *m_dayViewBtn = nullptr;
    QPushButton *m_monthViewBtn = nullptr;
    QPushButton *m_settingsBtn = nullptr;
    QStackedWidget *m_mainStackedWidget = nullptr;
    QLabel *m_todayStudyHourLabel = nullptr;
    QProgressBar *m_dayProgressBar = nullptr;
    QLabel *m_continuousDaysLabel = nullptr;
    QLabel *m_maxContinuousDaysLabel = nullptr;
    QLabel *m_completedProjectsLabel = nullptr;
    QLabel *m_studyCheckLabel = nullptr;
    QWidget *m_timeAxisWidget = nullptr;
    QMap<int, QPushButton*> m_timeAxisBtnMap;
    QLabel *m_monthTitleLabel = nullptr;
    QGridLayout *m_monthCalendarLayout = nullptr;
    QLabel *m_selectedDateLabel = nullptr;
    QLabel *m_targetHourShowLabel = nullptr;

    QDate m_currentDate;
    int m_currentYear;
    int m_currentMonth;
    QString m_saveFilePath;
    QString m_configFilePath;
    QMap<QDate, DateStudyData> m_studyDataMap;
    int m_maxContinuousDays = 0;
    int m_studyTargetHour = 4;

    QSystemTrayIcon *m_systemTrayIcon = nullptr;
    QMenu *m_trayMenu = nullptr;
    bool m_isMinToTray = false;
    bool m_isAutoStartup = false;
    int m_themeType = 0;
    QSettings *m_appSettings = nullptr;
    QString m_appDataPath;
};

#endif
