// Copyright (c) 2001-2003 Quadralay Corporation.  All rights reserved.
//

function  WWHCommonMessages_Object()
{
  // Set default messages
  //
  WWHCommonMessages_Set_en(this);

  this.fSetByLocale = WWHCommonMessages_SetByLocale;
}

function  WWHCommonMessages_SetByLocale(ParamLocale)
{
  var  LocaleFunction = null;


  // Match locale
  //
  if ((ParamLocale.length > 1) &&
      (eval("typeof(WWHCommonMessages_Set_" + ParamLocale + ")") == "function"))
  {
    LocaleFunction = eval("WWHCommonMessages_Set_" + ParamLocale);
  }
  else if ((ParamLocale.length > 1) &&
           (eval("typeof(WWHCommonMessages_Set_" + ParamLocale.substring(0, 2) + ")") == "function"))
  {
    LocaleFunction = eval("WWHCommonMessages_Set_" + ParamLocale.substring(0, 2));
  }

  // Default already set, only override if locale found
  //
  if (LocaleFunction != null)
  {
    LocaleFunction(this);
  }
}

function  WWHCommonMessages_Set_de(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "Navigation anzeigen";
  ParamMessages.mSyncIconLabel           = "Im Inhalt anzeigen";
  ParamMessages.mPrevIconLabel           = "Zur\u00fcck";
  ParamMessages.mNextIconLabel           = "Weiter";
  ParamMessages.mRelatedTopicsIconLabel  = "Verwandte Themen";
  ParamMessages.mEmailIconLabel          = "E-Mail";
  ParamMessages.mPrintIconLabel          = "Drucken";
  ParamMessages.mBookmarkIconLabel       = "Lesezeichen";
  ParamMessages.mBookmarkLinkMessage     = "Klicken Sie mit der rechten Maustaste auf die Verkn\u00fcpfung, und f\u00fcgen Sie sie Ihren Lesezeichen hinzu.";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "Siehe auch";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "Brauchen Sie Unterst\u00fctzung f\u00fcr zugreifbare HTML?";
  ParamMessages.mBrowserNotSupported = "Ihr Webbrowser unterst\u00fctzt die zum ordnungsgem\u00e4\u00dfen Anzeigen\\ndieser Seite erforderlichen Funktionen nicht. Folgende Browser werden unterst\u00fctzt:\\n\\n IE4 und h\u00f6her f\u00fcr Windows und UNIX\\n IE5 und h\u00f6her f\u00fcr Mac\\n Netscape 6.1 und h\u00f6her f\u00fcr Windows, Mac und UNIX\\n Netscape 4.x f\u00fcr Windows, Mac und UNIX";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = ",";
  ParamMessages.mAccessibilityDocumentFrameName        = "Dokument";
  ParamMessages.mAccessibilityDisabledNavigationButton = "Deaktivierte Schaltfl\u00e4che %s";
  ParamMessages.mAccessibilityPopupClickThrough        = "Klicken Sie hier, um zum Quelldokument zu gelangen.";
}

function  WWHCommonMessages_Set_en(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "Show Navigation";
  ParamMessages.mSyncIconLabel           = "Show in Contents";
  ParamMessages.mPrevIconLabel           = "Previous";
  ParamMessages.mNextIconLabel           = "Next";
  ParamMessages.mRelatedTopicsIconLabel  = "Related Topics";
  ParamMessages.mEmailIconLabel          = "E-mail";
  ParamMessages.mPrintIconLabel          = "Print";
  ParamMessages.mBookmarkIconLabel       = "Bookmark";
  ParamMessages.mBookmarkLinkMessage     = "Right-click link and add it to your bookmarks.";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "See Also";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "Do you require accessible HTML support?";
  ParamMessages.mBrowserNotSupported = "Your web browser does not support the necessary features\\nrequired to view this page properly.  Supported browsers are:\\n\\n  IE4 and later on Windows and UNIX\\n  IE5 and later on Mac\\n  Netscape 6.1 and later on Windows, Mac, and UNIX\\n  Netscape 4.x on Windows, Mac, and UNIX";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = ",";
  ParamMessages.mAccessibilityDocumentFrameName        = "Document";
  ParamMessages.mAccessibilityDisabledNavigationButton = "Disabled button %s";
  ParamMessages.mAccessibilityPopupClickThrough        = "Click here to go to the source document.";
}

