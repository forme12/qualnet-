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
#include "Welcome.h"
#include "Setup.h"
#include "AdjustNames.h"
#include "AdjustSubnets.h"
#include "DebugWindow.h"

#include "configFiles.h"

#include <QSplashScreen>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QTimer>

#include <QtPlugin>
Q_IMPORT_PLUGIN(qjpeg)
Q_IMPORT_PLUGIN(qgif)

using namespace SNT_HLA;

typedef const char* charPtr;

void GradientFrame::paintEvent( QPaintEvent * event )
    {
        QPainter p(this);
        QLinearGradient fade(0,height(),width(),0);
        fade.setColorAt(0, QColor(0, 50, 75, 255));
        fade.setColorAt(1, QColor(100, 175, 255, 255));
        fade.setSpread(QGradient::ReflectSpread);
        p.fillRect(rect(), fade);
    }
void ShadowLabel::paintEvent( QPaintEvent * event )
{
    QStyle *style = QWidget::style();
    QPainter painter(this);
    QRect cr = contentsRect();
    int flags = QStyle::visualAlignment(layoutDirection(), QFlag(alignment()));
    if (wordWrap())
        flags = flags | Qt::TextWordWrap;
    QPalette pal = palette();
    QColor org = pal.color(QPalette::WindowText);
    pal.setColor(QPalette::WindowText, QColor(255-org.red(),255-org.green(), 255-org.blue(), 255));
    style->drawItemText(&painter, cr, flags, pal, isEnabled(), text(), foregroundRole());
    cr.moveTo(cr.x()+1, cr.y()+1);
    style->drawItemText(&painter, cr, flags, pal, isEnabled(), text(), foregroundRole());
    cr.moveTo(cr.x()-2, cr.y()-2);
    pal.setColor(QPalette::WindowText, org );
    style->drawItemText(&painter, cr, flags, pal, isEnabled(), text(), foregroundRole());
}

Extractor::Extractor(QApplication *application) : app(application), welcome(0), fed(0), ns(0), dbW(0), connectThread(0), windowIcon(":/HLA_ext_NCF.png")
{
#ifdef _WIN32
    userHome = QString(getenv("HOMEDRIVE")) + QString(getenv("HOMEPATH")) + QString("\\My Documents");
#else
    userHome = QString(getenv("HOME"));
#endif
    exeHome = SNT_HLA::Config::instance().exeHome.c_str();
    extractorHome = SNT_HLA::Config::instance().qualnetHome.c_str();
    if (getenv("NCF_HOME") != 0)
        extractorHome = getenv("NCF_HOME");
#ifdef JNE_LIB
    else if (getenv("JNE_HOME") != 0)
        extractorHome = getenv("JNE_HOME");
#elif defined EXATA 
    else if (getenv("EXATA_HOME") != 0)
        extractorHome = getenv("EXATA_HOME");
#endif
    else if (getenv("QUALNET_HOME") != 0)
        extractorHome = getenv("QUALNET_HOME");
    else if (getenv("HLA_HOME") != 0)
        extractorHome = getenv("HLA_HOME");

    if (settings.value("showWelcome").isNull())
    {
        showWelcome = true;
    }
    else
    {
        showWelcome = settings.value("showWelcome").toBool();
    }

    if (showWelcome)
    {
        welcome = new Welcome(this);
        welcome->setWindowIcon(windowIcon);
    }
    setup = new Setup(this);
    setup->setWindowIcon(windowIcon);
    if (!settings.value("defaultTimeout").isNull())
    {
        setup->timeout->setValue(settings.value("defaultTimeout").toInt());
    }

    adjustSubnets = new AdjustSubnets(this);
    adjustSubnets->setWindowIcon(windowIcon);
    adjustNames = new AdjustNames(this);
    adjustNames->setWindowIcon(windowIcon);
    updateFedAmb = new UpdateFedAmb(this);
    updateFedAmb->setWindowIcon(windowIcon);

    if (showWelcome)
    {
        connect(welcome->Cancel, SIGNAL(clicked()), this, SLOT(cleanUp()));
        connect(setup->Back, SIGNAL(clicked()), setup, SLOT(back()));

    }
    else
    {
        setup->Back->hide();
    }
    connect(setup->Cancel, SIGNAL(clicked()), this, SLOT(cleanUp()));
    connect(updateFedAmb->Cancel, SIGNAL(clicked()), this, SLOT(cleanUp()));
    connect(adjustSubnets->Cancel, SIGNAL(clicked()), this, SLOT(cleanUp()));
    connect(adjustNames->Cancel, SIGNAL(clicked()), this, SLOT(cleanUp()));
    connect(updateFedAmb, SIGNAL(canceled()), this, SLOT(cleanUp()));
}

