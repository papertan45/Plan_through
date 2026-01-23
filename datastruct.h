#ifndef DATASTRUCT_H
#define DATASTRUCT_H

#include<QString>
#include<QMap>

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

#endif // DATASTRUCT_H