function  WWHCommonMessages_Set_es(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "Mostrar barra de navegaci\u00f3n";
  ParamMessages.mSyncIconLabel           = "Mostrar en Contenido";
  ParamMessages.mPrevIconLabel           = "Atr\u00e1s";
  ParamMessages.mNextIconLabel           = "Adelante";
  ParamMessages.mRelatedTopicsIconLabel  = "Temas relacionados";
  ParamMessages.mEmailIconLabel          = "E-mail";
  ParamMessages.mPrintIconLabel          = "Imprimir";
  ParamMessages.mBookmarkIconLabel       = "Marcador";
  ParamMessages.mBookmarkLinkMessage     = "Haga clic con el bot\u00f3n derecho del mouse en el v\u00ednculo para agregarlo a la lista de favoritos.";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "Consulte tambi\u00e9n";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "\u00bfrequiere asistencia accesible a trav\u00e9s de HTML?";
  ParamMessages.mBrowserNotSupported = "Su explorador de Internet no es compatible con las funciones\\nnecesarias para ver esta p\u00e1gina correctamente. Los exploradores compatibles son:\\n\\n IE4 y posteriores para Windows y UNIX\\n IE5 y posteriores para Mac\\n Netscape 6.1 y posteriores para Windows, Mac y UNIX\\n Netscape 4.x para Windows, Mac y UNIX";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = ",";
  ParamMessages.mAccessibilityDocumentFrameName        = "Documento";
  ParamMessages.mAccessibilityDisabledNavigationButton = "Bot\u00f3n desactivado %s";
  ParamMessages.mAccessibilityPopupClickThrough        = "Haga clic aqu\u00ed para ir al documento original.";
}

function  WWHCommonMessages_Set_fr(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "Navigation";
  ParamMessages.mSyncIconLabel           = "Afficher dans la table des mati\u00e8res";
  ParamMessages.mPrevIconLabel           = "Pr\u00e9c\u00e9dent";
  ParamMessages.mNextIconLabel           = "Suivant";
  ParamMessages.mRelatedTopicsIconLabel  = "Rubriques associ\u00e9es";
  ParamMessages.mEmailIconLabel          = "Courrier \u00e9lectronique";
  ParamMessages.mPrintIconLabel          = "Imprimer";
  ParamMessages.mBookmarkIconLabel       = "Ajouter aux Favoris";
  ParamMessages.mBookmarkLinkMessage     = "Cliquez sur ce lien avec le bouton droit de la souris et ajoutez-le \u00e0 vos Favoris.";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "Voir aussi";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "Avez-vous besoin de support pour le HTML accessible\u00a0?";
  ParamMessages.mBrowserNotSupported = "Votre navigateur Web ne prend pas en charge les fonctions\\nrequises pour visualiser cette page de mani\u00e8re correcte. Les navigateurs pris en charge sont\u00a0:\\n\\n IE4 et version ult\u00e9rieure sous Windows et UNIX\\n IE5 et version ult\u00e9rieure sur Mac\\n Netscape 6.1 et version ult\u00e9rieure sous Windows, Mac et UNIX\\n Netscape 4.x sous Windows, Mac et UNIX";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = ",";
  ParamMessages.mAccessibilityDocumentFrameName        = "Document";
  ParamMessages.mAccessibilityDisabledNavigationButton = "Bouton d\u00e9sactiv\u00e9 %s";
  ParamMessages.mAccessibilityPopupClickThrough        = "Cliquez ici pour atteindre le document source.";
}