void Extractor::Startup()
{
    QPixmap pixmap(":/splash.png");
    QSplashScreen splash(pixmap);
    splash.show();

    Config::instance().readParameterFiles();
    Config::instance().readModelFiles();
    setup->init();

    if (showWelcome)
    {
        welcome->show();
        splash.finish(welcome);
    }
    else
    {
        setup->show();
        splash.finish(setup);
    }

}

void Extractor::openUsersGuide()
{
    QString userGuideFileName = EXTRACTOR_VERSION "-UsersGuide.pdf";
    QString home(Config::instance().qualnetHome.c_str());
    QString userGuideFilePath  = home + "/documentation/" + userGuideFileName;

    userGuideFilePath = userGuideFilePath.replace('\\', '/');

    QFileInfo fInfo(userGuideFilePath);
    if (!fInfo.exists()) {
        QMessageBox::warning(NULL, "Error", QString("Cannot find ") + userGuideFileName);
        return;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    bool success = QDesktopServices::openUrl(userGuideFilePath);

    QApplication::restoreOverrideCursor();

    if (!success)
        QMessageBox::warning(NULL, "Error", QString("Cannot open ") + userGuideFilePath);

}
void Extractor::Extract()
{
    setup->hide();
    updateFedAmb->show();
    updateFedAmb->progressBar->setMaximum(0);
    updateFedAmb->progressBar->setValue(0);

    Config::instance().scenarioName = setup->scenarioName->text().toAscii().data();
    Config::instance().federationName = setup->federationName->currentText().toAscii().data();
    Config::instance().fedPath = setup->fedFileName->text().toAscii().data();
    Config::instance().initializeScenarioParameters();

    if (setup->useHla->isChecked())
    {
        Config::instance().setParameter(Parameter("HLA", "YES"));
        Config::instance().setParameter(Parameter("DIS", "NO"));
    }
    else if (setup->useDis->isChecked())
    {
        Config::instance().setParameter(Parameter("HLA", "NO"));
        Config::instance().setParameter(Parameter("DIS", "YES"));
    }

    if (fed)
        delete fed;
    fed = new SNT_HLA::FedAmb;
    if (ns)
        delete ns;
    ns = new SNT_HLA::NodeSet;

    if (!connectThread)
    {
        connectThread = new RtiConnectThread(this);
        connect(connectThread, SIGNAL(status(int, const QString &)), updateFedAmb, SLOT(updateStatus(int, const QString &)));
        connect(connectThread, SIGNAL(finished()), this, SLOT(connected()));
    }
    connectThread->start();
}

void Extractor::connected()
{
    if (connectThread && connectThread->connectShutdown())
        return;
    if (connectThread && connectThread->connectCompletedWithNoErrors())
        updateFedAmb->startCollecting(setup->timeout->value());
    else
    {
        QMessageBox::warning(NULL, "Error", QString("Failed to connect to the RTI. Please check the HLA Federation Settings."));
        updateFedAmb->hide();
        setup->show();
    }
}

void RtiConnectThread::run()
{
    shutdownRequested = false;
    noErrors = false;

    SNT_HLA::FedAmb* fed = extractor->fed;

    emit status(1, "Connecting to RTI");

    try
    {
        std::cout << "Initializing Federation\n\tfederation name " <<
            Config::instance().federationName << "\n\tFED file " << Config::instance().fedPath <<
            "\n\tfederate name " << Config::instance().federateName << std::flush;
        fed->Init(Config::instance().federationName, Config::instance().fedPath, Config::instance().federateName);
    }
    catch (RTI::Exception& e)
    {
        std::cerr << "Error initializing Federation: " << &e << std::endl;
        return;
    }

    if (shutdownRequested)
        return;

    std::cout << "Starting HLA extraction process" << std::flush;
    emit status(2, "Subscribing to Entities");

    if (shutdownRequested)
        return;

    try
    {
        fed->Subscribe(Factory<BaseEntity>::instance().getClass());
        fed->Subscribe(Factory<PhysicalEntity>::instance().getClass());
        fed->Subscribe(Factory<EmbeddedSystem>::instance().getClass());
        fed->Subscribe(Factory<RadioTransmitter>::instance().getClass());
    }
    catch (RTI::Exception& e)
    {
        std::cerr << "Error subscribing : " << &e << std::endl;
        return;
    }
    emit status(3, "Subscribing to Entities");

    if (shutdownRequested)
        return;

    try
    {
        fed->Subscribe(Factory<AggregateEntity>::instance().getClass());
    }
    catch (RTI::Exception&)
    {
        std::cout << "Error subscribing to AggreateEntities. AggeateEntities will not be available." << std::endl << std::flush;
    }
    noErrors = true;
}

int Extractor::WriteFiles()
{
    std::cout << "writing config files" << std::flush;
    Config::instance().dirSep = "/";
    QDir scenDir;
    scenDir.mkpath(setup->scenarioDirectory->text());
    QDir::setCurrent(setup->scenarioDirectory->text());
    Config::instance().scenarioDir = QDir::currentPath().toAscii().data();

    ConfigFileWriter cfWriter;
    if (setup->copyFedFile->isChecked())
    {
        cfWriter.copyFederationFile();
        QFileInfo fedFileInfo(setup->fedFileName->text());
        Config::instance().fedPath = fedFileInfo.fileName().toAscii().data();
        Config::instance().setParameter(Parameter("HLA-FED-FILE-PATH", Config::instance().fedPath));
    }

    cfWriter.writeRadiosFile(Config::instance().getParameter("HLA-RADIOS-FILE-PATH").value, *ns);
    cfWriter.writeEntitiesFile(Config::instance().getParameter("HLA-ENTITIES-FILE-PATH").value, fed );
    cfWriter.writeNetworksFile(Config::instance().getParameter("HLA-NETWORKS-FILE-PATH").value, *ns);
    cfWriter.writeNodesFile(Config::instance().getParameter("NODE-POSITION-FILE").value, *ns);
    cfWriter.writeRouterModelFile(Config::instance().getParameter("ROUTER-MODEL-CONFIG-FILE").value, *ns);
    cfWriter.writeConfigFile(Config::instance().getParameter("CONFIG-FILE-PATH").value, *ns );
    cfWriter.copyIconFiles();

    return 0;
}

void Extractor::cleanUp()
{
    bool exitApplication = true;
    settings.sync();
    if (connectThread)
    {
        if (connectThread->isRunning())
        {
            connectThread->terminate();
            connectThread->wait();
        }
        delete connectThread;
        connectThread = 0;
    }
    if (welcome)
    {
        welcome->hide();
//        delete welcome;
//        welcome = 0;
    }
    if (adjustSubnets)
    {
        adjustSubnets->hide();
        adjustSubnets->timeToDie();
//        delete adjustSubnets;
//        adjustSubnets = 0;
    }
    if (adjustNames)
    {
        adjustNames->hide();
        adjustNames->timeToDie();
//        delete adjustNames;
//        adjustNames = 0;
    }
    if (updateFedAmb)
    {
        updateFedAmb->Stop();
        updateFedAmb->hide();

//        delete updateFedAmb;
//        updateFedAmb = 0;
    }
    if (setup)
    {
        setup->hide();
//        delete setup;
//        setup = 0;
    }
    if (dbW)
    {
        if (dbW->isVisible())
        {
            QMessageBox::StandardButton answer = QMessageBox::question(dbW, "HLA Extractor", "The HLA Extractor is closing. Do you wish to close the diagnostic output window?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (answer == QMessageBox::Yes)
            {
                dbW->hide();
                delete dbW;
                dbW = 0;
            }
            else
            {
                exitApplication = false;
            }
        }
    }
    if (exitApplication)
        app->exit();
}
UpdateFedAmb::UpdateFedAmb(Extractor *ext) : QDialog(ext->setup, Qt::WindowTitleHint | Qt::WindowSystemMenuHint), extractor(ext)
{
    setupUi(this);
    timer = new QTimer(this);
    setWindowTitle("HLA Extractor Step 2 - Extracting");
    status->setText("Starting Extraction");
    setWindowModality(Qt::WindowModal);
    progressBar->setMaximum(0);
    connect(UsersGuide, SIGNAL(clicked()), extractor, SLOT(openUsersGuide()));
    connect(Next, SIGNAL(clicked()), this, SLOT(accept()));
    connect(Back, SIGNAL(clicked()), this, SLOT(back()));
}
UpdateFedAmb::~UpdateFedAmb()
{
    if (timer)
        delete timer;
}
void UpdateFedAmb::setStatusLabels()
{
    int num;
    SNT_HLA::HLAClass *cls;

    num = extractor->fed->numUpdatesReceived;
    if (num == 0 && startTime.elapsed() > 10000)
        status->setText("No entity updates received. Please check the simulator.");
    else if (num > 0 && lastUpdateTime.elapsed() > 10000)
        status->setText("No new entity updates received recently. All entity updates may be complete. Press Next to continue.");
    else if (num > 0)
        status->setText("Receiving Entity Updates");

    updatesReceived->setText(QString("%1").arg(num));

    cls = extractor->fed->findObjectClass("BaseEntity.PhysicalEntity");
    num = cls->allObjects.size();
    entitiesDiscovered->setText(QString("%1").arg(num));

    num = cls->numUpdated;
    entitiesUpdated->setText(QString("%1").arg(num));

    cls = extractor->fed->findObjectClass("EmbeddedSystem.RadioTransmitter");
    num = cls->allObjects.size();
    radiosDiscovered->setText(QString("%1").arg(num));

    num = cls->numUpdated;
    radiosUpdated->setText(QString("%1").arg(num));
}
void UpdateFedAmb::startCollecting(int timeout)
{
    progressBar->setMaximum(1000*timeout);
    startTime.start();
    status->setText("Waiting for Entitiy Updates");
    setStatusLabels();
    done = false;
    connect(timer, SIGNAL(timeout()), this, SLOT(Collect()));
    timer->start(100);
}
void UpdateFedAmb::Collect()
{
    if (!done)
    {
        QSize sz = size();
        extractor->fed->update_occurred = false;
        extractor->fed->rti->tick();
        if (extractor->fed->update_occurred)
            lastUpdateTime.start();
        progressBar->setValue(startTime.elapsed());
        setStatusLabels();
        if (progressBar->maximum() > 0 && startTime.elapsed() > progressBar->maximum())
        {
            accept();
        }
    }
}
void UpdateFedAmb::Stop()
{
    done = true;
    timer->stop();
}
void UpdateFedAmb::closeEvent(QCloseEvent *evt)
{
    cancel();
}
void showWarningMessage( QString msg )
{
    QMessageBox msgBox(QMessageBox::Warning, "Warning",
        "An EXata/QualNet scenario could not be created.",
        QMessageBox::Ok);
    msgBox.setInformativeText(msg);
    QFont fnt = msgBox.font();
    fnt.setPointSize(10);
    msgBox.setFont(fnt);
    msgBox.exec();
}
void UpdateFedAmb::accept()
{
    Stop();
    if (extractor->connectThread && extractor->connectThread->isRunning())
    {
        extractor->connectThread->terminate();
        extractor->connectThread->wait();
        delete extractor->connectThread;
        extractor->connectThread = 0;
        showWarningMessage(tr(
            "<p>The HLA Extractor could not connect to the RTI.</p>"
            "<p>You may need to increase the Extractor timeout or check your RTI configuration.</p>"
            ));
        hide();
        extractor->fed->Cleanup(Config::instance().federationName);
        extractor->setup->show();
        return;
    }

    extractor->fed->numUpdatesReceived;
    SNT_HLA::HLAClass *cls = extractor->fed->findObjectClass("BaseEntity.PhysicalEntity");
    int numEntities = cls->allObjects.size();
    int numEntitiesUpdated = cls->numUpdated;
    cls = extractor->fed->findObjectClass("EmbeddedSystem.RadioTransmitter");
    int numRadios = cls->allObjects.size();
    int numRadiosUpdated = cls->numUpdated;

    if (numEntities == 0 || numRadios == 0)
    {
        if (numEntities == 0 && numRadios == 0)
        {
            showWarningMessage(tr(
                "<p>The HLA Extractor could not find any entities or radios.</p>"
                "<p>This could be caused by:</p>"
                "<ul>"
                "<li>The other federates are not running</li>"
                "<li>The other federates do not have a scenario loaded</li>"
                "<li>The federates are not all using the same federation name.</li>"
                "<li>The other federates are not configured to use an external communication model.</li>"
                "<li>The scenario does not have any HLA Objects of type BaseEntity.PhysicalEntity or EmbeddedSystem.RadioTransmitter</li>"
                "</ul>"
                ));
        }
        else if (numEntities)
            showWarningMessage(tr(
                "<p>The HLA Extractor found some entities but could not find any radios."
                "<p>Please check the scenario loaded in the other federates and make sure that it is using"
                " HLA Objects of type EmbeddedSystem.RadioTransmitter for radios."
                ));
        else if (numRadios)
            showWarningMessage(tr(
                "<p>The HLA Extractor found some radios but could not find any entities.</p>"
                "<p>Please check the scenario loaded in the other federates and make sure that it is using"
                " HLA Objects of type BaseEntity.PhysicalEntity for entities</p>"));

        hide();
        extractor->fed->Cleanup(Config::instance().federationName);
        extractor->setup->show();
        return;
    }

    status->setText("Creating scenario");
    extractor->ns->extractNodes(extractor->fed);

    if (extractor->ns->size() == 0)
    {
        if (numEntitiesUpdated == 0 && numRadiosUpdated == 0)
        {
            showWarningMessage(tr(
                "<p>The HLA Extractor did not receive any updates for entities and radios.</p>"
                "<p>You may need to increase the Extractor timeout or check your RTI configuration.</p>"
                ));
        }
        else if (numEntitiesUpdated < numEntities || numRadiosUpdated < numRadios)
        {
            showWarningMessage(tr(
                "<p>The HLA Extractor did not receive updates for all entities and radios.</p>"
                "<p>You may need to increase the Extractor timeout or check your RTI configuration.</p>"
                ));
        }
        else
        {
            showWarningMessage(tr(
                "<p>The HLA Extractor found entities and radios, but could not associate any radios with entities.<p>"
                "<p>Please check the scenario loaded in the other federates"
                " and make sure that each HLA Object of type EmbeddedSystem.RadioTransmitter is"
                " associated with a HLA Object of type BaseEntity.PhysicalEntity.</p>"
                ));
        }
        hide();
        extractor->fed->Cleanup(Config::instance().federationName);
        extractor->setup->show();
        return;
    }
    else if (numEntitiesUpdated < numEntities || numRadiosUpdated < numRadios)
    {
        QMessageBox msgBox(QMessageBox::Warning, "Warning",
            tr("The EXata/QualNet scenario may contain errors."),
            QMessageBox::Ok);
        msgBox.setInformativeText(tr(
            "<p>The HLA Extractor did not receive updates for all entities and radios.</p>"
            "<p>You may need to increase the Extractor timeout or check your RTI configuration.</p>"
            ));
        QFont fnt = msgBox.font();
        fnt.setPointSize(10);
        msgBox.setFont(fnt);
        msgBox.exec();
    }

    hide();
    extractor->fed->Cleanup(Config::instance().federationName);
    extractor->adjustNames->show();
}
void UpdateFedAmb::back()
{
    extractor->setup->show();
    extractor->connectThread->requestShutdown();
    Stop();
    hide();
    extractor->fed->Cleanup(Config::instance().federationName);
}
void UpdateFedAmb::cancel()
{
    Stop();
    hide();
    extractor->cleanUp();
}
void UpdateFedAmb::updateStatus(int step, const QString &message)
{
    progressBar->setValue(step);
    status->setText(message);
}

int main(int argc, char *argv[])
{

    QCoreApplication::setOrganizationName("ScalableNetworkTechnologies");
    QCoreApplication::setOrganizationDomain("scalable-networks.com");
    QCoreApplication::setApplicationName(EXTRACTOR_VERSION);

    QApplication app(argc, argv);
    Config::instance().parseCommandLine(argc, (const char**)argv);
    Extractor extractor(&app);
    app.setWindowIcon(extractor.windowIcon);
    extractor.Startup();
    return app.exec();
}
