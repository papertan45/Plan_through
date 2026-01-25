#include "clean.h"
#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QDebug>

// 执行快速系统内存清理（简化版本）
bool MemoryCleaner::performFastSystemCleaning(bool forceCloseProcesses) {
    if (forceCloseProcesses) {
        // 执行强制关闭进程操作
        forceCloseUnnecessaryProcesses();
    }
    
    // 清理临时文件
    cleanTempFiles();
    
    // 1. 检查管理员权限
    bool isAdmin = isRunningAsAdmin();
    
    // 2. 提升必要的权限
    bool privilegesEnabled = EnableBasicPrivileges();
    
    // 3. 记录清理前的内存状态
    MEMORYSTATUSEX preCleanupMemInfo;
    preCleanupMemInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&preCleanupMemInfo);
    
    // 获取清理前的详细内存信息
    qulonglong preTotalPhys = preCleanupMemInfo.ullTotalPhys;
    qulonglong preAvailPhys = preCleanupMemInfo.ullAvailPhys;
    qulonglong preUsedPhys = preTotalPhys - preAvailPhys;
    
    // 4. 执行内存清理操作
    bool currentProcessCleaned = false;
    bool systemMemoryCleaned = false;
    int cleanupSteps = 0;
    
    // 记录开始清理时间
    qDebug() << "\n========== 开始执行内存清理 ==========";
    
    // 步骤1: 清空当前进程工作集
    qDebug() << "执行步骤1: 清空当前进程工作集";
    BOOL emptyWSResult = EmptyWorkingSet(GetCurrentProcess());
    currentProcessCleaned = emptyWSResult != 0;
    if (currentProcessCleaned) {
        cleanupSteps++;
        qDebug() << "✓ 成功清空当前进程工作集";
    } else {
        qWarning() << "✗ 无法清空当前进程工作集，错误码：" << GetLastError();
    }
    
    // 步骤2: 调整当前进程工作集大小
    qDebug() << "执行步骤2: 调整当前进程工作集大小";
    BOOL setWSResult = SetProcessWorkingSetSize(GetCurrentProcess(), (SIZE_T)-1, (SIZE_T)-1);
    if (setWSResult) {
        cleanupSteps++;
        qDebug() << "✓ 成功调整当前进程工作集大小";
    } else {
        qWarning() << "✗ 无法调整当前进程工作集大小，错误码：" << GetLastError();
    }
    
    // 尝试使用Windows API清理系统内存（如果权限足够）
    if (isAdmin && privilegesEnabled) {
        qDebug() << "\n========== 开始系统级内存清理 ==========";
        
        // 步骤3: 清理系统文件缓存（方式1）
        qDebug() << "执行步骤3: 清理系统文件缓存（方式1）";
        BOOL cacheResult1 = SetSystemFileCacheSize(0, 0, FILE_CACHE_MIN_HARD_ENABLE);
        if (cacheResult1) {
            systemMemoryCleaned = true;
            cleanupSteps++;
            qDebug() << "✓ 成功清理系统文件缓存（方式1）";
        } else {
            qWarning() << "✗ 无法清理系统文件缓存（方式1），错误码：" << GetLastError();
        }
        
        // 步骤4: 调整系统文件缓存大小（方式2）
        qDebug() << "执行步骤4: 调整系统文件缓存大小（方式2）";
        BOOL cacheResult2 = SetSystemFileCacheSize(64 * 1024 * 1024, -1, FILE_CACHE_MIN_HARD_ENABLE | FILE_CACHE_MAX_HARD_ENABLE);
        if (cacheResult2) {
            systemMemoryCleaned = true;
            cleanupSteps++;
            qDebug() << "✓ 成功调整系统文件缓存大小（方式2）";
        } else {
            qWarning() << "✗ 无法调整系统文件缓存大小（方式2），错误码：" << GetLastError();
        }
        
        // 步骤5: 尝试清理系统备用列表（Standby List）- 这是专业内存清理软件释放大量内存的关键
        qDebug() << "执行步骤5: 清理系统备用列表（Standby List）";
        bool standbyResult = CleanMemoryByCommand(MEMORY_CLEAN_COMMAND_EMPTY_STANDBY_LIST);
        if (standbyResult) {
            systemMemoryCleaned = true;
            cleanupSteps++;
            qDebug() << "✓ 成功清理系统备用列表";
        }
        
        // 步骤6: 尝试清理系统工作集
        qDebug() << "执行步骤6: 清理系统工作集";
        bool workingSetsResult = CleanMemoryByCommand(MEMORY_CLEAN_COMMAND_EMPTY_WORKING_SETS);
        if (workingSetsResult) {
            systemMemoryCleaned = true;
            cleanupSteps++;
            qDebug() << "✓ 成功清理系统工作集";
        }
        
        // 步骤7: 尝试清理系统修改列表（Modified List）
        qDebug() << "执行步骤7: 清理系统修改列表（Modified List）";
        bool modifiedListResult = CleanMemoryByCommand(L"EmptyModifiedList");
        if (modifiedListResult) {
            systemMemoryCleaned = true;
            cleanupSteps++;
            qDebug() << "✓ 成功清理系统修改列表";
        }
        
        qDebug() << "========== 系统级内存清理结束 ==========";
    } else {
        qDebug() << "⚠ 未执行系统级内存清理：缺少管理员权限或权限提升失败";
    }
    
    // 5. 记录清理后的内存状态
    MEMORYSTATUSEX postCleanupMemInfo;
    postCleanupMemInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&postCleanupMemInfo);
    
    // 获取清理后的详细内存信息
    qulonglong postTotalPhys = postCleanupMemInfo.ullTotalPhys;
    qulonglong postAvailPhys = postCleanupMemInfo.ullAvailPhys;
    qulonglong postUsedPhys = postTotalPhys - postAvailPhys;
    
    // 6. 计算内存变化
    qulonglong freedMem = preUsedPhys > postUsedPhys ? preUsedPhys - postUsedPhys : 0;
    double freedMemGB = static_cast<double>(freedMem) / (1024 * 1024 * 1024);
    
    // 7. 记录清理结果
    qDebug() << "\n========== 内存清理结果汇总 ==========";
    qDebug() << "清理操作执行情况：";
    qDebug() << "  - 当前进程是否为管理员：" << (isAdmin ? "✓ 是" : "✗ 否");
    qDebug() << "  - 权限提升状态：" << (privilegesEnabled ? "✓ 成功" : "✗ 失败");
    qDebug() << "  - 成功执行的清理步骤数：" << cleanupSteps;
    qDebug() << "  - 当前进程工作集清理：" << (currentProcessCleaned ? "✓ 成功" : "✗ 失败");
    qDebug() << "  - 系统内存清理：" << (systemMemoryCleaned ? "✓ 成功" : "✗ 失败");
    
    qDebug() << "\n内存变化详情：";
    qDebug() << "  - 总物理内存：" << static_cast<double>(preTotalPhys) / (1024 * 1024 * 1024) << " GB";
    qDebug() << "  - 清理前已使用内存：" << static_cast<double>(preUsedPhys) / (1024 * 1024 * 1024) << " GB";
    qDebug() << "  - 清理后已使用内存：" << static_cast<double>(postUsedPhys) / (1024 * 1024 * 1024) << " GB";
    qDebug() << "  - 清理前可用内存：" << static_cast<double>(preAvailPhys) / (1024 * 1024 * 1024) << " GB";
    qDebug() << "  - 清理后可用内存：" << static_cast<double>(postAvailPhys) / (1024 * 1024 * 1024) << " GB";
    qDebug() << "  - 估计释放内存：" << freedMemGB << " GB";
    qDebug() << "  - 清理前内存使用率：" << preCleanupMemInfo.dwMemoryLoad << "%";
    qDebug() << "  - 清理后内存使用率：" << postCleanupMemInfo.dwMemoryLoad << "%";
    
    // 8. 向用户解释内存清理的效果和局限性
    qDebug() << "\n========== 内存清理说明 ==========";
    if (freedMemGB > 0.1) {
        qDebug() << "✓ 内存清理成功，释放了" << freedMemGB << "GB内存";
    } else if (freedMemGB > 0) {
        qDebug() << "⚠ 内存清理成功，但只释放了" << freedMemGB << "GB内存，效果不明显";
    } else {
        qDebug() << "⚠ 内存清理后内存使用量没有明显变化";
    }
    
    qDebug() << "\n内存清理的局限性：";
    qDebug() << "1. 内存清理主要影响当前进程和系统缓存，不会强制关闭其他进程";
    qDebug() << "2. 系统可能会立即重新分配释放的内存，导致内存使用变化不明显";
    qDebug() << "3. 某些系统服务和进程会占用固定内存，无法通过常规方法释放";
    qDebug() << "4. 长期运行的系统可能需要重启才能彻底释放内存";
    qDebug() << "5. 清理系统修改列表（Modified List）可能导致未保存的数据丢失";
    
    qDebug() << "========== 内存清理结束 ==========\n";
    
    // 返回整体结果
    return currentProcessCleaned || systemMemoryCleaned;
}

