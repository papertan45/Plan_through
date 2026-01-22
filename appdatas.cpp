#include "appdatas.h"

AppDatas appDatas;

AppDatas::AppDatas() {
    m_appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_appSettings = new QSettings(m_appDataPath + "/app_settings.ini", QSettings::IniFormat);

    initSavePath();
    initConfigFile();
    initSettings();

    loadDataFromFile();
    loadConfigFromFile();
}

AppDatas::~AppDatas(){
    saveDataToFile();
    saveConfigToFile();
    saveSettings();
}

void AppDatas::initSavePath()
{
    QDir dir(m_appDataPath);
    if(!dir.exists())
    {
        dir.mkpath(m_appDataPath);
    }
    m_saveFilePath = m_appDataPath + "/study_data.json";

    QString userName = QProcessEnvironment::systemEnvironment().value("USERNAME");
    qDebug() << "当前登录用户名：" << userName;
    qDebug() << "当前学习数据存档路径：" << m_saveFilePath;
}

void AppDatas::initConfigFile()
{
    QDir dir(m_appDataPath);
    if(!dir.exists())
    {
        dir.mkpath(m_appDataPath);
    }
    m_configFilePath = m_appDataPath + "/study_config.json";
    qDebug() << "当前配置文件存档路径：" << m_configFilePath;
}

void AppDatas::saveConfigToFile()
{
    QJsonObject rootObj;
    rootObj.insert("studyTargetHour", m_studyTargetHour);

    QFile file(m_configFilePath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QJsonDocument doc(rootObj);
    file.write(doc.toJson(QJsonDocument::Compact));
    file.close();
}

void AppDatas::loadConfigFromFile()
{
    QFile file(m_configFilePath);
    if(!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QByteArray data = file.readAll();
    file.close();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if(error.error != QJsonParseError::NoError) return;

    QJsonObject rootObj = doc.object();
    if(rootObj.contains("studyTargetHour"))
    {
        m_studyTargetHour = rootObj["studyTargetHour"].toInt();
        if(m_studyTargetHour <1 || m_studyTargetHour>8) m_studyTargetHour =4;
    }
}

void AppDatas::saveDataToFile()
{
    QJsonObject rootObj;
    rootObj.insert("maxContinuousDays", m_maxContinuousDays);
    QJsonObject dateObj;

    QMap<QDate, DateStudyData>::const_iterator dateIt = m_studyDataMap.constBegin();
    while(dateIt != m_studyDataMap.constEnd())
    {
        QDate date = dateIt.key();
        DateStudyData data = dateIt.value();
        QString dateStr = date.toString("yyyy-MM-dd");

        QJsonObject studyObj;
        studyObj.insert("studyHours", data.studyHours);
        studyObj.insert("completedProjects", data.completedProjects);
        studyObj.insert("totalProjects", data.totalProjects);

        QJsonObject timeAxisObj;
        QMap<int, TimeAxisItem>::const_iterator timeIt = data.timeAxisData.constBegin();
        while(timeIt != data.timeAxisData.constEnd())
        {
            int hour = timeIt.key();
            TimeAxisItem item = timeIt.value();
            QJsonObject itemObj;
            itemObj.insert("type", item.type);
            itemObj.insert("isCompleted", item.isCompleted);
            timeAxisObj.insert(QString::number(hour), itemObj);
            ++timeIt;
        }
        studyObj.insert("timeAxisData", timeAxisObj);
        dateObj.insert(dateStr, studyObj);
        ++dateIt;
    }
    rootObj.insert("studyData", dateObj);

    QJsonDocument doc(rootObj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    if (data.isEmpty()) {
        qWarning() << "数据序列化失败，跳过保存";
        return;
    }

    QString tempFilePath = m_saveFilePath + ".tmp";
    QFile tempFile(tempFilePath);
    if(!tempFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "临时文件打开失败：" << tempFile.errorString();
        return;
    }
    qint64 written = tempFile.write(data);
    tempFile.close();

    if (written != data.size()) {
        qWarning() << "临时文件写入不完整，跳过保存";
        QFile::remove(tempFilePath);
        return;
    }

    QFile::remove(m_saveFilePath);
    if (!QFile::rename(tempFilePath, m_saveFilePath)) {
        qWarning() << "覆盖存档文件失败";
        QFile::remove(tempFilePath);
    }
}

void AppDatas::loadDataFromFile()
{
    QFile file(m_saveFilePath);
    if(!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "存档文件不存在或打开失败：" << file.errorString();
        return;
    }

    QByteArray data = file.readAll();
    file.close();
    if (data.isEmpty()) {
        qWarning() << "存档文件为空，跳过加载";
        return;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if(error.error != QJsonParseError::NoError) {
        qWarning() << "JSON解析失败：" << error.errorString();
        return;
    }

    QJsonObject rootObj = doc.object();
    if(rootObj.contains("maxContinuousDays")) m_maxContinuousDays = rootObj["maxContinuousDays"].toInt();

    if(rootObj.contains("studyData"))
    {
        QJsonObject dateObj = rootObj["studyData"].toObject();
        QStringList dateList = dateObj.keys();
        foreach (QString dateStr, dateList)
        {
            QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
            if(!date.isValid()) continue;

            QJsonObject studyObj = dateObj[dateStr].toObject();
            DateStudyData studyData;
            studyData.studyHours = studyObj["studyHours"].toInt();
            studyData.completedProjects = studyObj["completedProjects"].toInt();
            studyData.totalProjects = studyObj["totalProjects"].toInt();

            QJsonObject timeAxisObj = studyObj["timeAxisData"].toObject();
            QStringList hourList = timeAxisObj.keys();
            foreach (QString hourStr, hourList)
            {
                bool ok;
                int hour = hourStr.toInt(&ok);
                if(!ok) continue;

                QJsonObject itemObj = timeAxisObj[hourStr].toObject();
                TimeAxisItem item;
                item.type = itemObj["type"].toString();
                item.isCompleted = itemObj["isCompleted"].toBool();
                studyData.timeAxisData.insert(hour, item);
            }
            m_studyDataMap.insert(date, studyData);
        }
    }
}

void AppDatas::initSettings()
{
    m_isAutoStartup = m_appSettings->value("auto_startup", false).toBool();
    setAutoStartup(m_isAutoStartup);

    m_isMinToTray = m_appSettings->value("min_to_tray", false).toBool();
    m_themeType = m_appSettings->value("theme", 0).toInt();
}

void AppDatas::saveSettings()
{
    m_appSettings->setValue("auto_startup", m_isAutoStartup);
    m_appSettings->setValue("min_to_tray", m_isMinToTray);
    m_appSettings->setValue("theme", m_themeType);
    m_appSettings->sync();
}

void AppDatas::setAutoStartup(bool isAuto)
{
    m_isAutoStartup = isAuto;
    QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    QString appPath = QApplication::applicationFilePath().replace("/", "\\");
    if(isAuto) reg.setValue("PlanThrough", appPath);
    else reg.remove("PlanThrough");
}

const QString& AppDatas::path(QString type){
    return type=="Root"?m_appDataPath:
               type=="Save"?m_saveFilePath:
               m_configFilePath;
}

int AppDatas::calculateContinuousDays()
{
    int days = 0;
    QDate current = QDate::currentDate();
    while (contains(current) && m_studyDataMap[current].studyHours > 0) {
        days++;
        current = current.addDays(-1);
    }
    return days;
}