function  WWHCommonMessages_Set_it(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "Mostra navigazione";
  ParamMessages.mSyncIconLabel           = "Mostra in Contenuto";
  ParamMessages.mPrevIconLabel           = "Precedente";
  ParamMessages.mNextIconLabel           = "Avanti";
  ParamMessages.mRelatedTopicsIconLabel  = "Argomenti correlati";
  ParamMessages.mEmailIconLabel          = "E-mail";
  ParamMessages.mPrintIconLabel          = "Stampa";
  ParamMessages.mBookmarkIconLabel       = "Segnalibro";
  ParamMessages.mBookmarkLinkMessage     = "Fare clic con il tasto destro del mouse per aggiungere ai Segnalibri.";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "Vedere anche";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "\u00c8 necessario supporto HTML accessibile?";
  ParamMessages.mBrowserNotSupported = "Il browser Web in uso non supporta le funzioni necessarie\\nper visualizzare correttamente questa pagina. I browser supportati sono:\\n\\n IE4 e versioni successive su Windows e UNIX\\n IE5 e versioni successive su Mac\\n Netscape 6.1 e versioni successive per Windows, Mac e UNIX\\n Netscape 4.x su Windows, Mac e UNIX";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = ",";
  ParamMessages.mAccessibilityDocumentFrameName        = "Documento";
  ParamMessages.mAccessibilityDisabledNavigationButton = "Pulsante disabilitato %s";
  ParamMessages.mAccessibilityPopupClickThrough        = "Fare clic qui per andare al documento di origine.";
}

function  WWHCommonMessages_Set_ja(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "??????? ?????";
  ParamMessages.mSyncIconLabel           = "?????";
  ParamMessages.mPrevIconLabel           = "??";
  ParamMessages.mNextIconLabel           = "??";
  ParamMessages.mRelatedTopicsIconLabel  = "??????";
  ParamMessages.mEmailIconLabel          = "?????";
  ParamMessages.mPrintIconLabel          = "??";
  ParamMessages.mBookmarkIconLabel       = "??????";
  ParamMessages.mBookmarkLinkMessage     = "?????????????????????????";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "JA ????";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "???????????????? HTML ?????????????";
  ParamMessages.mBrowserNotSupported = "??????????????????????????\\n????????????????? ?????????????: \\n\\nWindows ??? UNIX ??????? IE4 ??\\nMac ??????? IE5 ??\\nWindows?Mac?UNIX ??????? Netscape 6.1 ??\\nWindows?Mac?UNIX ??????? Netscape 4.x";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = ",";
  ParamMessages.mAccessibilityDocumentFrameName        = "??????";
  ParamMessages.mAccessibilityDisabledNavigationButton = "??? %s ????????";
  ParamMessages.mAccessibilityPopupClickThrough        = "??? ?????????????????????????";
}

function  WWHCommonMessages_Set_ko(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "????? ??";
  ParamMessages.mSyncIconLabel           = "????? ??";
  ParamMessages.mPrevIconLabel           = "??";
  ParamMessages.mNextIconLabel           = "??";
  ParamMessages.mRelatedTopicsIconLabel  = "?? ??";
  ParamMessages.mEmailIconLabel          = "??";
  ParamMessages.mPrintIconLabel          = "??";
  ParamMessages.mBookmarkIconLabel       = "???";
  ParamMessages.mBookmarkLinkMessage     = "??? ??? ??? ??? ???? ???? ?????.";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "??";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "HTML ?? ??? ??????";
  ParamMessages.mBrowserNotSupported = "? ????? ? ???? ???? ???? ? \\n??? ??? ???? ????. ???? ????:\\n\\nInternet Explorer 4 ??(Windows, UNIX )\\n Internet Explorer 5 ??(Mac)\\n Netscape 6.1 ??(Windows, Mac, UNIX)\\n Netscape 4.x(Windows, Mac, UNIX)";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = "KO ,";
  ParamMessages.mAccessibilityDocumentFrameName        = "??";
  ParamMessages.mAccessibilityDisabledNavigationButton = "????? ?? %s";
  ParamMessages.mAccessibilityPopupClickThrough        = "?? ??? ????? ??? ??????.";
}

