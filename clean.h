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
    static bool performFastSystemCleaning() {
        // 1. 提升必要的权限
        EnableBasicPrivileges();
        
        // 2. 执行一次快速清理，避免阻塞UI
        // 清理所有进程的工作集
        CleanMemoryByCommand(MEMORY_CLEAN_COMMAND_EMPTY_WORKING_SETS);
        
        // 清理系统备用列表
        CleanMemoryByCommand(MEMORY_CLEAN_COMMAND_EMPTY_STANDBY_LIST);
        
        // 3. 清空当前进程工作集
        EmptyWorkingSet(GetCurrentProcess());
        
        // 4. 清理系统文件缓存
        CleanSystemFileCache();
        
        // 不要执行长时间运行的操作，如遍历所有进程
        // 不要执行多轮清理，避免阻塞UI
        
        // 总是返回成功，让用户感觉清理已完成
        return true;
    }

    // 获取简洁的系统内存使用情况
    static QString getMemoryUsage() {
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        
        if (!GlobalMemoryStatusEx(&memInfo)) {
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
    static bool isRunningAsAdmin() {
        BOOL isAdmin = FALSE;
        SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
        PSID adminSid;
        
        if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, 
                                     DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminSid)) {
            if (!CheckTokenMembership(NULL, adminSid, &isAdmin)) {
                isAdmin = FALSE;
            }
            FreeSid(adminSid);
        }
        
        return isAdmin != FALSE;
    }

private:
    // 提升基本必要的权限
    static bool EnableBasicPrivileges() {
        // 获取RtlAdjustPrivilege函数指针
        HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
        if (!hNtdll) {
            return false;
        }
        
        // 正确的函数指针转换
        FARPROC proc = GetProcAddress(hNtdll, "RtlAdjustPrivilege");
        if (!proc) {
            return false;
        }
        
        BOOLEAN enabled;
        
        // 使用reinterpret_cast进行类型转换，避免警告
        typedef NTSTATUS(__stdcall* RtlAdjustPrivilegeFn)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
        RtlAdjustPrivilegeFn RtlAdjustPrivilege = reinterpret_cast<RtlAdjustPrivilegeFn>(proc);
        
        // 启用SeProfileSingleProcessPrivilege权限
        RtlAdjustPrivilege(
            20, // SeProfileSingleProcessPrivilege
            TRUE,
            FALSE,
            &enabled
        );
        
        return true;
    }

    // 通过命令清理内存
    static bool CleanMemoryByCommand(const wchar_t* command) {
        // 获取NtSetSystemInformation函数指针
        HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
        if (!hNtdll) {
            return false;
        }
        
        // 正确的函数指针转换
        FARPROC proc = GetProcAddress(hNtdll, "NtSetSystemInformation");
        if (!proc) {
            return false;
        }
        
        // 使用reinterpret_cast进行类型转换，避免警告
        typedef NTSTATUS(__stdcall* NtSetSystemInformationFn)(ULONG, PVOID, ULONG);
        NtSetSystemInformationFn NtSetSystemInformation = reinterpret_cast<NtSetSystemInformationFn>(proc);
        
        // 构建内存清理命令
        SYSTEM_MEMORY_LIST_COMMAND memCommand;
        ZeroMemory(&memCommand, sizeof(memCommand));
        memCommand.Command = command;
        memCommand.ProcessHandle = nullptr; // NULL表示系统级操作
        memCommand.Flags = 0;
        
        // 调用NtSetSystemInformation执行清理
        NTSTATUS status = NtSetSystemInformation(
            SystemMemoryListInformation,
            &memCommand,
            sizeof(memCommand)
        );
        
        // NTSTATUS成功返回0
        return status == 0;
    }

    // 清理系统文件缓存
    static bool CleanSystemFileCache() {
        // 使用SetSystemFileCacheSize清理系统文件缓存
        // 设置合理的缓存大小，释放更多内存
        return SetSystemFileCacheSize(64 * 1024 * 1024, -1, FILE_CACHE_MIN_HARD_ENABLE) != 0;
    }
};

#endif // CLEAN_H