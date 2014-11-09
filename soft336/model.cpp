#include "model.h"

/**
 * @brief ClientInfo::ClientInfo
 * @param parent
 * @param clientAddress
 */
ClientInfo::ClientInfo(QObject *parent, QString clientAddress) : QObject(parent)
{
    qDebug() << "Client Created";
    address = clientAddress;

    //start the timeout timer
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerExpired()));
    timer->start(2000);

    //start the control string listener
    socketTCP = new QTcpSocket(this);
    socketTCP->connectToHost(address, TCP_PORT);
    connect(socketTCP, SIGNAL(readyRead()), this, SLOT(readyRead()));

    qDebug() << socketTCP;
}

void ClientInfo::timerExpired()
{
    qDebug() << "Client Timeout";
    emit clientTimeout();
}

void ClientInfo::restartTimer()
{
    //qDebug() << "Timer Restarted";
    timer->start(2000);
}

QString ClientInfo::getAddress() const
{
    return address;
}

void ClientInfo::readyRead()
{
    QByteArray data;

    while(socketTCP->bytesAvailable() > 0)
        data.append(socketTCP->readAll());

    qDebug() << "data received" << data << " from " << address;
    //do something with data here
}

/**
 * @brief ClientList::ClientList
 * @param parent
 */
ClientList::ClientList(QObject *parent)
//    : QAbstractListModel(parent)
    :QAbstractTableModel(parent)
{
    clients = QList<ClientInfo *>();
}

int ClientList::rowCount(const QModelIndex &) const
{
    return clients.size();
}

int ClientList::columnCount(const QModelIndex &) const
{
    return 3;
}

QVariant ClientList::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (clients.size() > 0)
            return clients.at(index.row())->getAddress(); //problem?
    }
    return QVariant();
}

void ClientList::appendClient(QString clientAddress)
{
    if(!hasAddress(clientAddress)) {
        qDebug("Client Added");
        beginInsertRows(QModelIndex(), 0, 0);
        ClientInfo *newClient = new ClientInfo(this, clientAddress);
        clients.insert(0, newClient);
        connect(newClient, SIGNAL(clientTimeout()), this, SLOT(clientTimeout()));
        endInsertRows();
    }
}

QHostAddress ClientList::getAddressAt(const QModelIndex &index)
{
    //qDebug() << "get address at (clientlist) called" << clients.at(index.row())->getAddress();
    return QHostAddress(clients.at(index.row())->getAddress());
}

//iterate through our list, return true if we find a matching address (and restarts it's timer)
bool ClientList::hasAddress(QString address)
{
    QList<ClientInfo *>::iterator i;
    for (i = clients.begin(); i != clients.end(); ++i) {
        if((*i)->getAddress() == address) {
            (*i)->restartTimer();
            return true;
        }
    }
    return false;
}



void ClientList::clientTimeout()
{
    //remove the timeout client here
    int loc = clients.indexOf((ClientInfo *)QObject::sender());
    beginRemoveRows(QModelIndex(), loc, loc);
    qDebug("Client Removed");
    clients.removeAt(loc);
    delete QObject::sender(); //might not be needed?
    endRemoveRows();
}