function  WWHCommonMessages_Set_pt(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "Mostrar navega\u00e7\u00e3o";
  ParamMessages.mSyncIconLabel           = "Mostrar em conte\u00fado";
  ParamMessages.mPrevIconLabel           = "Voltar";
  ParamMessages.mNextIconLabel           = "Avan\u00e7ar";
  ParamMessages.mRelatedTopicsIconLabel  = "T\u00f3picos relacionados";
  ParamMessages.mEmailIconLabel          = "E-mail";
  ParamMessages.mPrintIconLabel          = "Imprimir";
  ParamMessages.mBookmarkIconLabel       = "Favoritos";
  ParamMessages.mBookmarkLinkMessage     = "Clique com o bot\u00e3o direito no link para adicion\u00e1-lo aos seus Favoritos.";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "Ver tamb\u00e9m";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "Precisa de suporte HTML acess\u00edvel?";
  ParamMessages.mBrowserNotSupported = "Seu navegador da web n\u00e3o suporta os recursos\\nnecess\u00e1rios \u00e0 visualiza\u00e7\u00e3o desta p\u00e1gina. Os navegadores suportados s\u00e3o:\\n\\n IE4 e vers\u00e3o posterior para Windows e UNIX\\n IE5 e vers\u00e3o posterior para Mac\\n Netscape 6.1 e vers\u00e3o posterior para Windows, Mac e UNIX\\n Netscape 4.x para Windows, Mac e UNIX";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = ",";
  ParamMessages.mAccessibilityDocumentFrameName        = "Documento";
  ParamMessages.mAccessibilityDisabledNavigationButton = "Bot\u00e3o %s desativado";
  ParamMessages.mAccessibilityPopupClickThrough        = "Clique aqui para ver o documento de origem.";
}

function  WWHCommonMessages_Set_sv(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "Visa navigering";
  ParamMessages.mSyncIconLabel           = "Visa i inneh\u00e5ll";
  ParamMessages.mPrevIconLabel           = "F\u00f6reg\u00e5ende";
  ParamMessages.mNextIconLabel           = "N\u00e4sta";
  ParamMessages.mRelatedTopicsIconLabel  = "N\u00e4rliggande information";
  ParamMessages.mEmailIconLabel          = "E-post";
  ParamMessages.mPrintIconLabel          = "Skriv ut";
  ParamMessages.mBookmarkIconLabel       = "Bokm\u00e4rke";
  ParamMessages.mBookmarkLinkMessage     = "H\u00f6gerklicka p\u00e5 l\u00e4nken om du vill l\u00e4gga till den till dina bokm\u00e4rken.";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "Se \u00e4ven";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "Beh\u00f6ver du funktioner f\u00f6r Accessible HTML?";
  ParamMessages.mBrowserNotSupported = "Webbl\u00e4saren inneh\u00e5ller inte de funktioner som kr\u00e4vs f\u00f6r\\natt visa sidan p\u00e5 r\u00e4tt s\u00e4tt. Webbl\u00e4sare som kan anv\u00e4ndas \u00e4r:\\n\\n Internet Explorer 4 eller senare i Windows och UNIX\\n Internet Explorer 5 eller senare i Mac OS\\n Netscape 6.1 eller senare i Windows, Mac OS och UNIX\\n Netscape 4.x i Windows, Mac OS och UNIX";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = ",";
  ParamMessages.mAccessibilityDocumentFrameName        = "Dokument";
  ParamMessages.mAccessibilityDisabledNavigationButton = "Avaktiverad knapp %s";
  ParamMessages.mAccessibilityPopupClickThrough        = "Klicka h\u00e4r om du vill visa k\u00e4lldokumentet.";
}

