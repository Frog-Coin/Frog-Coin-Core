#include "masternodemanager.h"
#include "ui_masternodemanager.h"
#include "addeditadrenalinenode.h"
#include "adrenalinenodeconfigdialog.h"

#include "sync.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "activemasternode.h"
#include "masternodeconfig.h"
#include "masternodeman.h"
#include "masternode.h"
#include "walletdb.h"
#include "wallet.h"
#include "init.h"
#include "rpcserver.h"
#include "guiutil.h"
#include <boost/lexical_cast.hpp>
#include <fstream>

using namespace json_spirit;
using namespace std;

#include <QAbstractItemDelegate>
#include <QClipboard>
#include <QPainter>
#include <QTimer>
#include <QDebug>
#include <QScrollArea>
#include <QScroller>
#include <QDateTime>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QItemSelectionModel>
#include <QDesktopServices>

#if QT_VERSION < 0x050000
#include <QUrl>
#else
#include <QUrlQuery>
#endif

MasternodeManager::MasternodeManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MasternodeManager),
    clientModel(0),
    walletModel(0)
{
    ui->setupUi(this);

    ui->editButton->setEnabled(false);
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(false);

    int columnAddressWidth = 200;
    int columnProtocolWidth = 60;
    int columnStatusWidth = 80;
    int columnActiveWidth = 130;
    int columnLastSeenWidth = 130;

    ui->tableWidgetMasternodes->setColumnWidth(0, columnAddressWidth);
    ui->tableWidgetMasternodes->setColumnWidth(1, columnProtocolWidth);
    ui->tableWidgetMasternodes->setColumnWidth(2, columnStatusWidth);
    ui->tableWidgetMasternodes->setColumnWidth(3, columnActiveWidth);
    ui->tableWidgetMasternodes->setColumnWidth(4, columnLastSeenWidth);

    ui->tableWidgetMasternodes->setContextMenuPolicy(Qt::CustomContextMenu);
    QAction *copyAddressAction = new QAction(tr("Copy Address"), this);
    QAction *copyPubkeyAction = new QAction(tr("Copy Pubkey"), this);
    contextMenu = new QMenu();
    contextMenu->addAction(copyAddressAction);
    contextMenu->addAction(copyPubkeyAction);
    connect(ui->tableWidgetMasternodes, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));
    connect(copyAddressAction, SIGNAL(triggered()), this, SLOT(copyAddress()));
    connect(copyPubkeyAction, SIGNAL(triggered()), this, SLOT(copyPubkey()));

    ui->tableWidgetMasternodes->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_2->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateNodeList()));
    if(!GetBoolArg("-reindexaddr", false))
        timer->start(30000);

    updateNodeList();

}

void MasternodeManager::on_tabWidget_currentChanged(int index)
{
    if (index == 1)
    {
        on_UpdateButton_clicked();
    }
}

MasternodeManager::~MasternodeManager()
{
    delete ui;
}

void MasternodeManager::on_tableWidget_2_itemSelectionChanged()
{
    if(ui->tableWidget_2->selectedItems().count() > 0)
    {
        ui->editButton->setEnabled(true);
        ui->startButton->setEnabled(true);
        ui->stopButton->setEnabled(true);
    }
}

static QString seconds_to_DHMS(quint32 duration)
{
  QString res;
  int seconds = (int) (duration % 60);
  duration /= 60;
  int minutes = (int) (duration % 60);
  duration /= 60;
  int hours = (int) (duration % 24);
  int days = (int) (duration / 24);
  if((hours == 0)&&(days == 0))
      return res.asprintf("%02dm:%02ds", minutes, seconds);
  if (days == 0)
      return res.asprintf("%02dh:%02dm:%02ds", hours, minutes, seconds);
  return res.asprintf("%dd %02dh:%02dm:%02ds", days, hours, minutes, seconds);
}

