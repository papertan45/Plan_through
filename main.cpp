#include <QApplication>
#include <QMessageBox>
#include <QLocalServer>
#include <QLocalSocket>
#include "mainwindow.h"

#define SERVER_NAME "PlanThrough_SingleInstance_Server"

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    QApplication a(argc, argv);

    QLocalSocket socket;
    socket.connectToServer(SERVER_NAME);
    if(socket.waitForConnected(200))
    {
        QMessageBox::warning(nullptr, "提示", "软件已在运行中，请双击托盘内图标打开", QMessageBox::Ok);
        return 0;
    }

    QLocalServer* server = new QLocalServer(&a);
    // 每次有新连接，都断开并重新监听，永久占用服务名
    QObject::connect(server, &QLocalServer::newConnection, [=](){
        QLocalSocket *clientSocket = server->nextPendingConnection();
        clientSocket->close();
        clientSocket->deleteLater();
    });
    server->listen(SERVER_NAME);

    MainWindow w;
    w.show();

    return a.exec();
}
