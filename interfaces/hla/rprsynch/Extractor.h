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

#ifndef _SNT_HLA_EXTRACTOR_H_
#define _SNT_HLA_EXTRACTOR_H_

#include "FedAmb.h"
#include "sim_types.h"
#include "ui_Extract.h"

#include <QSettings>
#include <QProgressDialog>
#include <QThread>
#include <QIcon>
#include <QTime>


class Extractor;
class QApplication;
class Welcome;
class Setup;
class AdjustSubnets;
class AdjustNames;
class UpdateFedAmb;
class RtiConnectThread;
class DebugWindow;

class UpdateFedAmb : public QDialog, public Ui::Extract
{
    Q_OBJECT

    private:
        QTimer* timer;
        QTime startTime;
        QTime lastUpdateTime;
        Extractor *extractor;
        bool done;

        void setStatusLabels();

    public:
        UpdateFedAmb(Extractor *ext);
        ~UpdateFedAmb();
        void startCollecting(int timeout);

    public slots:
        void Collect();
        void Stop();
        void closeEvent(QCloseEvent *evt);
        void accept();
        void back();
        void cancel();
        void updateStatus(int step, const QString &message);
};

class RtiConnectThread : public QThread
{
    Q_OBJECT

    private:
        Extractor *extractor;
        bool shutdownRequested;
        bool noErrors;

    public:
        RtiConnectThread(Extractor *ext) : extractor(ext),
                shutdownRequested(false), noErrors(true) {}
        void run();
        void requestShutdown() { shutdownRequested = true; }
        bool connectShutdown() { return shutdownRequested; }
        bool connectCompletedWithNoErrors() { return noErrors; }

    signals:
        void status(int step, const QString &message);
};

class Extractor : public QObject
{
    Q_OBJECT

    public:
        QApplication *app;
        Welcome* welcome;
        Setup* setup;
        AdjustSubnets* adjustSubnets;
        AdjustNames* adjustNames;
        UpdateFedAmb* updateFedAmb;
        RtiConnectThread *connectThread;
        DebugWindow* dbW;
        QIcon windowIcon;

        QString exeHome;
        QString extractorHome;
        QString userHome;
        QSettings settings;
        bool showWelcome;

        SNT_HLA::FedAmb* fed;
        SNT_HLA::NodeSet* ns;

        Extractor(QApplication *application);

        int WriteFiles();
        void Startup();

    public slots:
        void openUsersGuide();
        void Extract();
        void connected();
        void cleanUp();
};

#endif
