#ifndef IPSENDER_H
#define IPSENDER_H
#include <QObject>
#include <QUdpSocket>
#include <QtNetwork>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QFileSystemWatcher>
#include <xmlparse.h>

struct watching
{
    QString dns_source;
    int subindex;
    int hostindex;
    int procindex;
    bool in_interface;
};

class ipsender : public QObject
{
    Q_OBJECT

public:
    ipsender();
    void populateList(sys &sys_1, QFileSystemWatcher &watched);
    void selfPopulate(QVector<watching> watched, QStringList paths, sys &sys_1);
    QString schemaPath;
    QUdpSocket *socket;
    QHostAddress groupAddress;
    int ssvTTL;
    quint16 ssvSocket;

public slots:
    void sendinfo();

};

#endif // SENDER_H
