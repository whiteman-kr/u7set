!win32:VERSION = 15.0.0

TEMPLATE = lib
CONFIG += qt warn_off thread exceptions hide_symbols staticlib

TARGET = QScintilla

INCLUDEPATH += . ./include ./lexlib ./src ./Qt4Qt5
!CONFIG(staticlib) {
    DEFINES += QSCINTILLA_MAKE_DLL
}
DEFINES += SCINTILLA_QT SCI_LEXER

greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets printsupport

    greaterThan(QT_MINOR_VERSION, 1) {
	    macx:QT += macextras
    }

    # Work around QTBUG-39300.
    CONFIG -= android_install
}

msvc {
	QMAKE_CXXFLAGS += /wd4996
}

# DESTDIR
#
win32 {
	CONFIG(debug, debug|release): DESTDIR = ../bin/debug
	CONFIG(release, debug|release): DESTDIR = ../bin/release
}
unix {
	CONFIG(debug, debug|release): DESTDIR = ../bin_unix/debug
	CONFIG(release, debug|release): DESTDIR = ../bin_unix/release
}

HEADERS = \
   $$PWD/include/ILexer.h \
   $$PWD/include/ILoader.h \
   $$PWD/include/Platform.h \
   $$PWD/include/Sci_Position.h \
   $$PWD/include/SciLexer.h \
   $$PWD/include/Scintilla.h \
   $$PWD/include/ScintillaWidget.h \
   $$PWD/lexlib/Accessor.h \
   $$PWD/lexlib/CharacterCategory.h \
   $$PWD/lexlib/CharacterSet.h \
   $$PWD/lexlib/DefaultLexer.h \
   $$PWD/lexlib/LexAccessor.h \
   $$PWD/lexlib/LexerBase.h \
   $$PWD/lexlib/LexerModule.h \
   $$PWD/lexlib/LexerNoExceptions.h \
   $$PWD/lexlib/LexerSimple.h \
   $$PWD/lexlib/OptionSet.h \
   $$PWD/lexlib/PropSetSimple.h \
   $$PWD/lexlib/SparseState.h \
   $$PWD/lexlib/StringCopy.h \
   $$PWD/lexlib/StyleContext.h \
   $$PWD/lexlib/SubStyles.h \
   $$PWD/lexlib/WordList.h \
   $$PWD/Qt4Qt5/Qsci/qsciabstractapis.h \
   $$PWD/Qt4Qt5/Qsci/qsciapis.h \
   $$PWD/Qt4Qt5/Qsci/qscicommand.h \
   $$PWD/Qt4Qt5/Qsci/qscicommandset.h \
   $$PWD/Qt4Qt5/Qsci/qscidocument.h \
   $$PWD/Qt4Qt5/Qsci/qsciglobal.h \
   $$PWD/Qt4Qt5/Qsci/qscilexer.h \
   $$PWD/Qt4Qt5/Qsci/qscilexeravs.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerbash.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerbatch.h \
   $$PWD/Qt4Qt5/Qsci/qscilexercmake.h \
   $$PWD/Qt4Qt5/Qsci/qscilexercoffeescript.h \
   $$PWD/Qt4Qt5/Qsci/qscilexercpp.h \
   $$PWD/Qt4Qt5/Qsci/qscilexercsharp.h \
   $$PWD/Qt4Qt5/Qsci/qscilexercss.h \
   $$PWD/Qt4Qt5/Qsci/qscilexercustom.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerd.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerdiff.h \
   $$PWD/Qt4Qt5/Qsci/qscilexeredifact.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerfortran.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerfortran77.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerhtml.h \
   $$PWD/Qt4Qt5/Qsci/qscilexeridl.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerjava.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerjavascript.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerjson.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerlua.h \
   $$PWD/Qt4Qt5/Qsci/qscilexermakefile.h \
   $$PWD/Qt4Qt5/Qsci/qscilexermarkdown.h \
   $$PWD/Qt4Qt5/Qsci/qscilexermatlab.h \
   $$PWD/Qt4Qt5/Qsci/qscilexeroctave.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerpascal.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerperl.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerpo.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerpostscript.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerpov.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerproperties.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerpython.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerruby.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerspice.h \
   $$PWD/Qt4Qt5/Qsci/qscilexersql.h \
   $$PWD/Qt4Qt5/Qsci/qscilexertcl.h \
   $$PWD/Qt4Qt5/Qsci/qscilexertex.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerverilog.h \
   $$PWD/Qt4Qt5/Qsci/qscilexervhdl.h \
   $$PWD/Qt4Qt5/Qsci/qscilexerxml.h \
   $$PWD/Qt4Qt5/Qsci/qscilexeryaml.h \
   $$PWD/Qt4Qt5/Qsci/qscimacro.h \
   $$PWD/Qt4Qt5/Qsci/qsciprinter.h \
   $$PWD/Qt4Qt5/Qsci/qsciscintilla.h \
   $$PWD/Qt4Qt5/Qsci/qsciscintillabase.h \
   $$PWD/Qt4Qt5/Qsci/qscistyle.h \
   $$PWD/Qt4Qt5/Qsci/qscistyledtext.h \
   $$PWD/Qt4Qt5/ListBoxQt.h \
   $$PWD/Qt4Qt5/SciAccessibility.h \
   $$PWD/Qt4Qt5/SciClasses.h \
   $$PWD/Qt4Qt5/ScintillaQt.h \
   $$PWD/src/AutoComplete.h \
   $$PWD/src/CallTip.h \
   $$PWD/src/CaseConvert.h \
   $$PWD/src/CaseFolder.h \
   $$PWD/src/Catalogue.h \
   $$PWD/src/CellBuffer.h \
   $$PWD/src/CharClassify.h \
   $$PWD/src/ContractionState.h \
   $$PWD/src/DBCS.h \
   $$PWD/src/Decoration.h \
   $$PWD/src/Document.h \
   $$PWD/src/EditModel.h \
   $$PWD/src/Editor.h \
   $$PWD/src/EditView.h \
   $$PWD/src/ElapsedPeriod.h \
   $$PWD/src/ExternalLexer.h \
   $$PWD/src/FontQuality.h \
   $$PWD/src/Indicator.h \
   $$PWD/src/IntegerRectangle.h \
   $$PWD/src/KeyMap.h \
   $$PWD/src/LineMarker.h \
   $$PWD/src/MarginView.h \
   $$PWD/src/Partitioning.h \
   $$PWD/src/PerLine.h \
   $$PWD/src/Position.h \
   $$PWD/src/PositionCache.h \
   $$PWD/src/RESearch.h \
   $$PWD/src/RunStyles.h \
   $$PWD/src/ScintillaBase.h \
   $$PWD/src/Selection.h \
   $$PWD/src/SparseVector.h \
   $$PWD/src/SplitVector.h \
   $$PWD/src/Style.h \
   $$PWD/src/UniConversion.h \
   $$PWD/src/UniqueString.h \
   $$PWD/src/ViewStyle.h \
   $$PWD/src/XPM.h