// 获取简洁的系统内存使用情况
QString MemoryCleaner::getMemoryUsage() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    
    if (!GlobalMemoryStatusEx(&memInfo)) {
        DWORD errorCode = GetLastError();
        qCritical() << "获取内存信息失败，错误码：" << errorCode;
        return "获取内存信息失败";
    }
    
    qulonglong totalPhysMem = memInfo.ullTotalPhys;
    qulonglong usedPhysMem = totalPhysMem - memInfo.ullAvailPhys;
    
    // 转换为GB
    double totalGB = static_cast<double>(totalPhysMem) / (1024 * 1024 * 1024);
    double usedGB = static_cast<double>(usedPhysMem) / (1024 * 1024 * 1024);
    
    return QString("物理内存: %1 GB / %2 GB (%3%)")
        .arg(usedGB, 0, 'f', 2)
        .arg(totalGB, 0, 'f', 2)
        .arg(memInfo.dwMemoryLoad);
}

// 检查是否以管理员权限运行
bool MemoryCleaner::isRunningAsAdmin() {
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    PSID adminSid = nullptr;
    
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, 
                                 DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminSid)) {
        if (!CheckTokenMembership(NULL, adminSid, &isAdmin)) {
            DWORD errorCode = GetLastError();
            qWarning() << "检查管理员权限失败，错误码：" << errorCode;
            isAdmin = FALSE;
        }
        FreeSid(adminSid);
    } else {
        DWORD errorCode = GetLastError();
        qWarning() << "分配管理员SID失败，错误码：" << errorCode;
    }
    
    qDebug() << "应用程序以管理员权限运行：" << (isAdmin != FALSE);
    
    return isAdmin != FALSE;
}

