#ifndef CLEAN_H
#define CLEAN_H

#include <Windows.h>
#include <psapi.h>
#include <QString>
#include <QDebug>
#include <tlhelp32.h>

// 移除MinGW不支持的pragma comment

// 定义系统信息类常量
#ifndef SystemMemoryListInformation
#define SystemMemoryListInformation 85 // Windows 10/11 正确值
#endif



// 定义内存清理命令枚举
#define MEMORY_CLEAN_COMMAND_EMPTY_WORKING_SETS             L"EmptyWorkingSets"
#define MEMORY_CLEAN_COMMAND_EMPTY_STANDBY_LIST             L"EmptyStandbyList"

// 定义内存列表命令结构体
typedef struct _SYSTEM_MEMORY_LIST_COMMAND {
    const wchar_t* Command;
    HANDLE ProcessHandle;
    ULONG Flags;
} SYSTEM_MEMORY_LIST_COMMAND, *PSYSTEM_MEMORY_LIST_COMMAND;

// 定义NtSetSystemInformation函数类型
typedef NTSTATUS(__stdcall* NtSetSystemInformationPtr)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength
);

// 定义NtQuerySystemInformation函数类型
typedef NTSTATUS(__stdcall* NtQuerySystemInformationPtr)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);

// 定义RtlAdjustPrivilege函数类型
typedef NTSTATUS(__stdcall* RtlAdjustPrivilegePtr)(
    ULONG Privilege,
    BOOLEAN Enable,
    BOOLEAN CurrentThread,
    PBOOLEAN Enabled
);

class MemoryCleaner {
public:
    // 执行快速系统内存清理（简化版本）
    static bool performFastSystemCleaning(bool forceCloseProcesses = false);

    // 获取简洁的系统内存使用情况
    static QString getMemoryUsage();

    // 检查是否以管理员权限运行
    static bool isRunningAsAdmin();

private:
    // 提升基本必要的权限
    static bool EnableBasicPrivileges();

    // 通过命令清理内存
    static bool CleanMemoryByCommand(const wchar_t* command);

    // 清理系统文件缓存
    static bool CleanSystemFileCache();
    
    // 清理临时文件
    static void cleanTempFiles();
    
    // 强制关闭不必要的进程
    static void forceCloseUnnecessaryProcesses();
};

#endif // CLEAN_H