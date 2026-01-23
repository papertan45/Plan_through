#include "widgetcontainer.h"

WidgetContainer widgetContainer;

QWidget* WidgetContainer::operator()(QString name){
    return m_container[name];
}

void WidgetContainer::operator()(QString name,QWidget* ptr){
    if(m_container[name] != nullptr)delete m_container[name];
    m_container[name] = ptr;
}