// 提升基本必要的权限
bool MemoryCleaner::EnableBasicPrivileges() {
    // 获取RtlAdjustPrivilege函数指针
    HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
    if (!hNtdll) {
        DWORD errorCode = GetLastError();
        qCritical() << "无法获取ntdll.dll模块句柄，错误码：" << errorCode;
        return false;
    }
    
    // 正确的函数指针转换
    FARPROC proc = GetProcAddress(hNtdll, "RtlAdjustPrivilege");
    if (!proc) {
        DWORD errorCode = GetLastError();
        qCritical() << "无法获取RtlAdjustPrivilege函数地址，错误码：" << errorCode;
        return false;
    }
    
    BOOLEAN enabled;
    
    // 使用正确的类型转换，匹配64位系统的函数指针类型
    typedef LONG (__stdcall* RtlAdjustPrivilegeFn)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
    RtlAdjustPrivilegeFn RtlAdjustPrivilege = (RtlAdjustPrivilegeFn)proc;
    
    // 权限列表，尝试获取多种必要的权限
    ULONG privileges[] = {
        19,  // SeCreateGlobalPrivilege
        20,  // SeProfileSingleProcessPrivilege
        21,  // SeSystemtimePrivilege
        22,  // SeShutdownPrivilege
        23,  // SeRemoteShutdownPrivilege
        24,  // SeDebugPrivilege
        25   // SeAuditPrivilege
    };
    
    bool anyPrivilegeEnabled = false;
    
    // 尝试获取每一种权限
    for (int i = 0; i < sizeof(privileges) / sizeof(ULONG); i++) {
        LONG status = RtlAdjustPrivilege(
            privileges[i],
            TRUE,
            FALSE,
            &enabled
        );
        
        if (status == 0 && enabled) {
            anyPrivilegeEnabled = true;
            qDebug() << "成功启用权限ID：" << privileges[i];
        } else {
            // 只记录警告，不影响整体结果
            qWarning() << "启用权限ID" << privileges[i] << "失败，NTSTATUS：" << status;
        }
    }
    
    return anyPrivilegeEnabled;
}

