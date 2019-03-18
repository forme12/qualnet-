// Copyright (c) 2001-2003 Quadralay Corporation.  All rights reserved.
//

function  WWHBookGroups_Books(ParamTop)
{
  var  BookGrouping1;


  ParamTop.fAddDirectory("Users%20Guide", null, null, null, null);
  ParamTop.fAddDirectory("Statistics%20Database%20Users%20Guide", null, null, null, null);
  BookGrouping1 = ParamTop.fAddGrouping("Model Libraries", null, null, null);
    BookGrouping1.fAddDirectory("Model%20Library%20Index", null, null, null, null);
    BookGrouping1.fAddDirectory("Advanced%20Wireless", null, null, null, null);
    BookGrouping1.fAddDirectory("Cellular", null, null, null, null);
    BookGrouping1.fAddDirectory("Developer", null, null, null, null);
    BookGrouping1.fAddDirectory("LTE", null, null, null, null);
    BookGrouping1.fAddDirectory("Multimedia%20and%20Enterprise", null, null, null, null);
    BookGrouping1.fAddDirectory("Sensor%20Networks", null, null, null, null);
    BookGrouping1.fAddDirectory("Standard%20Interfaces", null, null, null, null);
    BookGrouping1.fAddDirectory("TIREM", null, null, null, null);
    BookGrouping1.fAddDirectory("UMTS", null, null, null, null);
    BookGrouping1.fAddDirectory("Urban%20Propagation", null, null, null, null);
    BookGrouping1.fAddDirectory("Wireless", null, null, null, null);
  ParamTop.fAddDirectory("Programmers%20Guide", null, null, null, null);
  ParamTop.fAddDirectory("Installation%20Guide", null, null, null, null);
  ParamTop.fAddDirectory("Distributed%20Reference%20Guide", null, null, null, null);
  ParamTop.fAddDirectory("Product%20Tour", null, null, null, null);
}

function  WWHBookGroups_ShowBooks()
{
  return true;
}

function  WWHBookGroups_ExpandAllAtTop()
{
  return false;
}