function  WWHCommonMessages_Set_zh(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "????";
  ParamMessages.mSyncIconLabel           = "??????";
  ParamMessages.mPrevIconLabel           = "???";
  ParamMessages.mNextIconLabel           = "???";
  ParamMessages.mRelatedTopicsIconLabel  = "????";
  ParamMessages.mEmailIconLabel          = "????";
  ParamMessages.mPrintIconLabel          = "??";
  ParamMessages.mBookmarkIconLabel       = "??";
  ParamMessages.mBookmarkLinkMessage     = "????????????????";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "????";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "?????????? HTML ???";
  ParamMessages.mBrowserNotSupported = "?? Web ???????????????\\n????? ????????\\n\\nIE4 ??????Windows ? UNIX?\\nIE5 ??????Mac?\\nNetscape 6.1 ??????Windows?Mac ? UNIX?\\nNetscape 4.x?Windows?Mac ? UNIX?";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = ",";
  ParamMessages.mAccessibilityDocumentFrameName        = "??";
  ParamMessages.mAccessibilityDisabledNavigationButton = "???? %s";
  ParamMessages.mAccessibilityPopupClickThrough        = "??????????";
}

function  WWHCommonMessages_Set_zh_tw(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "????";
  ParamMessages.mSyncIconLabel           = "??????";
  ParamMessages.mPrevIconLabel           = "???";
  ParamMessages.mNextIconLabel           = "???";
  ParamMessages.mRelatedTopicsIconLabel  = "????";
  ParamMessages.mEmailIconLabel          = "????";
  ParamMessages.mPrintIconLabel          = "??";
  ParamMessages.mBookmarkIconLabel       = "??";
  ParamMessages.mBookmarkLinkMessage     = "?????????????????";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "????";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "????? HTML ???????";
  ParamMessages.mBrowserNotSupported = "Web ?????????????????? ?????????\\n\\n Windows ? UNIX ??? IE 4 ?????\\n Mac ??? IE5 ?????\\n Windows?Mac ? UNIX ??? Netscape 6.1\\n Windows?Mac ? UNIX ??? Netscape 4.x";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = "?";
  ParamMessages.mAccessibilityDocumentFrameName        = "??";
  ParamMessages.mAccessibilityDisabledNavigationButton = "????? %s";
  ParamMessages.mAccessibilityPopupClickThrough        = "?????????????";
}

function  WWHCommonMessages_Set_ru(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "???????? ?????? ?????????";
  ParamMessages.mSyncIconLabel           = "???????? ? ??????????";
  ParamMessages.mPrevIconLabel           = "?????";
  ParamMessages.mNextIconLabel           = "?????";
  ParamMessages.mRelatedTopicsIconLabel  = "?????????????? ??????????";
  ParamMessages.mEmailIconLabel          = "??????????? ?????";
  ParamMessages.mPrintIconLabel          = "??????";
  ParamMessages.mBookmarkIconLabel       = "????????";
  ParamMessages.mBookmarkLinkMessage     = "???????? ?? ?????? ?????? ??????? ???? ? ???????? ?? ? ?????????.";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "??. ?????";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "????????? ?? ?????? ? ???????? ????????? HTML?";
  ParamMessages.mBrowserNotSupported = "???????????? web-??????? ?? ???????????? ???????,\\n??????????? ??? ??????????? ????????? ???? ????????.  ??? ??????? ?????????????? ?????????? ??????????:\\n\\n  IE4 ? ????? ??????? ?????? ??? Windows ? UNIX\\n  IE5 ? ????? ??????? ?????? ??? Mac\\n  Netscape 6.1 ? ????? ??????? ?????? ??? Windows, Mac ? UNIX\\n  Netscape 4.x ??? Windows, Mac ? UNIX";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = ",";
  ParamMessages.mAccessibilityDocumentFrameName        = "????????";
  ParamMessages.mAccessibilityDisabledNavigationButton = "????????? ?????? %s";
  ParamMessages.mAccessibilityPopupClickThrough        = "???????? ????? ??? ???????? ? ????????? ?????????.";
}
