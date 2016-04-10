#include "ipsender.h"
#include "xmlparse.h"

/*
 * For the FileSystemWatcher to work properly for the IPC Sender, the following MUST be in the main
 *
 *  QFileSystemWatcher *watched = new QFileSystemWatcher(&a); //"a" being the QApplication in the main.cpp
 *  ipsender send;
 *  send.populateList(ssv_sys, *watched);
 *  QObject::connect(watched,SIGNAL(fileChanged(QString)),&send,SLOT(sendinfo()));
 *
 * Only then will the program properly detect any changes to the watched files
 */

int subindex;
int nameindex;
int watchindex;
bool already_populated;
QString localhost;
QString datagram; //Datagram for changes in a file
QString datagram2; //Datagram for intermittent messages to the networked hosts
QVector<watching> w_info;
QStringList w_list;
sys *ssv;

ipsender::ipsender()
{
    subindex = 0;
    nameindex = 0;
    watchindex = 0;
    already_populated = false;
    localhost = QHostInfo::localHostName();
    groupAddress = QHostAddress("239.255.43.21");
    ssvSocket = 45454;
    socket = new QUdpSocket(this);
    socket->bind(QHostAddress::AnyIPv4, ssvSocket, QUdpSocket::ShareAddress);
    socket->joinMulticastGroup(groupAddress);


    QThread* time_thread = new QThread(0);
    QTimer* timer = new QTimer(this);

    timer->moveToThread(time_thread);

    connect(timer, SIGNAL(timeout()), this, SLOT(inter_send()));
    timer->start(4000);


}

void ipsender::populateList(sys &sys_1, QFileSystemWatcher &watched)
{
    if(!already_populated)
    {
        ssv = &sys_1;


    for (int i = 0; i < sys_1.index.size(); i++)
    {
        if(!QString::compare(localhost,sys_1.index[i].hostdns,Qt::CaseSensitive))
        {
            qDebug() << "Found a match for " << localhost << " on index " << i;
            subindex = sys_1.index[i].subindex;
            nameindex = sys_1.index[i].hostindex;
            break;
        }
    }

    for (int i = 0; i < sys_1.index.size(); i++)
    {
        if(sys_1.index[i].subindex == subindex && sys_1.index[i].hostindex == nameindex)
        {

            watched.addPath(sys_1.index[i].statuspath);
            w_info.push_back(watching());
            w_info[watchindex].subindex = subindex;
            w_info[watchindex].hostindex = nameindex;
            w_info[watchindex].in_interface = false;
            w_info[watchindex].procindex = sys_1.index[i].procindex;
            w_info[watchindex].dns_source = localhost;
            watchindex++;

            if(!sys_1.index[i].ifstatpath.isEmpty())
            {
                watched.addPath(sys_1.index[i].ifstatpath);
                w_info.push_back(watching());
                w_info[watchindex].subindex = subindex;
                w_info[watchindex].hostindex = nameindex;
                w_info[watchindex].in_interface = true;
                w_info[watchindex].procindex = sys_1.index[i].procindex;
                w_info[watchindex].dns_source = localhost;
                watchindex++;
            }
        }
    }



    qDebug() << "Watching list populated!";
    qDebug() << "Info on Watched Paths";

    for (int i = 0; i < w_info.size(); i++)
    {
        qDebug() << "Sub Index: " << w_info[i].subindex
                 << " Host Index: " << w_info[i].hostindex
                 << " Process Index: " << w_info[i].procindex
                 << " Interface?: " << w_info[i].in_interface
                 << " Source Host: " << w_info[i].dns_source
                 << " Status Path: " << watched.files()[i];
    }

    w_list = watched.files();

    watchindex = 0;
    selfPopulate(w_info, watched.files(), sys_1);
    already_populated = true;
    }

}

