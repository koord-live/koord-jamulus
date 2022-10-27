#include <QApplication>
#include <QDebug>
#include <QFileOpenEvent>
//#include <clientdlg.h>
//#include <urlhandler.h>
#include <singleapplication.h>

//FIXME - special handling for M1 Macs
#if (defined ( Q_OS_MACX ) && ifndef Q_PROCESSOR_X86_64)
class KdApplication : public QApplication
#else
class KdApplication : public SingleApplication
#endif
{
    Q_OBJECT

public:
    KdApplication(int& argc, char* argv[]);

    int run();

public slots:
    void OnConnectFromURLHandler(const QString& value);

    // custom event handler for macOS (+ iOS?) custom url handling - koord://<address> urls
//    bool event(QEvent *event) override;
protected:
    bool event(QEvent *event) override;

};


