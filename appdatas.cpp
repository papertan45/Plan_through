#include "appdatas.h"

AppDatas appDatas;

// 构造函数，初始化应用数据管理
AppDatas::AppDatas() {
    m_appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/Plan_through";
    m_appSettings = new QSettings(m_appDataPath + "/app_settings.ini", QSettings::IniFormat);

    initSavePath();
    initConfigFile();
    initSettings();

    cleanupOldLogs();

    loadDataFromFile();
    loadConfigFromFile();
}

// 析构函数，释放资源并保存数据
AppDatas::~AppDatas(){
    saveDataToFile();
    saveConfigToFile();
    saveSettings();
    
    if (m_appSettings) {
        delete m_appSettings;
        m_appSettings = nullptr;
    }
}

// 初始化存档路径
void AppDatas::initSavePath()
{
    QDir dir(m_appDataPath);
    if(!dir.exists())
    {
        dir.mkpath(m_appDataPath);
    }
    m_saveFilePath = m_appDataPath + "/study_data.json";
    m_logDirectory = m_appDataPath + "/logs";
    
    QDir logDir(m_logDirectory);
    if(!logDir.exists())
    {
        logDir.mkpath(m_logDirectory);
    }

    QString userName = QProcessEnvironment::systemEnvironment().value("USERNAME");
    qDebug() << "当前登录用户名：" << userName;
    qDebug() << "当前学习数据存档路径：" << m_saveFilePath;
    qDebug() << "当前日志目录：" << m_logDirectory;
}

// 初始化配置文件
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

// 保存配置到文件
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

// 从文件加载配置
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

// 保存数据到文件
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

    if (QFile::exists(m_saveFilePath)) {
        QString backupFilePath = m_saveFilePath + ".bak";
        QFile::remove(backupFilePath);
        if (!QFile::rename(m_saveFilePath, backupFilePath)) {
            qWarning() << "创建备份文件失败";
            QFile::remove(tempFilePath);
            return;
        }
    }
    
    if (!QFile::rename(tempFilePath, m_saveFilePath)) {
        qWarning() << "覆盖存档文件失败";
        QFile::remove(tempFilePath);
        if (QFile::exists(m_saveFilePath + ".bak")) {
            QFile::rename(m_saveFilePath + ".bak", m_saveFilePath);
        }
        return;
    }
    
    QFile::remove(m_saveFilePath + ".bak");
    
    // 保存数据后写入日志
    saveLog();
}

// 保存每日日志
void AppDatas::saveLog()
{
    QString logFileName = QDate::currentDate().toString("yyyy-MM-dd") + ".json";
    QString logFilePath = m_logDirectory + "/" + logFileName;
    
    QJsonObject rootObj;
    rootObj.insert("maxContinuousDays", m_maxContinuousDays);
    QJsonObject dateObj;
    
    QDate currentDate = QDate::currentDate();
    if (m_studyDataMap.contains(currentDate)) {
        const DateStudyData& data = m_studyDataMap[currentDate];
        QString dateStr = currentDate.toString("yyyy-MM-dd");
        
        QJsonObject studyObj;
        studyObj.insert("studyHours", data.studyHours);
        studyObj.insert("completedProjects", data.completedProjects);
        studyObj.insert("totalProjects", data.totalProjects);
        
        QJsonObject timeAxisObj;
        QMap<int, TimeAxisItem>::const_iterator timeIt = data.timeAxisData.constBegin();
        while(timeIt != data.timeAxisData.constEnd())
        {
            int hour = timeIt.key();
            const TimeAxisItem& item = timeIt.value();
            QJsonObject itemObj;
            itemObj.insert("type", item.type);
            itemObj.insert("isCompleted", item.isCompleted);
            timeAxisObj.insert(QString::number(hour), itemObj);
            ++timeIt;
        }
        studyObj.insert("timeAxisData", timeAxisObj);
        dateObj.insert(dateStr, studyObj);
    }
    
    rootObj.insert("studyData", dateObj);
    
    QJsonDocument doc(rootObj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    
    QFile logFile(logFilePath);
    if(!logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "日志文件打开失败：" << logFile.errorString();
        return;
    }
    
    logFile.write(data);
    logFile.close();
    qDebug() << "日志保存成功：" << logFilePath;
    
    // 清理旧日志
    cleanupOldLogs();
}

// 清理旧日志
void AppDatas::cleanupOldLogs()
{
    QDir logDir(m_logDirectory);
    QFileInfoList logFiles = logDir.entryInfoList(QStringList() << "*.json", QDir::Files);
    
    QDate currentDate = QDate::currentDate();
    int daysToKeep = 30;
    
    foreach (const QFileInfo& fileInfo, logFiles) {
        QString fileName = fileInfo.fileName();
        QString dateStr = fileName.left(10);
        QDate logDate = QDate::fromString(dateStr, "yyyy-MM-dd");
        
        if (logDate.isValid()) {
            int daysDiff = logDate.daysTo(currentDate);
            if (daysDiff > daysToKeep) {
                QString logFilePath = fileInfo.absoluteFilePath();
                if (QFile::remove(logFilePath)) {
                    qDebug() << "删除旧日志文件：" << logFilePath;
                } else {
                    qWarning() << "删除旧日志文件失败：" << logFilePath;
                }
            }
        }
    }
}

// 从日志读取数据
// 返回：是否成功
bool AppDatas::loadDataFromLogs()
{
    QDir logDir(m_logDirectory);
    QFileInfoList logFiles = logDir.entryInfoList(QStringList() << "*.json", QDir::Files);
    
    if (logFiles.isEmpty()) {
        qDebug() << "没有找到日志文件";
        return false;
    }
    
    bool loaded = false;
    int maxContinuous = 0;
    
    foreach (const QFileInfo& fileInfo, logFiles) {
        QString logFilePath = fileInfo.absoluteFilePath();
        QFile logFile(logFilePath);
        
        if(!logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "日志文件打开失败：" << logFile.errorString();
            continue;
        }
        
        QByteArray data = logFile.readAll();
        logFile.close();
        
        if (data.isEmpty()) {
            qWarning() << "日志文件为空：" << logFilePath;
            continue;
        }
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        if(error.error != QJsonParseError::NoError) {
            qWarning() << "日志JSON解析失败：" << error.errorString();
            continue;
        }
        
        QJsonObject rootObj = doc.object();
        if(rootObj.contains("maxContinuousDays")) {
            int logMaxContinuous = rootObj["maxContinuousDays"].toInt();
            if (logMaxContinuous > maxContinuous) {
                maxContinuous = logMaxContinuous;
            }
        }
        
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
                loaded = true;
            }
        }
    }
    
    if (loaded) {
        m_maxContinuousDays = maxContinuous;
        qDebug() << "从日志文件加载数据成功";
    }
    
    return loaded;
}