void ipsender::selfPopulate(QVector<watching> watched, QStringList paths, sys &sys_1)
{
    qDebug() << "Populating host's own data structure";
    for ( int i = 0; i < watched.size(); i++)
    {
        if (watched[i].procindex == -1)
        {
            if(watched[i].in_interface == false)
            {
                QFile file(paths[i]);

                if (!file.open(QFile::ReadOnly | QFile::Text)) break;
                QTextStream in(&file);

                sys_1.subsystems[watched[i].subindex].hosts[watched[i].hostindex].status = in.readAll();
                qDebug() << sys_1.subsystems[watched[i].subindex].hosts[watched[i].hostindex].status_path;
                qDebug() << sys_1.subsystems[watched[i].subindex].hosts[watched[i].hostindex].status;
            }
            else if (watched[i].in_interface == true)
            {
                QFile file(paths[i]);

                if (!file.open(QFile::ReadOnly | QFile::Text)) break;
                QTextStream in(&file);

                sys_1.subsystems[watched[i].subindex].hosts[watched[i].hostindex].hostInterface.status = in.readAll();
                qDebug() << sys_1.subsystems[watched[i].subindex].hosts[watched[i].hostindex].hostInterface.status_path;
                qDebug() << sys_1.subsystems[watched[i].subindex].hosts[watched[i].hostindex].hostInterface.status;
            }
        }

        if (watched[i].procindex != -1)
        {
            if(watched[i].in_interface == false)
            {
                QFile file(paths[i]);

                if (!file.open(QFile::ReadOnly | QFile::Text)) break;
                QTextStream in(&file);

                sys_1.subsystems[watched[i].subindex].hosts[watched[i].hostindex].processes[watched[i].procindex].status = in.readAll();
                qDebug() << sys_1.subsystems[watched[i].subindex].hosts[watched[i].hostindex].processes[watched[i].procindex].status_path;
                qDebug() << sys_1.subsystems[watched[i].subindex].hosts[watched[i].hostindex].processes[watched[i].procindex].status;
            }
            else if (watched[i].in_interface == true)
            {
                QFile file(paths[i]);

                if (!file.open(QFile::ReadOnly | QFile::Text)) break;
                QTextStream in(&file);

                sys_1.subsystems[watched[i].subindex].hosts[watched[i].hostindex].processes[watched[i].procindex].procInterface.status = in.readAll();
                qDebug() << sys_1.subsystems[watched[i].subindex].hosts[watched[i].hostindex].processes[watched[i].procindex].procInterface.status_path;
                qDebug() << sys_1.subsystems[watched[i].subindex].hosts[watched[i].hostindex].processes[watched[i].procindex].procInterface.status;
            }
        }
    }
}