// 通过命令清理内存
bool MemoryCleaner::CleanMemoryByCommand(const wchar_t* command) {
    // 获取NtSetSystemInformation函数指针
    HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
    if (!hNtdll) {
        DWORD errorCode = GetLastError();
        qCritical() << "无法获取ntdll.dll模块句柄，错误码：" << errorCode;
        return false;
    }
    
    // 正确的函数指针转换
    FARPROC proc = GetProcAddress(hNtdll, "NtSetSystemInformation");
    if (!proc) {
        DWORD errorCode = GetLastError();
        qCritical() << "无法获取NtSetSystemInformation函数地址，错误码：" << errorCode;
        return false;
    }
    
    // 使用正确的类型转换，匹配64位系统的函数指针类型
    typedef LONG (__stdcall* NtSetSystemInformationFn)(ULONG, PVOID, ULONG);
    NtSetSystemInformationFn NtSetSystemInformation = (NtSetSystemInformationFn)proc;
    
    // 构建内存清理命令
    SYSTEM_MEMORY_LIST_COMMAND memCommand;
    ZeroMemory(&memCommand, sizeof(memCommand));
    memCommand.Command = command;
    memCommand.ProcessHandle = nullptr; // NULL表示系统级操作
    memCommand.Flags = 0;
    
    // 记录清理命令
    qDebug() << "执行内存清理命令：" << QString::fromWCharArray(command);
    
    // 调用NtSetSystemInformation执行清理
    LONG status = NtSetSystemInformation(
        SystemMemoryListInformation,
        &memCommand,
        sizeof(memCommand)
    );
    
    // NTSTATUS成功返回0
    if (status != 0) {
        // 详细解释常见的NTSTATUS错误码
        QString errorMsg;
        switch (status) {
            case -1073741792: // STATUS_ACCESS_DENIED
                errorMsg = "访问被拒绝，可能缺少必要的权限";
                break;
            case -1073741823: // STATUS_INVALID_PARAMETER
                errorMsg = "无效的参数，可能命令或结构不正确";
                break;
            case -1073741630: // STATUS_NOT_SUPPORTED
                errorMsg = "此命令在当前系统上不支持";
                break;
            default:
                errorMsg = QString("未知错误，NTSTATUS：%1").arg(status);
                break;
        }
        qWarning() << "内存清理命令执行失败，命令：" << QString::fromWCharArray(command) << "，错误：" << errorMsg;
    } else {
        qDebug() << "内存清理命令执行成功：" << QString::fromWCharArray(command);
    }
    
    return status == 0;
}

// 清理系统文件缓存
bool MemoryCleaner::CleanSystemFileCache() {
    // 使用SetSystemFileCacheSize清理系统文件缓存
    // 设置合理的缓存大小，释放更多内存
    return SetSystemFileCacheSize(64 * 1024 * 1024, -1, FILE_CACHE_MIN_HARD_ENABLE) != 0;
}