// 从文件加载数据
void AppDatas::loadDataFromFile()
{
    QFile file(m_saveFilePath);
    if(file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray data = file.readAll();
        file.close();
        
        if (!data.isEmpty()) {
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(data, &error);
            if(error.error == QJsonParseError::NoError) {
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
                return;
            } else {
                qWarning() << "存档JSON解析失败：" << error.errorString();
            }
        } else {
            qWarning() << "存档文件为空";
        }
    } else {
        qWarning() << "存档文件不存在或打开失败：" << file.errorString();
    }
    
    // 存档文件读取失败，尝试从日志读取
    if (!loadDataFromLogs()) {
        // 日志读取失败，按照老办法创建（即保持m_studyDataMap为空，后续会自动创建）
        qDebug() << "日志读取失败，将按照老办法创建数据";
    }
}

// 初始化设置
void AppDatas::initSettings()
{
    m_isAutoStartup = m_appSettings->value("auto_startup", false).toBool();
    setAutoStartup(m_isAutoStartup);

    m_isMinToTray = m_appSettings->value("min_to_tray", false).toBool();
    m_themeType = m_appSettings->value("theme", 0).toInt();
    
    // 加载自动清理内存设置
    m_isAutoCleanMemoryEnabled = m_appSettings->value("auto_clean_memory_enabled", true).toBool();
    m_autoCleanMemoryThreshold = m_appSettings->value("auto_clean_memory_threshold", 80).toInt();
}

// 保存设置
void AppDatas::saveSettings()
{
    m_appSettings->setValue("auto_startup", m_isAutoStartup);
    m_appSettings->setValue("min_to_tray", m_isMinToTray);
    m_appSettings->setValue("theme", m_themeType);
    
    // 保存自动清理内存设置
    m_appSettings->setValue("auto_clean_memory_enabled", m_isAutoCleanMemoryEnabled);
    m_appSettings->setValue("auto_clean_memory_threshold", m_autoCleanMemoryThreshold);
    
    m_appSettings->sync();
}

// 设置是否自动启动
// 参数1：是否自动启动
void AppDatas::setAutoStartup(bool isAuto)
{
    m_isAutoStartup = isAuto;
    QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    QString appPath = QApplication::applicationFilePath().replace("/", "\\");
    if(isAuto) reg.setValue("PlanThrough", appPath);
    else reg.remove("PlanThrough");
}

// 获取指定类型的路径
// 参数1：路径类型，支持"Root"、"Save"、"Config"、"Log"
// 返回：路径字符串
const QString& AppDatas::path(QString type){
    return type=="Root"?m_appDataPath:
               type=="Save"?m_saveFilePath:
               type=="Config"?m_configFilePath:
               type=="Log"?m_logDirectory:
               m_appDataPath;
}

// 计算连续学习天数
// 返回：连续学习天数
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