void MasternodeManager::updateNodeList()
{
    static int64_t nTimeListUpdated = GetTime();
    int64_t nSecondsToWait = nTimeListUpdated - GetTime() + 30;
    if (nSecondsToWait > 0) return;

    TRY_LOCK(cs_masternodes, lockMasternodes);
    if(!lockMasternodes)
        return;

    ui->countLabel->setText("Updating...");
    ui->tableWidgetMasternodes->setSortingEnabled(false);
    ui->tableWidgetMasternodes->clearContents();
    ui->tableWidgetMasternodes->setRowCount(0);
    std::vector<CMasternode> vMasternodes = mnodeman.GetFullMasternodeVector();

    BOOST_FOREACH(CMasternode& mn, vMasternodes)
    {
        int mnRow = 0;
        ui->tableWidgetMasternodes->insertRow(0);

        // populate list
        // Address, Protocol, Status, Active Seconds, Last Seen, Pub Key
        QTableWidgetItem* addressItem = new QTableWidgetItem(QString::fromStdString(mn.addr.ToString()));
        QTableWidgetItem* protocolItem = new QTableWidgetItem(QString::number(mn.protocolVersion));
        QTableWidgetItem* statusItem = new QTableWidgetItem(QString::number(mn.IsEnabled()));
        QTableWidgetItem* activeSecondsItem = new QTableWidgetItem(seconds_to_DHMS((qint64)(mn.lastTimeSeen - mn.sigTime)));
        QTableWidgetItem* lastSeenItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat(mn.lastTimeSeen)));

        CScript pubkey;
        pubkey =GetScriptForDestination(mn.pubkey.GetID());
        CTxDestination address1;
        ExtractDestination(pubkey, address1);
        CFrogCoinAddress address2(address1);
        QTableWidgetItem *pubkeyItem = new QTableWidgetItem(QString::fromStdString(address2.ToString()));

        ui->tableWidgetMasternodes->setItem(mnRow, 0, addressItem);
        ui->tableWidgetMasternodes->setItem(mnRow, 1, protocolItem);
        ui->tableWidgetMasternodes->setItem(mnRow, 2, statusItem);
        ui->tableWidgetMasternodes->setItem(mnRow, 3, activeSecondsItem);
        ui->tableWidgetMasternodes->setItem(mnRow, 4, lastSeenItem);
        ui->tableWidgetMasternodes->setItem(mnRow, 5, pubkeyItem);
    }

    ui->countLabel->setText(QString::number(ui->tableWidgetMasternodes->rowCount()));
    ui->tableWidgetMasternodes->setSortingEnabled(true);
}


void MasternodeManager::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if(model)
    {
    }
}

void MasternodeManager::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
    if(model && model->getOptionsModel())
    {
    }

}

void MasternodeManager::on_createButton_clicked()
{
    AddEditAdrenalineNode* aenode = new AddEditAdrenalineNode();
    aenode->exec();
}

void MasternodeManager::on_startButton_clicked()
{
    std::string statusObj;

    // start the node
    QItemSelectionModel* selectionModel = ui->tableWidget_2->selectionModel();
    QModelIndexList selected = selectionModel->selectedRows();
    if(selected.count() == 0)
    {
        statusObj += "<br>Select a Masternode alias to start" ;
        QMessageBox msg;
        msg.setText(QString::fromStdString(statusObj));
        msg.exec();
        return;
    }

    QModelIndex index = selected.at(0);
    int r = index.row();
    std::string sAlias = ui->tableWidget_2->item(r, 0)->text().toStdString();

    if(pwalletMain->IsLocked()) {
        statusObj += "<br>Please unlock your wallet to start Masternode" ;
        QMessageBox msg;
        msg.setText(QString::fromStdString(statusObj));
        msg.exec();
        return;
    }

    statusObj += "<center>Alias: " + sAlias;

    BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
        if(mne.getAlias() == sAlias) {
            std::string errorMessage;
            std::string strDonateAddress = "";
            std::string strDonationPercentage = "";

            bool result = activeMasternode.Register(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), strDonateAddress, strDonationPercentage, errorMessage);

            if(result) {
                statusObj += "<br>Successfully started masternode." ;
            } else {
                statusObj += "<br>Failed to start masternode.<br>Error: " + errorMessage;
            }
            break;
        }
    }

    pwalletMain->Lock();
    statusObj += "</center>";
    QMessageBox msg;
    msg.setText(QString::fromStdString(statusObj));
    msg.exec();

    MasternodeManager::on_UpdateButton_clicked();
}

void MasternodeManager::showContextMenu(const QPoint& point)
{
    QTableWidgetItem* item = ui->tableWidgetMasternodes->itemAt(point);
    if (item) contextMenu->exec(QCursor::pos());
}

void MasternodeManager::copyAddress()
{
    std::string sData;
    int row;
    QItemSelectionModel* selectionModel = ui->tableWidgetMasternodes->selectionModel();
    QModelIndexList selectedRows = selectionModel->selectedRows();
    if(selectedRows.count() == 0)
        return;

    for (int i = 0; i < selectedRows.count(); i++)
    {
        QModelIndex index = selectedRows.at(i);
        row = index.row();
        sData += ui->tableWidgetMasternodes->item(row, 0)->text().toStdString();
        if (i < selectedRows.count()-1)
            sData += "\n";
    }

    QApplication::clipboard()->setText(QString::fromStdString(sData));
}

void MasternodeManager::copyPubkey()
{
    std::string sData;
    int row;
    QItemSelectionModel* selectionModel = ui->tableWidgetMasternodes->selectionModel();
    QModelIndexList selectedRows = selectionModel->selectedRows();
    if(selectedRows.count() == 0)
        return;

    for (int i = 0; i < selectedRows.count(); i++)
    {
        QModelIndex index = selectedRows.at(i);
        row = index.row();
        sData += ui->tableWidgetMasternodes->item(row, 5)->text().toStdString();
        if (i < selectedRows.count()-1)
            sData += "\n";
    }

    QApplication::clipboard()->setText(QString::fromStdString(sData));
}