SOURCES = \
   $$PWD/lexers/LexA68k.cpp \
   $$PWD/lexers/LexAbaqus.cpp \
   $$PWD/lexers/LexAda.cpp \
   $$PWD/lexers/LexAPDL.cpp \
   $$PWD/lexers/LexAsm.cpp \
   $$PWD/lexers/LexAsn1.cpp \
   $$PWD/lexers/LexASY.cpp \
   $$PWD/lexers/LexAU3.cpp \
   $$PWD/lexers/LexAVE.cpp \
   $$PWD/lexers/LexAVS.cpp \
   $$PWD/lexers/LexBaan.cpp \
   $$PWD/lexers/LexBash.cpp \
   $$PWD/lexers/LexBasic.cpp \
   $$PWD/lexers/LexBatch.cpp \
   $$PWD/lexers/LexBibTeX.cpp \
   $$PWD/lexers/LexBullant.cpp \
   $$PWD/lexers/LexCaml.cpp \
   $$PWD/lexers/LexCLW.cpp \
   $$PWD/lexers/LexCmake.cpp \
   $$PWD/lexers/LexCOBOL.cpp \
   $$PWD/lexers/LexCoffeeScript.cpp \
   $$PWD/lexers/LexConf.cpp \
   $$PWD/lexers/LexCPP.cpp \
   $$PWD/lexers/LexCrontab.cpp \
   $$PWD/lexers/LexCsound.cpp \
   $$PWD/lexers/LexCSS.cpp \
   $$PWD/lexers/LexD.cpp \
   $$PWD/lexers/LexDiff.cpp \
   $$PWD/lexers/LexDMAP.cpp \
   $$PWD/lexers/LexDMIS.cpp \
   $$PWD/lexers/LexECL.cpp \
   $$PWD/lexers/LexEDIFACT.cpp \
   $$PWD/lexers/LexEiffel.cpp \
   $$PWD/lexers/LexErlang.cpp \
   $$PWD/lexers/LexErrorList.cpp \
   $$PWD/lexers/LexEScript.cpp \
   $$PWD/lexers/LexFlagship.cpp \
   $$PWD/lexers/LexForth.cpp \
   $$PWD/lexers/LexFortran.cpp \
   $$PWD/lexers/LexGAP.cpp \
   $$PWD/lexers/LexGui4Cli.cpp \
   $$PWD/lexers/LexHaskell.cpp \
   $$PWD/lexers/LexHex.cpp \
   $$PWD/lexers/LexHTML.cpp \
   $$PWD/lexers/LexIndent.cpp \
   $$PWD/lexers/LexInno.cpp \
   $$PWD/lexers/LexJSON.cpp \
   $$PWD/lexers/LexKix.cpp \
   $$PWD/lexers/LexKVIrc.cpp \
   $$PWD/lexers/LexLaTeX.cpp \
   $$PWD/lexers/LexLisp.cpp \
   $$PWD/lexers/LexLout.cpp \
   $$PWD/lexers/LexLPeg.cpp \
   $$PWD/lexers/LexLua.cpp \
   $$PWD/lexers/LexMagik.cpp \
   $$PWD/lexers/LexMake.cpp \
   $$PWD/lexers/LexMarkdown.cpp \
   $$PWD/lexers/LexMatlab.cpp \
   $$PWD/lexers/LexMaxima.cpp \
   $$PWD/lexers/LexMetapost.cpp \
   $$PWD/lexers/LexMMIXAL.cpp \
   $$PWD/lexers/LexModula.cpp \
   $$PWD/lexers/LexMPT.cpp \
   $$PWD/lexers/LexMSSQL.cpp \
   $$PWD/lexers/LexMySQL.cpp \
   $$PWD/lexers/LexNimrod.cpp \
   $$PWD/lexers/LexNsis.cpp \
   $$PWD/lexers/LexNull.cpp \
   $$PWD/lexers/LexOpal.cpp \
   $$PWD/lexers/LexOScript.cpp \
   $$PWD/lexers/LexPascal.cpp \
   $$PWD/lexers/LexPB.cpp \
   $$PWD/lexers/LexPerl.cpp \
   $$PWD/lexers/LexPLM.cpp \
   $$PWD/lexers/LexPO.cpp \
   $$PWD/lexers/LexPOV.cpp \
   $$PWD/lexers/LexPowerPro.cpp \
   $$PWD/lexers/LexPowerShell.cpp \
   $$PWD/lexers/LexProgress.cpp \
   $$PWD/lexers/LexProps.cpp \
   $$PWD/lexers/LexPS.cpp \
   $$PWD/lexers/LexPython.cpp \
   $$PWD/lexers/LexR.cpp \
   $$PWD/lexers/LexRebol.cpp \
   $$PWD/lexers/LexRegistry.cpp \
   $$PWD/lexers/LexRuby.cpp \
   $$PWD/lexers/LexRust.cpp \
   $$PWD/lexers/LexSAS.cpp \
   $$PWD/lexers/LexScriptol.cpp \
   $$PWD/lexers/LexSmalltalk.cpp \
   $$PWD/lexers/LexSML.cpp \
   $$PWD/lexers/LexSorcus.cpp \
   $$PWD/lexers/LexSpecman.cpp \
   $$PWD/lexers/LexSpice.cpp \
   $$PWD/lexers/LexSQL.cpp \
   $$PWD/lexers/LexStata.cpp \
   $$PWD/lexers/LexSTTXT.cpp \
   $$PWD/lexers/LexTACL.cpp \
   $$PWD/lexers/LexTADS3.cpp \
   $$PWD/lexers/LexTAL.cpp \
   $$PWD/lexers/LexTCL.cpp \
   $$PWD/lexers/LexTCMD.cpp \
   $$PWD/lexers/LexTeX.cpp \
   $$PWD/lexers/LexTxt2tags.cpp \
   $$PWD/lexers/LexVB.cpp \
   $$PWD/lexers/LexVerilog.cpp \
   $$PWD/lexers/LexVHDL.cpp \
   $$PWD/lexers/LexVisualProlog.cpp \
   $$PWD/lexers/LexYAML.cpp \
   $$PWD/lexlib/Accessor.cpp \
   $$PWD/lexlib/CharacterCategory.cpp \
   $$PWD/lexlib/CharacterSet.cpp \
   $$PWD/lexlib/DefaultLexer.cpp \
   $$PWD/lexlib/LexerBase.cpp \
   $$PWD/lexlib/LexerModule.cpp \
   $$PWD/lexlib/LexerNoExceptions.cpp \
   $$PWD/lexlib/LexerSimple.cpp \
   $$PWD/lexlib/PropSetSimple.cpp \
   $$PWD/lexlib/StyleContext.cpp \
   $$PWD/lexlib/WordList.cpp \
   $$PWD/Qt4Qt5/InputMethod.cpp \
   $$PWD/Qt4Qt5/ListBoxQt.cpp \
   $$PWD/Qt4Qt5/MacPasteboardMime.cpp \
   $$PWD/Qt4Qt5/PlatQt.cpp \
   $$PWD/Qt4Qt5/qsciabstractapis.cpp \
   $$PWD/Qt4Qt5/qsciapis.cpp \
   $$PWD/Qt4Qt5/qscicommand.cpp \
   $$PWD/Qt4Qt5/qscicommandset.cpp \
   $$PWD/Qt4Qt5/qscidocument.cpp \
   $$PWD/Qt4Qt5/qscilexer.cpp \
   $$PWD/Qt4Qt5/qscilexeravs.cpp \
   $$PWD/Qt4Qt5/qscilexerbash.cpp \
   $$PWD/Qt4Qt5/qscilexerbatch.cpp \
   $$PWD/Qt4Qt5/qscilexercmake.cpp \
   $$PWD/Qt4Qt5/qscilexercoffeescript.cpp \
   $$PWD/Qt4Qt5/qscilexercpp.cpp \
   $$PWD/Qt4Qt5/qscilexercsharp.cpp \
   $$PWD/Qt4Qt5/qscilexercss.cpp \
   $$PWD/Qt4Qt5/qscilexercustom.cpp \
   $$PWD/Qt4Qt5/qscilexerd.cpp \
   $$PWD/Qt4Qt5/qscilexerdiff.cpp \
   $$PWD/Qt4Qt5/qscilexeredifact.cpp \
   $$PWD/Qt4Qt5/qscilexerfortran.cpp \
   $$PWD/Qt4Qt5/qscilexerfortran77.cpp \
   $$PWD/Qt4Qt5/qscilexerhtml.cpp \
   $$PWD/Qt4Qt5/qscilexeridl.cpp \
   $$PWD/Qt4Qt5/qscilexerjava.cpp \
   $$PWD/Qt4Qt5/qscilexerjavascript.cpp \
   $$PWD/Qt4Qt5/qscilexerjson.cpp \
   $$PWD/Qt4Qt5/qscilexerlua.cpp \
   $$PWD/Qt4Qt5/qscilexermakefile.cpp \
   $$PWD/Qt4Qt5/qscilexermarkdown.cpp \
   $$PWD/Qt4Qt5/qscilexermatlab.cpp \
   $$PWD/Qt4Qt5/qscilexeroctave.cpp \
   $$PWD/Qt4Qt5/qscilexerpascal.cpp \
   $$PWD/Qt4Qt5/qscilexerperl.cpp \
   $$PWD/Qt4Qt5/qscilexerpo.cpp \
   $$PWD/Qt4Qt5/qscilexerpostscript.cpp \
   $$PWD/Qt4Qt5/qscilexerpov.cpp \
   $$PWD/Qt4Qt5/qscilexerproperties.cpp \
   $$PWD/Qt4Qt5/qscilexerpython.cpp \
   $$PWD/Qt4Qt5/qscilexerruby.cpp \
   $$PWD/Qt4Qt5/qscilexerspice.cpp \
   $$PWD/Qt4Qt5/qscilexersql.cpp \
   $$PWD/Qt4Qt5/qscilexertcl.cpp \
   $$PWD/Qt4Qt5/qscilexertex.cpp \
   $$PWD/Qt4Qt5/qscilexerverilog.cpp \
   $$PWD/Qt4Qt5/qscilexervhdl.cpp \
   $$PWD/Qt4Qt5/qscilexerxml.cpp \
   $$PWD/Qt4Qt5/qscilexeryaml.cpp \
   $$PWD/Qt4Qt5/qscimacro.cpp \
   $$PWD/Qt4Qt5/qsciprinter.cpp \
   $$PWD/Qt4Qt5/qsciscintilla.cpp \
   $$PWD/Qt4Qt5/qsciscintillabase.cpp \
   $$PWD/Qt4Qt5/qscistyle.cpp \
   $$PWD/Qt4Qt5/qscistyledtext.cpp \
   $$PWD/Qt4Qt5/SciAccessibility.cpp \
   $$PWD/Qt4Qt5/SciClasses.cpp \
   $$PWD/Qt4Qt5/ScintillaQt.cpp \
   $$PWD/src/AutoComplete.cpp \
   $$PWD/src/CallTip.cpp \
   $$PWD/src/CaseConvert.cpp \
   $$PWD/src/CaseFolder.cpp \
   $$PWD/src/Catalogue.cpp \
   $$PWD/src/CellBuffer.cpp \
   $$PWD/src/CharClassify.cpp \
   $$PWD/src/ContractionState.cpp \
   $$PWD/src/DBCS.cpp \
   $$PWD/src/Decoration.cpp \
   $$PWD/src/Document.cpp \
   $$PWD/src/EditModel.cpp \
   $$PWD/src/Editor.cpp \
   $$PWD/src/EditView.cpp \
   $$PWD/src/ExternalLexer.cpp \
   $$PWD/src/Indicator.cpp \
   $$PWD/src/KeyMap.cpp \
   $$PWD/src/LineMarker.cpp \
   $$PWD/src/MarginView.cpp \
   $$PWD/src/PerLine.cpp \
   $$PWD/src/PositionCache.cpp \
   $$PWD/src/RESearch.cpp \
   $$PWD/src/RunStyles.cpp \
   $$PWD/src/ScintillaBase.cpp \
   $$PWD/src/Selection.cpp \
   $$PWD/src/Style.cpp \
   $$PWD/src/UniConversion.cpp \
   $$PWD/src/ViewStyle.cpp \
   $$PWD/src/XPM.cpp

INCLUDEPATH = \
    $$PWD/include \
    $$PWD/lexlib \
    $$PWD/Qt4Qt5 \
    $$PWD/Qt4Qt5/Qsci \
    $$PWD/src

#DEFINES = 