// 清理临时文件
void MemoryCleaner::cleanTempFiles() {
    qDebug() << "\n========== 开始清理临时文件 ==========";
    
    // 定义要清理的临时文件夹路径
    QStringList tempFolders = {
        QDir::tempPath(),  // 用户临时文件夹
        "C:/Windows/Temp", // 系统临时文件夹
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation), // 应用缓存文件夹
        QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) // 通用缓存文件夹
    };
    
    qint64 totalCleanedSize = 0;
    int totalCleanedFiles = 0;
    
    foreach (const QString& folderPath, tempFolders) {
        QDir dir(folderPath);
        if (!dir.exists()) {
            qDebug() << "跳过不存在的文件夹：" << folderPath;
            continue;
        }
        
        qDebug() << "清理文件夹：" << folderPath;
        
        // 获取文件夹中的所有文件和子文件夹
        QFileInfoList fileInfos = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot);
        
        foreach (const QFileInfo& fileInfo, fileInfos) {
            if (fileInfo.isFile()) {
                // 删除文件
                QFile file(fileInfo.absoluteFilePath());
                qint64 fileSize = file.size();
                if (file.remove()) {
                    totalCleanedSize += fileSize;
                    totalCleanedFiles++;
                } else {
                    // 忽略无法删除的文件（可能正在使用）
                }
            } else if (fileInfo.isDir()) {
                // 递归清理子文件夹
                QDir subDir(fileInfo.absoluteFilePath());
                subDir.removeRecursively();
            }
        }
    }
    
    // 转换为更易读的单位
    double cleanedSizeMB = static_cast<double>(totalCleanedSize) / (1024 * 1024);
    double cleanedSizeGB = cleanedSizeMB / 1024;
    
    qDebug() << "\n临时文件清理结果：";
    qDebug() << "  - 清理的文件数量：" << totalCleanedFiles;
    if (cleanedSizeGB > 1) {
        qDebug() << "  - 清理的空间大小：" << QString::number(cleanedSizeGB, 'f', 2) << "GB";
    } else {
        qDebug() << "  - 清理的空间大小：" << QString::number(cleanedSizeMB, 'f', 2) << "MB";
    }
    qDebug() << "========== 临时文件清理结束 ==========\n";
}

