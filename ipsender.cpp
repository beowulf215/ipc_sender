#include "ipsender.h"

int subindex;
int nameindex;
int watchindex;
QString localhost;
QFileSystemWatcher watched;
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

void ipsender::populateList(sys &sys_1)
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
    qDebug() << "List of status paths being monitored...";
    qDebug() << watched.files();
    qDebug() << "Info for corresponding paths...";

    for (int i = 0; i < w_info.size(); i++)
    {
        qDebug() << "Sub Index: " << w_info[i].subindex
                 << " Host Index: " << w_info[i].hostindex
                 << " Process Index: " << w_info[i].procindex
                 << " Interface?: " << w_info[i].in_interface
                 << " Source Host: " << w_info[i].dns_source;
    }



}
