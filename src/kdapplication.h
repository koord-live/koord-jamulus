#include <QApplication>
#include <QDebug>
#include <QFileOpenEvent>
#include <clientdlg.h>


class KdApplication : public QApplication
{
public:
    KdApplication(int &argc, char **argv)
        : QApplication(argc, argv)
    {
    }

    // custom event handler for macOS (+ iOS?) custom url handling - koord://<address> urls
    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::FileOpen)
        {
            QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
            qInfo() << "Open URL" << openEvent->url();
            QString nu_address = openEvent->url().toString();

            // here we have a URL open event on macOS
            // get reference to CClientDlg object, and call connect
            const QWidgetList &list = QApplication::topLevelWidgets();
            for(QWidget *w : list)
            {
                CClientDlg *mainWindow = qobject_cast<CClientDlg*>(w);
                if(mainWindow)
                {
                    qDebug() << "MainWindow found" << w;
                    qInfo() << "Emitting EventJoinConnectClicked signal with url: " << nu_address;
                    // send EventJoinConnectClicked signal, trigger OnEventJoinConnectClicked
                    emit mainWindow->EventJoinConnectClicked(nu_address);
                }
            }
        }

        return QApplication::event(event);
    }
};


