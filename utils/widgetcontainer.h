#ifndef WIDGETCONTAINER_H
#define WIDGETCONTAINER_H

#include <QWidget>
#include <QMap>
#include <QString>

/**
 * @brief The WidgetContainer class
 * 存储主要的几个组件的指针。
 */
class WidgetContainer
{
public:
    /**
     * @brief operator ()
     * @param name 组件名称，一般为其类型，但首字母小写
     * @return 组件指针
     */
    QWidget* operator()(QString name);
    /**
     * @brief operator () 存储组件指针
     * @param name 要存储的组件名称，一般为其类型，但首字母小写
     * @param ptr 组件指针
     */
    void operator()(QString name,QWidget* ptr);

private:
    QMap<QString,QWidget*> m_container;
};

extern WidgetContainer widgetContainer;

#endif // WIDGETCONTAINER_H
