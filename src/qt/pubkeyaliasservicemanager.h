#ifndef PUBKEYALIASSERVICEMANAGER_H
#define PUBKEYALIASSERVICEMANAGER_H

#include "util.h"
#include "sync.h"

#include <QMenu>
#include <QWidget>
#include <QTimer>
#include <QItemSelectionModel>

namespace Ui {
    class PubkeyaliasserviceManager;

}
class ClientModel;
class WalletModel;
class QAbstractItemView;
class QItemSelectionModel;

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Pubkeyaliasservice Manager page widget */
class PubkeyaliasserviceManager : public QWidget
{
    Q_OBJECT

public:
    explicit PubkeyaliasserviceManager(QWidget *parent = 0);
    ~PubkeyaliasserviceManager();

    void setClientModel(ClientModel *clientModel);
    void setWalletModel(WalletModel *walletModel);

private:
    QMenu* contextMenu;
    
public slots:
    void updateNodeList();
    void updateAdrenalineNode(QString alias, QString addr, QString privkey, QString txHash, QString txIndex, QString status);
    void on_UpdateButton_clicked();
    void copyAddress();
    void copyPubkey();

signals:

private:
    QTimer *timer;
    Ui::PubkeyaliasserviceManager *ui;
    ClientModel *clientModel;
    WalletModel *walletModel;
    CCriticalSection cs_adrenaline;

private slots:
    void showContextMenu(const QPoint&);
    void on_createButton_clicked();
    void on_startButton_clicked();
    void on_tableWidget_2_itemSelectionChanged();
    void on_tabWidget_currentChanged(int index);
};
#endif // PUBKEYALIASSERVICEMANAGER_H
