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
#include "widgets/dayview.h"
#include "widgets/monthview.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void applyTheme(int themeType);
    void showWindowFromTray();

private slots:
    void switchToDayView();
    void switchToMonthView();
    void showSettingsWindow();
    void onTrayIconClicked(QSystemTrayIcon::ActivationReason reason);
    void onAutoStartupChanged(int state);
    void onMinToTrayChanged(int state);
    void onThemeChanged(int index);
    void openSavePath();
    void openLogPath();
    void goToMsStoreRate();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void initUI();
    void initSystemTray();

private:
    QPushButton *m_dayViewBtn = nullptr;
    QPushButton *m_monthViewBtn = nullptr;
    QPushButton *m_settingsBtn = nullptr;
    QStackedWidget *m_mainStackedWidget = nullptr;

    QSystemTrayIcon *m_systemTrayIcon = nullptr;
    QMenu *m_trayMenu = nullptr;

    DayView* m_dayView = nullptr;
    MonthView* m_monthView = nullptr;
};

#endif
