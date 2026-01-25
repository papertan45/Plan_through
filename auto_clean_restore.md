# 自动清理功能恢复计划

## 需要恢复的内容

1. **mainwindow.h**
   - 恢复内存监控定时器成员变量
   - 恢复initMemoryMonitorTimer函数声明
   - 恢复checkMemoryUsage槽函数
   - 恢复onAutoCleanThresholdChanged槽函数
   - 恢复onAutoCleanEnabledChanged槽函数

2. **mainwindow.cpp**
   - 恢复MainWindow构造函数中对initMemoryMonitorTimer的调用
   - 恢复initMemoryMonitorTimer函数实现
   - 恢复checkMemoryUsage函数实现，修改为执行深度清理
   - 恢复onAutoCleanThresholdChanged函数实现
   - 恢复onAutoCleanEnabledChanged函数实现

3. **appdatas.h**
   - 恢复自动清理相关成员变量
   - 恢复setAutoCleanMemoryThreshold函数
   - 恢复setAutoCleanMemoryEnabled函数
   - 恢复autoCleanMemoryThreshold函数
   - 恢复isAutoCleanMemoryEnabled函数

4. **appdatas.cpp**
   - 恢复initSettings函数中加载自动清理设置的代码
   - 恢复saveSettings函数中保存自动清理设置的代码

5. **mainwindow.cpp**（设置窗口）
   - 恢复autoCleanLayout的创建和设置
   - 恢复自动清理相关UI控件
   - 恢复autoCleanLayout的添加到mainLayout

## 修改内容

1. **checkMemoryUsage函数**
   - 修改MemoryCleaner::performFastSystemCleaning调用，将参数改为true，执行深度清理

2. **设置窗口UI**
   - 保持自动清理相关UI不变，用户可以继续调整自动清理设置

## 实现步骤

1. 恢复头文件中的声明
2. 恢复源文件中的实现
3. 修改checkMemoryUsage函数，执行深度清理
4. 恢复设置窗口中的自动清理UI
5. 测试编译和运行
