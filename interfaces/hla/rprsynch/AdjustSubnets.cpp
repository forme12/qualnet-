// Copyright (c) 2001-2008, Scalable Network Technologies, Inc.  All Rights Reserved.
//                          6100 Center Drive West
//                          Suite 1250 
//                          Los Angeles, CA 90045
//                          sales@scalable-networks.com
//
// This source code is licensed, not sold, and is subject to a written
// license agreement.  Among other things, no portion of this source
// code may be copied, transmitted, disclosed, displayed, distributed,
// translated, used as the basis for a derivative work, or used, in
// whole or in part, for any program or purpose other than its intended
// use in compliance with the license agreement as part of the EXata
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.

#include "Extractor.h"
#include "AdjustSubnets.h"
#include "AdjustNames.h"
#include "Setup.h"
#include "rpr_types.h"

#include <QComboBox>
#include <sstream>

typedef struct subnetRowInfo
{
    int row;
    SNT_HLA::Node* node;
    SNT_HLA::Network* net;
} subnetRowInfo;

QList<subnetRowInfo> rowInfo;

AdjustSubnets::AdjustSubnets(Extractor *ext) : QDialog(0, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
            extractor(ext)
{
    setupUi(this);
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->verticalHeader()->hide();
    setSizeGripEnabled(true);
    connect(UsersGuide, SIGNAL(clicked()), extractor, SLOT(openUsersGuide()));
    connect(Back, SIGNAL(clicked()), this, SLOT(back()));
    connect(Finish, SIGNAL(clicked()), this, SLOT(accept()));
}

void AdjustSubnets::show()
{
    QDialog::show();
    scenarioName->setText(QString(     "Scenario Name:       ") + extractor->setup->scenarioName->text());
    scenarioDirectory->setText(QString("Scenario Directory:  ") + extractor->setup->scenarioDirectory->text());

    tableWidget->horizontalHeaderItem(0)->setToolTip("This is the name of the radio. It consists of the EXata/QualNet name and the interface id");
    tableWidget->horizontalHeaderItem(1)->setToolTip("The radio belongs to this force");
    tableWidget->horizontalHeaderItem(2)->setToolTip("Select an IPv4 subnet for the radio to use for communicationsI");

    QStringList addrs;
    SNT_HLA::NetworkSet& nets = extractor->ns->networksUsed();
    SNT_HLA::NetworkSet::iterator netIt = nets.begin();
    while( netIt != nets.end() )
    {
        QString addr((*netIt)->address.c_str());
        if( addrs.indexOf(addr) == -1 )
            addrs << addr;
        netIt++;
    }

    tableWidget->clearContents();
    tableWidget->setRowCount(0);
    QAbstractItemModel* model = tableWidget->model();
    rowInfo.clear();
    SNT_HLA::NodeSet::iterator it = extractor->ns->begin();
    while( it != extractor->ns->end() )
    {
        QApplication::processEvents();
        if( extractor == 0 )
            break;
        if( (*it)->entity )
        {
            std::set<SNT_HLA::RadioTransmitter*>::iterator rit = (*it)->radios.begin();
            while( rit != (*it)->radios.end() )
            {
                int row = tableWidget->rowCount();
                tableWidget->insertRow(row);

                subnetRowInfo info;
                info.row = row;
                info.node = *it;
                info.net = extractor->ns->getNetwork(*it, (*rit));
                rowInfo.append(info);

                QString name = QString((*it)->getNodeName()) + ":" + QString::number((*rit)->RadioIndex);
                QString addr(extractor->ns->getNetworkAddress(*it, (*rit)).c_str());
                QModelIndex index = model->index(row, 0, QModelIndex());
                std::stringstream fs;
                fs << (*it)->entity->ForceIdentifier << std::ends;
                QString force = fs.str().c_str();
                model->setData(index, name);
                index = model->index(row, 1, QModelIndex());
                model->setData(index, force);
                QComboBox* box = new QComboBox;
                box->addItems(addrs);
                int idx = addrs.indexOf(addr);
                if( idx == -1 )
                {
                    idx = box->count();
                    box->insertItem(idx, addr);
                }
                box->setCurrentIndex(idx);
                box->setToolTip("Select an IPv4 subnet for the radio to use for communications");
                tableWidget->setCellWidget(row, 2, box);
                tableWidget->item(row, 0)->setFlags(0);
                tableWidget->item(row, 1)->setFlags(0);
                tableWidget->item(row,0)->setToolTip("This is the name of the radio. It consists of the EXata/QualNet name and the interface id");
                tableWidget->item(row,1)->setToolTip("The radio belongs to this force");
                tableWidget->resizeColumnToContents(0);
                rit++;
            }
        }
        it++;
    }
}

void AdjustSubnets::accept()
{
    for( int row=0; row<tableWidget->rowCount(); row++)
    {
        subnetRowInfo info = rowInfo.at(row);
        if( !info.net )
            continue;
        QComboBox* box = dynamic_cast<QComboBox*>(tableWidget->cellWidget(row,2));
        if( box->currentText().compare( info.net->address.c_str() ) != 0 )
        {
            SNT_HLA::Network* net = extractor->ns->getNetwork(box->currentText().toAscii().data());
            if( net )
            {
                info.net->removeNode(info.node);
                net->addNode(info.node);
            }
        }
    }

    extractor->WriteFiles();
    hide();
    extractor->cleanUp();
}

void AdjustSubnets::back()
{
    for( int row=0; row<tableWidget->rowCount(); row++)
    {
        subnetRowInfo info = rowInfo.at(row);
        if( !info.net )
            continue;
        QComboBox* box = dynamic_cast<QComboBox*>(tableWidget->cellWidget(row,2));
        QString curr = box->currentText();
        std::string old = info.net->address;
        if( box->currentText().compare( info.net->address.c_str() ) != 0 )
        {
            SNT_HLA::Network* net = extractor->ns->getNetwork(box->currentText().toAscii().data());
            if( net )
            {
                info.net->removeNode(info.node);
                net->addNode(info.node);
            }
        }
    }
    hide();
    extractor->adjustNames->show();
}


