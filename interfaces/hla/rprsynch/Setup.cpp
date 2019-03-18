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
#include "Setup.h"
#include "FileNameWidget.h"
#include "DebugWindow.h"
#include "Welcome.h"

#include "Config.h"
#include <QMessageBox>
#include <sstream>

using namespace SNT_HLA;

QString Setup::findFile(QString filename)
{
    QList<QFileInfo> fedFileInfo;
    fedFileInfo.append(QFileInfo(filename));

    fedFileInfo.append(QFileInfo(QString(fedFileInfo[0].fileName())));
    fedFileInfo.append(QFileInfo(QString("../")+fedFileInfo[0].fileName()));
    fedFileInfo.append(QFileInfo(extractor->exeHome+"/bin/"+fedFileInfo[0].fileName()));
    fedFileInfo.append(QFileInfo(extractor->exeHome+"/gui/"+fedFileInfo[0].fileName()));
    fedFileInfo.append(QFileInfo(extractor->exeHome+"/interfaces/hla/rprsynch/data"+fedFileInfo[0].fileName()));
    fedFileInfo.append(QFileInfo(extractor->extractorHome+"/bin/"+fedFileInfo[0].fileName()));
    fedFileInfo.append(QFileInfo(extractor->extractorHome+"/gui/"+fedFileInfo[0].fileName()));
    fedFileInfo.append(QFileInfo(extractor->extractorHome+"/interfaces/hla/rprsynch/data"+fedFileInfo[0].fileName()));
    int i;
    for( i=0; i<fedFileInfo.size(); i++ )
    {
        if( fedFileInfo[i].exists() )
        {
            return fedFileInfo[i].canonicalFilePath();
        }
    }
    return QString("");
}

Setup::Setup(Extractor *ext) : QDialog(0, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
                extractor(ext)
{
    setupUi(this);
    setSizeGripEnabled(true);
    if( !extractor->settings.value("scenarioDirectory").isNull() )
    {
        scenarioDirectory->setText(extractor->settings.value("scenarioDirectory").toString());
    }
    else if( !extractor->extractorHome.isEmpty() )
        scenarioDirectory->setText(extractor->extractorHome+"\\scenarios\\"+scenarioName->text());
    else if( !extractor->userHome.isEmpty() )
        scenarioDirectory->setText(extractor->userHome+"\\"+scenarioName->text());

    QString fedPath = findFile(SNT_HLA::Config::instance().fedPath.c_str());
    if( fedPath.isEmpty() )
        fedFileName->setText("");
    else
        fedFileName->setText(fedPath);

    timeout->setSuffix("  seconds");
    timeout->setMaximum(99999);
    timeout->setSpecialValueText(tr("None"));

    connect(UsersGuide, SIGNAL(clicked()), extractor, SLOT(openUsersGuide()));
    connect(Next, SIGNAL(clicked()), this, SLOT(accept()));
    connect(findFedFile, SIGNAL(clicked()), this, SLOT(findFed()));
    connect(findScenarioDirectory, SIGNAL(clicked()), this, SLOT(findScnDir()));
    connect(scenarioDirectory, SIGNAL(textEdited(const QString &)), this, SLOT(setScnDir(const QString &)));
    connect(timeout, SIGNAL(valueChanged(int)), this, SLOT(handleTimeoutUpdate(int)));
}
void Setup::handleTimeoutUpdate(int i)
{
    if( i == 1 )
        timeout->setSuffix("  second");
    else
        timeout->setSuffix("  seconds");
}

void Setup::init()
{
}

void Setup::accept()
{
    extractor->settings.setValue("defaultTimeout", timeout->value());

    if( fedFileName->text().isEmpty() )
    {
        QMessageBox::warning(this, tr("HLA Extractor"),
                tr("You must select a FED file"),
                   QMessageBox::Ok);
        return;
    }

    QFileInfo scenDir(scenarioDirectory->text());
    if( scenDir.exists() && !scenDir.isDir() )
    {
        QMessageBox::warning(this, tr("HLA Extractor"),
                tr("Won't be able to create the scenarios\n"
                      "There is a file with the same name as the scenario directory"),
                   QMessageBox::Ok);
        return;
    }

    if( debugOutput->isChecked() && !extractor->dbW)
    {
        extractor->dbW = new DebugWindow;
        extractor->dbW->setWindowTitle("HLA Extractor - Diagnostic Output");
        extractor->dbW->grabStream(std::cout);
        extractor->dbW->grabStream(std::cerr);
        extractor->dbW->grabStream(std::clog);
        extractor->dbW->resize(700,500);
        extractor->dbW->show();
    }
    extractor->Extract();
}

void Setup::findFed()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open FED file"),
            extractor->exeHome+"\\gui", tr("FED Files (*.fed);;All Files (*.*)"));
    if( !filename.isEmpty() )
        fedFileName->setText(filename);
}
void Setup::findScnDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
        extractor->extractorHome+"\\scenarios\\", QFileDialog::ShowDirsOnly );
    if( !dir.isEmpty() )
    {
        setScnDir(dir);
    }
}
void Setup::setScnDir(const QString &dir)
{
    scenarioDirectory->setText(dir);
    extractor->settings.setValue("scenarioDirectory", dir);
}

void Setup::back()
{
    hide();
    extractor->welcome->show();
}
