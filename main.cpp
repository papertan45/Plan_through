#include <QApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include "mainwindow.h"

#define SERVER_NAME "PlanThrough_SingleInstance_Server"
static MainWindow *g_mainWindow = nullptr;

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    QApplication a(argc, argv);

    QLocalSocket socket;
    socket.connectToServer(SERVER_NAME);
    if(socket.waitForConnected(200))
    {
        socket.close();
        return 0;
    }

    QLocalServer* server = new QLocalServer(&a);
    QObject::connect(server, &QLocalServer::newConnection, [=](){
        QLocalSocket *clientSocket = server->nextPendingConnection();
        clientSocket->close();
        clientSocket->deleteLater();
        if(g_mainWindow)
        {
            g_mainWindow->showWindowFromTray();
        }
    });
    server->listen(SERVER_NAME);

    MainWindow w;
    g_mainWindow = &w;
    w.show();

    return a.exec();
}