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
#include "AdjustNames.h"
#include "AdjustSubnets.h"
#include "Setup.h"
#include "FileNameWidget.h"

#include <QImageReader>
#include <QHeaderView>

QString getImageFilter()
{
    QString filter("Images (");
    QList<QByteArray> imageFormats = QImageReader::supportedImageFormats();
    filter = filter + "*." + imageFormats.at(0);
    for( int i=1; i<imageFormats.size(); i++)
    {
        filter += " *." + imageFormats.at(i);
    }
    filter += ");;All Files (*.*)";
    return filter;
}

AdjustNames::AdjustNames(Extractor *ext) : QDialog(0, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
                extractor(ext)
{
    setupUi(this);
    setSizeGripEnabled(true);
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->verticalHeader()->hide();
    tableWidget->setEditTriggers(QAbstractItemView::AllEditTriggers);
    connect(UsersGuide, SIGNAL(clicked()), extractor, SLOT(openUsersGuide()));
    connect(Next, SIGNAL(clicked()), this, SLOT(accept()));
    connect(Back, SIGNAL(clicked()), this, SLOT(back()));
}
/*
void AdjustNames::resizeEvent( QResizeEvent * event )
{
    int w = event->size().width();
    int h = event->size().height();
    BackFrame->resize(event->size());
    if( w > 300 && h > 150 )
    {
        Cancel->setGeometry(w-85,h-36,77,26);
        Next->setGeometry(w-168,h-36,77,26);
        line->setGeometry(14,h-53,w-21,16);
        title->setGeometry(220,21,w-299,18);
        instructions->setGeometry(220,h-79,w-229,18);
        tableWidget->setGeometry(220, 45, w-229, h-130);
    }
}
*/
void AdjustNames::show()
{
    QDialog::show();

    scenarioName->setText(QString(     "Scenario Name:       ") + extractor->setup->scenarioName->text());
    scenarioDirectory->setText(QString("Scenario Directory:  ") + extractor->setup->scenarioDirectory->text());

    tableWidget->clearContents();
    tableWidget->setRowCount(0);

    tableWidget->horizontalHeaderItem(0)->setToolTip("This is the node id used by the EXata/QualNet simulator and GUI");
    tableWidget->horizontalHeaderItem(1)->setToolTip("This is the name used by the HLA federate that published the entity");
    tableWidget->horizontalHeaderItem(2)->setToolTip("Enter text to change the name that will be used by the EXata/QualNet simulator and GUI");
    tableWidget->horizontalHeaderItem(3)->setToolTip("Enter an icon file name or browse to change the icon used by the EXata/QualNet simulator and GUI");

    QAbstractItemModel* model = tableWidget->model();
    SNT_HLA::NodeSet::iterator it = extractor->ns->begin();
    //QFontMetrics fontMetrics = tableWidget->fontMetrics();
    //int iconWidth = 0;
    while( it != extractor->ns->end() )
    {
        QApplication::processEvents();
        if( extractor == 0 )
            break;
        if( (*it)->entity )
        {
            QString name((*it)->getNodeName());
            int id = (*it)->NodeId;
            QString iconName((*it)->getIconName().c_str());
            QIcon icon(iconName);
            QFileInfo iconInfo(iconName);
            int row = tableWidget->rowCount();
            tableWidget->insertRow(row);
            QModelIndex index = model->index(row, 1, QModelIndex());
            model->setData(index, name);
            index = model->index(row, 2, QModelIndex());
            model->setData(index, name);
            index = model->index(row, 0, QModelIndex());
            model->setData(index, id);
            tableWidget->item(row, 0)->setFlags(0);
            tableWidget->item(row, 1)->setFlags(0);
            FileNameWidget* wid = new FileNameWidget("Open Icon File", extractor->exeHome+"/gui/icons", getImageFilter(),tableWidget);
            wid->setText(iconName);
            wid->setToolTip("Enter an icon file name or browse to change the icon used by the EXata/QualNet simulator and GUI");
            tableWidget->setCellWidget(row, 3, wid);
            tableWidget->item(row,0)->setToolTip("This is the node id used by the EXata/QualNet simulator and GUI");
            tableWidget->item(row,1)->setToolTip("This is the name used by the HLA federate that published the entity");
            tableWidget->item(row,2)->setToolTip("Enter text to change the name that will be used by the EXata/QualNet simulator and GUI");
            tableWidget->resizeColumnToContents(0);
            tableWidget->resizeColumnToContents(1);
            tableWidget->resizeColumnToContents(2);
        }
        //tableWidget->setColumnWidth(3, iconWidth);
        it++;
    }
}
void AdjustNames::accept()
{
    QAbstractItemModel* model = tableWidget->model();
    for( int row=0; row<tableWidget->rowCount(); row++)
    {
        QModelIndex index = model->index(row, 0, QModelIndex());
        int id = model->data(index).toInt();
        SNT_HLA::Node* node = extractor->ns->findByNodeID(id);
        if( node && node->entity )
        {
            index = model->index(row, 2, QModelIndex());
            QString nodeName = model->data(index).toString();
            node->setNodeName(nodeName.toAscii().data());
            index = model->index(row, 3, QModelIndex());
            QIcon icon = model->data(index).value<QIcon>();
            FileNameWidget* wid = dynamic_cast<FileNameWidget*>(tableWidget->cellWidget(row, 3));
            QString iconName = wid->text();
            node->setIconName(iconName.toAscii().data());
        }
    }
    hide();
    extractor->adjustSubnets->show();
}
void AdjustNames::back()
{
    hide();
    extractor->setup->show();
}

