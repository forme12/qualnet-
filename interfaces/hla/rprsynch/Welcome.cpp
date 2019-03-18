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


#include "Welcome.h"
#include "Extractor.h"
#include "Setup.h"

Welcome::Welcome(Extractor *ext) : QDialog(0, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
        extractor(ext)
{
    setupUi(this);
    setSizeGripEnabled(true);
    connect(Next, SIGNAL(clicked()), this, SLOT(accept()));
    connect(UsersGuide, SIGNAL(clicked()), extractor, SLOT(openUsersGuide()));
}

void Welcome::accept()
{
    if( this->showWelcome->isChecked() )
    {
        extractor->settings.setValue("showWelcome", true);
    }
    else
    {
        extractor->settings.setValue("showWelcome", false);
    }
    hide();
    extractor->setup->show();
}

