#ifndef SERVICE_H
#define SERVICE_H

#include <QString>
#include <Windows.h>

class ServiceManager
{
public:
    static bool installService(const QString& serviceName, const QString& displayName, const QString& executablePath);
    static bool uninstallService(const QString& serviceName);
    static bool startService(const QString& serviceName);
    static bool stopService(const QString& serviceName);
    static bool isServiceInstalled(const QString& serviceName);
    static bool isServiceRunning(const QString& serviceName);
};

#endif // SERVICE_H
