#ifndef APPDATAS_H
#define APPDATAS_H

#include "datastruct.h"
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QString>
#include <QProcessEnvironment>
#include <QSettings>
#include <QApplication>
#include <QStandardPaths>

/**
 * @brief The AppDatas class
 * 负责用户数据读取与存储。
 */
class AppDatas
{
public:
    AppDatas();
    ~AppDatas();

    void initSavePath();
    void initConfigFile();
    void initSettings();

    void loadConfigFromFile();
    void loadDataFromFile();

    void saveConfigToFile();
    void saveDataToFile();
    void saveSettings();

public:
    void setAutoStartup(bool isAuto);
    void setMinToTray(bool isMinToTray){m_isMinToTray = isMinToTray;}
    void setTheme(int themeType){m_themeType = themeType;}
    void setTargetHour(int targetHour){m_studyTargetHour = targetHour;}
    void setMaxContinDays(int continDays){m_maxContinuousDays = continDays;}
    DateStudyData& operator[](const QDate& key){return m_studyDataMap[key];}

public:
    const QString& path(QString type = "Root");
    bool isAutoStartup(){return m_isAutoStartup;}
    bool isMinToTray(){return m_isMinToTray;}
    int themeType(){return m_themeType;}
    int targetHour(){return m_studyTargetHour;}
    int maxContinDays(){return m_maxContinuousDays;}
    DateStudyData value(const QDate& key){return m_studyDataMap.value(key);}
    bool contains(const QDate& key){return m_studyDataMap.contains(key);}
    int calculateContinuousDays();

private:
    QString m_appDataPath;
    QString m_saveFilePath;
    QString m_configFilePath;

    QMap<QDate, DateStudyData> m_studyDataMap;
    int m_studyTargetHour = 4;
    int m_maxContinuousDays = 0;

    QSettings *m_appSettings;
    bool m_isAutoStartup = false;
    bool m_isMinToTray = false;
    int m_themeType = 0;
};

extern AppDatas appDatas;

#endif // APPDATAS_H