void ipsender::sendinfo(const QString &path)
{
    if(already_populated)
    {

    qDebug() << "Detected Change for " + path;

    for (int i = 0; i < w_info.size(); i++)
    {
        if(!path.compare(w_list[i]))
        {
            //{
                if (w_info[i].procindex == -1)
                {
                    if(w_info[i].in_interface == false)
                    {
                        QFile file(w_list[i]);

                        if (!file.open(QFile::ReadOnly | QFile::Text)) break;
                        QTextStream in(&file);

                        ssv->subsystems[w_info[i].subindex].hosts[w_info[i].hostindex].status = in.readAll();

                        datagram = QString("%1;%2;%3;%4;%5").arg(w_info[i].subindex).arg(w_info[i].hostindex).arg(w_info[i].procindex)
                                .arg(w_info[i].in_interface).arg(ssv->subsystems[w_info[i].subindex].hosts[w_info[i].hostindex].status);

                    }
                    else if (w_info[i].in_interface == true)
                    {
                        QFile file(w_list[i]);

                        if (!file.open(QFile::ReadOnly | QFile::Text)) break;
                        QTextStream in(&file);

                        ssv->subsystems[w_info[i].subindex].hosts[w_info[i].hostindex].hostInterface.status = in.readAll();

                        datagram = QString("%1;%2;%3;%4;%5").arg(w_info[i].subindex).arg(w_info[i].hostindex).arg(w_info[i].procindex)
                                .arg(w_info[i].in_interface).arg(ssv->subsystems[w_info[i].subindex].hosts[w_info[i].hostindex].hostInterface.status);
                    }
                }

                if (w_info[i].procindex != -1)
                {
                    if(w_info[i].in_interface == false)
                    {
                        QFile file(w_list[i]);

                        if (!file.open(QFile::ReadOnly | QFile::Text)) break;
                        QTextStream in(&file);

                        ssv->subsystems[w_info[i].subindex].hosts[w_info[i].hostindex].processes[w_info[i].procindex].status = in.readAll();

                        datagram = QString("%1;%2;%3;%4;%5").arg(w_info[i].subindex).arg(w_info[i].hostindex).arg(w_info[i].procindex)
                                .arg(w_info[i].in_interface).arg(ssv->subsystems[w_info[i].subindex].hosts[w_info[i].hostindex].processes[w_info[i].procindex].status);
                    }
                    else if (w_info[i].in_interface == true)
                    {
                        QFile file(w_list[i]);

                        if (!file.open(QFile::ReadOnly | QFile::Text)) break;
                        QTextStream in(&file);

                        ssv->subsystems[w_info[i].subindex].hosts[w_info[i].hostindex].processes[w_info[i].procindex].procInterface.status = in.readAll();

                        datagram = QString("%1;%2;%3;%4;%5").arg(w_info[i].subindex).arg(w_info[i].hostindex).arg(w_info[i].procindex)
                                .arg(w_info[i].in_interface).arg(ssv->subsystems[w_info[i].subindex].hosts[w_info[i].hostindex].processes[w_info[i].procindex].procInterface.status);
                    }
                }
            //}

        }
    }

    qDebug() << "Datagram prepared as: " + datagram;
    QByteArray data (datagram.toStdString().c_str());
    socket->writeDatagram(data,groupAddress,ssvSocket);

    }

}


void ipsender::inter_send()
{
    if (already_populated)
    {
        qDebug() << "Sending Intermittent Datagram";
        for (int i = 0; i < w_info.size(); i++)
        {

            if (w_info[i].procindex == -1)
            {
                if(w_info[i].in_interface == false)
                {
                    datagram2 = QString("%1;%2;%3;%4;%5").arg(w_info[i].subindex).arg(w_info[i].hostindex).arg(w_info[i].procindex)
                            .arg(w_info[i].in_interface).arg(ssv->subsystems[w_info[i].subindex].hosts[w_info[i].hostindex].status);

                }
                else if (w_info[i].in_interface == true)
                {
                    datagram2 = QString("%1;%2;%3;%4;%5").arg(w_info[i].subindex).arg(w_info[i].hostindex).arg(w_info[i].procindex)
                            .arg(w_info[i].in_interface).arg(ssv->subsystems[w_info[i].subindex].hosts[w_info[i].hostindex].hostInterface.status);
                }
            }

            if (w_info[i].procindex != -1)
            {
                if(w_info[i].in_interface == false)
                {
                    datagram2 = QString("%1;%2;%3;%4;%5").arg(w_info[i].subindex).arg(w_info[i].hostindex).arg(w_info[i].procindex)
                            .arg(w_info[i].in_interface).arg(ssv->subsystems[w_info[i].subindex].hosts[w_info[i].hostindex].processes[w_info[i].procindex].status);
                }
                else if (w_info[i].in_interface == true)
                {
                    datagram2 = QString("%1;%2;%3;%4;%5").arg(w_info[i].subindex).arg(w_info[i].hostindex).arg(w_info[i].procindex)
                            .arg(w_info[i].in_interface).arg(ssv->subsystems[w_info[i].subindex].hosts[w_info[i].hostindex].processes[w_info[i].procindex].procInterface.status);
                }
            }

            qDebug() << "Intermittent Datagrams prepared as\n" << datagram2;
            QByteArray data2 (datagram2.toStdString().c_str());
            socket->writeDatagram(data2,groupAddress,ssvSocket);
        }
    }
}