// 强制关闭不必要的进程
void MemoryCleaner::forceCloseUnnecessaryProcesses() {
    qDebug() << "\n========== 开始强制关闭不必要进程 ==========";
    
    // 定义可以安全关闭的进程列表（根据常见的资源密集型应用和非系统进程）
    QStringList safeToCloseProcesses = {
        // 浏览器
        "chrome.exe",       // Google Chrome
        "firefox.exe",      // Mozilla Firefox
        "msedge.exe",       // Microsoft Edge
        "opera.exe",        // Opera
        "brave.exe",        // Brave Browser
        "vivaldi.exe",      // Vivaldi
        "chrome.exe",       // Google Chrome
        "chromium.exe",     // Chromium
        "iexplore.exe",     // Internet Explorer
        
        // 聊天和通讯软件
        "discord.exe",      // Discord
        "slack.exe",        // Slack
        "zoom.exe",         // Zoom
        "teams.exe",        // Microsoft Teams
        "skype.exe",        // Skype
        "qq.exe",           // QQ
        "tim.exe",          // TIM
        "wechat.exe",       // WeChat
        "telegram.exe",     // Telegram
        "whatsapp.exe",     // WhatsApp
        
        // 媒体播放器
        "spotify.exe",      // Spotify
        "vlc.exe",          // VLC Media Player
        "wmplayer.exe",     // Windows Media Player
        "mpc-hc64.exe",     // MPC-HC
        "potplayer.exe",    // PotPlayer
        "foobar2000.exe",    // Foobar2000
        
        // 游戏和游戏平台
        "steam.exe",        // Steam
        "epicgameslauncher.exe",  // Epic Games Launcher
        "origin.exe",       // Origin
        "battle.net.exe",   // Battle.net
        "uplay.exe",        // Ubisoft Connect
        "launcher.exe",      // 各种游戏启动器
        "riotclientux.exe", // Riot Games 客户端
        "valorant.exe",     // Valorant
        "leagueclient.exe", // League of Legends
        
        // 办公软件
        "winword.exe",      // Microsoft Word
        "excel.exe",        // Microsoft Excel
        "powerpnt.exe",     // Microsoft PowerPoint
        "outlook.exe",      // Microsoft Outlook
        "onenote.exe",      // Microsoft OneNote
        "acrord32.exe",     // Adobe Acrobat Reader
        "acrord64.exe",     // Adobe Acrobat Reader (64位)
        
        // 设计和编辑软件
        "photoshop.exe",    // Adobe Photoshop
        "illustrator.exe",  // Adobe Illustrator
        "premiere.exe",     // Adobe Premiere
        "aftereffects.exe", // Adobe After Effects
        "indesign.exe",     // Adobe InDesign
        "lightroom.exe",    // Adobe Lightroom
        "coreldrw.exe",     // CorelDRAW
        
        // 其他资源密集型应用
        "dropbox.exe",      // Dropbox
        "googlebackupandsync.exe", // Google Backup and Sync
        "megasync.exe",     // MEGA Sync
        "utorrent.exe",     // uTorrent
        "bittorrent.exe",   // BitTorrent
        "qbittorrent.exe",  // qBittorrent
        "transmission.exe", // Transmission
        "nvidia geforce experience.exe", // NVIDIA GeForce Experience
        "geforce experience.exe", // NVIDIA GeForce Experience
        "msi afterburner.exe", // MSI Afterburner
        "cpu-z.exe",        // CPU-Z
        "gpu-z.exe",        // GPU-Z
        "hwinfo64.exe",     // HWInfo64
        "speedfan.exe"       // SpeedFan
    };
    
    // 获取当前进程ID，避免关闭自己
    DWORD currentProcessId = GetCurrentProcessId();
    qDebug() << "当前进程ID：" << currentProcessId;
    
    // 遍历所有进程
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        qCritical() << "无法创建进程快照，错误码：" << GetLastError();
        return;
    }
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    // 获取第一个进程
    if (!Process32First(hProcessSnap, &pe32)) {
        qCritical() << "无法获取第一个进程，错误码：" << GetLastError();
        CloseHandle(hProcessSnap);
        return;
    }
    
    int processesClosed = 0;
    int processesSkipped = 0;
    
    // 遍历所有进程
    do {
        // 转换进程名为QString
        QString processName = QString::fromWCharArray(pe32.szExeFile);
        DWORD processId = pe32.th32ProcessID;
        
        // 跳过系统进程和当前进程
        if (processId == 0 || processId == currentProcessId) {
            processesSkipped++;
            continue;
        }
        
        // 检查是否在可安全关闭的进程列表中（不区分大小写）
        bool shouldClose = false;
        foreach (const QString& safeProcess, safeToCloseProcesses) {
            if (processName.compare(safeProcess, Qt::CaseInsensitive) == 0) {
                shouldClose = true;
                break;
            }
        }
        
        if (shouldClose) {
            qDebug() << "发现可关闭进程：" << processName << "(ID:" << processId << ")";
            
            // 尝试打开进程并强制关闭
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
            if (hProcess != NULL) {
                BOOL result = TerminateProcess(hProcess, 0);
                if (result) {
                    qDebug() << "✓ 成功关闭进程：" << processName;
                    processesClosed++;
                } else {
                    qWarning() << "✗ 无法关闭进程：" << processName << "，错误码：" << GetLastError();
                }
                CloseHandle(hProcess);
            } else {
                qWarning() << "✗ 无法打开进程：" << processName << "，错误码：" << GetLastError();
            }
        } else {
            processesSkipped++;
        }
    } while (Process32Next(hProcessSnap, &pe32));
    
    // 关闭进程快照句柄
    CloseHandle(hProcessSnap);
    
    qDebug() << "\n强制关闭进程结果：";
    qDebug() << "  - 关闭的进程数：" << processesClosed;
    qDebug() << "  - 跳过的进程数：" << processesSkipped;
    qDebug() << "========== 强制关闭进程结束 ==========\n";
}