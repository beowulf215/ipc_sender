#include "ipsender.h"

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
QString localhost;
QVector<watching> w_info;

ipsender::ipsender()
{
    subindex = 0;
    nameindex = 0;
    watchindex = 0;
    localhost = QHostInfo::localHostName();
    groupAddress = QHostAddress("239.255.43.21");
    ssvSocket = 45454;
    socket = new QUdpSocket(this);
}

void ipsender::populateList(sys &sys_1, QFileSystemWatcher &watched)
{


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

    watchindex = 0;
    selfPopulate(w_info, watched.files(), sys_1);


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

void ipsender::sendinfo()
{
    qDebug() << "Sent";
}
