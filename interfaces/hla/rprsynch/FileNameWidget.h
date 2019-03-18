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

#ifndef _HLA_SNT_FILE_NAME_WIDGET_H_
#define _HLA_SNT_FILE_NAME_WIDGET_H_

#include <QFileDialog>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QResizeEvent>
#include <QToolButton>
#include <QHBoxLayout>

class FileNameWidget : public QWidget
{
    Q_OBJECT

    private:
        QIcon icon;
        QLabel* iconLabel;
        QLineEdit* nameEditor;
        QToolButton* findFile;
        QHBoxLayout* layout;
        QString Caption;
        QString Dir;
        QString Filter;
    public:
        FileNameWidget(QString caption, QString dir, QString filter, QWidget *parent = 0) : 
          Caption(caption), Dir(dir), Filter(filter), QWidget(parent)
        {
            iconLabel = new QLabel(this);
            nameEditor = new QLineEdit(this);
            nameEditor->setFrame(false);
            findFile = new QToolButton(this);
            findFile->setText("...");
            layout = new QHBoxLayout(this);
            layout->setContentsMargins(0,0,0,0);
            layout->setMargin(0);
            layout->addWidget(iconLabel);
            layout->addWidget(nameEditor);
            layout->addWidget(findFile);
            setLayout(layout);
            connect(findFile, SIGNAL(clicked()), this, SLOT(findFileSlot()));
        }
        void setText(QString str, bool shortName=true)
        {
            icon = QIcon(str);
            if( !icon.isNull() )
                iconLabel->setPixmap(icon.pixmap(iconLabel->size()));
            if(shortName)
            {
                QFileInfo iconInfo(str);
                nameEditor->setText(iconInfo.baseName());
            }
            else
            {
                nameEditor->setText(str);
            }
        }
        QString text()
        {
            return nameEditor->text();
        }
        void resizeEvent( QResizeEvent * event )
        {
            int w = event->size().width();
            int h = event->size().height();
            iconLabel->setGeometry(1,1,h-2,h-2);
            nameEditor->setGeometry(h,1,w-2*h-1,h-2);
            findFile->setGeometry(w-h,1,h-2,h-2);
        }
        
    public slots:
        void findFileSlot()
        {
            QString filename = QFileDialog::getOpenFileName(this, Caption,
                Dir, Filter);
            if( !filename.isEmpty() )
            setText(filename, false);
            updateGeometry();
        }
};

#endif
