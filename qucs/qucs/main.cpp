/***************************************************************************
                                 main.cpp
                                ----------
    begin                : Thu Aug 28 2003
    copyright            : (C) 2003 by Michael Margraf
    email                : michael.margraf@alumni.tu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*!
 * \file main.cpp
 * \brief Implementation of the main application.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <ctype.h>
#include <locale.h>

#include <QApplication>
#include <QString>
#include <QStringList>
#include <QTextCodec>
#include <QTranslator>
#include <QFile>
#include <QMessageBox>
#include <QRegExp>
#include <QtSvg>

#include "qucs.h"
#include "main.h"
#include "node.h"

#include "schematic.h"
#include "module.h"

#ifdef _WIN32
#include <Windows.h>  //for OutputDebugString
#endif

tQucsSettings QucsSettings;

QucsApp *QucsMain = 0;  // the Qucs application itself
QString lastDir;    // to remember last directory for several dialogs
QStringList qucsPathList;

// #########################################################################
// Loads the settings file and stores the settings.
bool loadSettings()
{
    QSettings settings("qucs","qucs");

    if(settings.contains("x"))QucsSettings.x=settings.value("x").toInt();
    if(settings.contains("y"))QucsSettings.y=settings.value("y").toInt();
    if(settings.contains("dx"))QucsSettings.dx=settings.value("dx").toInt();
    if(settings.contains("dy"))QucsSettings.dy=settings.value("dy").toInt();
    if(settings.contains("font"))QucsSettings.font.fromString(settings.value("font").toString());
    if(settings.contains("LargeFontSize"))QucsSettings.largeFontSize=settings.value("LargeFontSize").toDouble(); // use toDouble() as it can interpret the string according to the current locale
    if(settings.contains("maxUndo"))QucsSettings.maxUndo=settings.value("maxUndo").toInt();
    if(settings.contains("NodeWiring"))QucsSettings.NodeWiring=settings.value("NodeWiring").toInt();
    if(settings.contains("BGColor"))QucsSettings.BGColor.setNamedColor(settings.value("BGColor").toString());
    if(settings.contains("Editor"))QucsSettings.Editor=settings.value("Editor").toString();
    if(settings.contains("FileTypes"))QucsSettings.FileTypes=settings.value("FileTypes").toStringList();
    if(settings.contains("Language"))QucsSettings.Language=settings.value("Language").toString();
    if(settings.contains("Comment"))QucsSettings.Comment.setNamedColor(settings.value("Comment").toString());
    if(settings.contains("String"))QucsSettings.String.setNamedColor(settings.value("String").toString());
    if(settings.contains("Integer"))QucsSettings.Integer.setNamedColor(settings.value("Integer").toString());
    if(settings.contains("Real"))QucsSettings.Real.setNamedColor(settings.value("Real").toString());
    if(settings.contains("Character"))QucsSettings.Character.setNamedColor(settings.value("Character").toString());
    if(settings.contains("Type"))QucsSettings.Type.setNamedColor(settings.value("Type").toString());
    if(settings.contains("Attribute"))QucsSettings.Attribute.setNamedColor(settings.value("Attribute").toString());
    if(settings.contains("Directive"))QucsSettings.Directive.setNamedColor(settings.value("Directive").toString());
    if(settings.contains("Task"))QucsSettings.Comment.setNamedColor(settings.value("Task").toString());

    if(settings.contains("Editor"))QucsSettings.Editor = settings.value("Editor").toString();
    //if(settings.contains("BinDir"))QucsSettings.BinDir = settings.value("BinDir").toString();
    //if(settings.contains("LangDir"))QucsSettings.LangDir = settings.value("LangDir").toString();
    //if(settings.contains("LibDir"))QucsSettings.LibDir = settings.value("LibDir").toString();
    if(settings.contains("AdmsXmlBinDir"))QucsSettings.AdmsXmlBinDir = settings.value("AdmsXmlBinDir").toString();
    if(settings.contains("AscoBinDir"))QucsSettings.AscoBinDir = settings.value("AscoBinDir").toString();
    //if(settings.contains("OctaveDir"))QucsSettings.OctaveDir = settings.value("OctaveDir").toString();
    //if(settings.contains("ExamplesDir"))QucsSettings.ExamplesDir = settings.value("ExamplesDir").toString();
    //if(settings.contains("DocDir"))QucsSettings.DocDir = settings.value("DocDir").toString();
    if(settings.contains("OctaveBinDir"))QucsSettings.OctaveBinDir.setPath(settings.value("OctaveBinDir").toString());
    if(settings.contains("QucsHomeDir"))
      if(settings.value("QucsHomeDir").toString() != "")
         QucsSettings.QucsHomeDir.setPath(settings.value("QucsHomeDir").toString());
    QucsSettings.QucsWorkDir = QucsSettings.QucsHomeDir;

    if (settings.contains("IgnoreVersion")) QucsSettings.IgnoreFutureVersion = settings.value("IgnoreVersion").toBool();
    // check also for old setting name with typo...
    else if (settings.contains("IngnoreVersion")) QucsSettings.IgnoreFutureVersion = settings.value("IngnoreVersion").toBool();
    else QucsSettings.IgnoreFutureVersion = false;

    if (settings.contains("GraphAntiAliasing")) QucsSettings.GraphAntiAliasing = settings.value("GraphAntiAliasing").toBool();
    else QucsSettings.GraphAntiAliasing = false;

    if (settings.contains("TextAntiAliasing")) QucsSettings.TextAntiAliasing = settings.value("TextAntiAliasing").toBool();
    else QucsSettings.TextAntiAliasing = false;

    QucsSettings.RecentDocs = settings.value("RecentDocs").toString().split("*",QString::SkipEmptyParts);
    QucsSettings.numRecentDocs = QucsSettings.RecentDocs.count();


    // If present read in the list of directory paths in which Qucs should
    // search for subcircuit schematics
    int npaths = settings.beginReadArray("Paths");
    for (int i = 0; i < npaths; ++i)
    {
        settings.setArrayIndex(i);
        QString apath = settings.value("path").toString();
        qucsPathList.append(apath);
    }
    settings.endArray();

    QucsSettings.numRecentDocs = 0;

    return true;
}

// #########################################################################
// Saves the settings in the settings file.
bool saveApplSettings(QucsApp *qucs)
{
    QSettings settings ("qucs","qucs");

    settings.setValue("x", QucsSettings.x);
    settings.setValue("y", QucsSettings.y);
    settings.setValue("dx", QucsSettings.dx);
    settings.setValue("dy", QucsSettings.dy);
    settings.setValue("font", QucsSettings.font.toString());
    // store LargeFontSize as a string, so it will be also human-readable in the settings file (will be a @Variant() otherwise)
    settings.setValue("LargeFontSize", QString::number(QucsSettings.largeFontSize));
    settings.setValue("maxUndo", QucsSettings.maxUndo);
    settings.setValue("NodeWiring", QucsSettings.NodeWiring);
    settings.setValue("BGColor", QucsSettings.BGColor.name());
    settings.setValue("Editor", QucsSettings.Editor);
    settings.setValue("FileTypes", QucsSettings.FileTypes);
    settings.setValue("Language", QucsSettings.Language);
    settings.setValue("Comment", QucsSettings.Comment.name());
    settings.setValue("String", QucsSettings.String.name());
    settings.setValue("Integer", QucsSettings.Integer.name());
    settings.setValue("Real", QucsSettings.Real.name());
    settings.setValue("Character", QucsSettings.Character.name());
    settings.setValue("Type", QucsSettings.Type.name());
    settings.setValue("Attribute", QucsSettings.Attribute.name());
    settings.setValue("Directive", QucsSettings.Directive.name());
    settings.setValue("Task", QucsSettings.Comment.name());
    settings.setValue("Editor", QucsSettings.Editor);
    //settings.setValue("BinDir", QucsSettings.BinDir);
    //settings.setValue("LangDir", QucsSettings.LangDir);
    //settings.setValue("LibDir", QucsSettings.LibDir);
    settings.setValue("AdmsXmlBinDir", QucsSettings.AdmsXmlBinDir.canonicalPath());
    settings.setValue("AscoBinDir", QucsSettings.AscoBinDir.canonicalPath());
    //settings.setValue("OctaveDir", QucsSettings.OctaveDir);
    //settings.setValue("ExamplesDir", QucsSettings.ExamplesDir);
    //settings.setValue("DocDir", QucsSettings.DocDir);
    settings.setValue("OctaveBinDir", QucsSettings.OctaveBinDir.canonicalPath());
    settings.setValue("QucsHomeDir", QucsSettings.QucsHomeDir.canonicalPath());
    settings.setValue("IgnoreVersion", QucsSettings.IgnoreFutureVersion);
    settings.setValue("GraphAntiAliasing", QucsSettings.GraphAntiAliasing);
    settings.setValue("TextAntiAliasing", QucsSettings.TextAntiAliasing);

    // Copy the list of directory paths in which Qucs should
    // search for subcircuit schematics from qucsPathList
    settings.remove("Paths");
    settings.beginWriteArray("Paths");
    int i = 0;
    foreach(QString path, qucsPathList) {
         settings.setArrayIndex(i);
         settings.setValue("path", path);
         i++;
     }
     settings.endArray();

  return true;

}

/*!
 * \brief qucsMessageOutput handles qDebug, qWarning, qCritical, qFatal.
 * \param type Message type (Qt enum)
 * \param msg Message
 *
 * The message handler is used to get control of the messages.
 * Particulary on Windows, as the messages are sent to the debugger and do not
 * show on the terminal. The handler could aslo be extended to create a log
 * mechanism.
 * <http://qt-project.org/doc/qt-4.8/debug.html#warning-and-debugging-messages>
 * <http://qt-project.org/doc/qt-4.8/qtglobal.html#qInstallMsgHandler>
 */
void qucsMessageOutput(QtMsgType type, const char *msg)
{
  switch (type) {
  case QtDebugMsg:
    fprintf(stderr, "Debug: %s\n", msg);
    break;
  case QtWarningMsg:
    fprintf(stderr, "Warning: %s\n", msg);
    break;
  case QtCriticalMsg:
    fprintf(stderr, "Critical: %s\n", msg);
    break;
  case QtFatalMsg:
    fprintf(stderr, "Fatal: %s\n", msg);
    abort();
  }

#ifdef _WIN32
  OutputDebugStringA(msg);
#endif
}

Schematic *openSchematic(QString schematic)
{
  qDebug() << "*** try to load schematic :" << schematic;

  QFile file(schematic);  // save simulator messages
  if(file.open(QIODevice::ReadOnly)) {
    file.close();
  }
  else {
    fprintf(stderr, "Error: Could not load schematic %s\n", schematic.ascii());
    return NULL;
  }

  // populate Modules list
  Module::registerModules ();

  // new schematic from file
  Schematic *sch = new Schematic(0, schematic);

  // load schematic file if possible
  if(!sch->loadDocument()) {
    fprintf(stderr, "Error: Could not load schematic %s\n", schematic.ascii());
    delete sch;
    return NULL;
  }
  return sch;
}

int doNetlist(QString schematic, QString netlist)
{
  Schematic *sch = openSchematic(schematic);
  if (sch == NULL) {
    return 1;
  }

  qDebug() << "*** try to write netlist  :" << netlist;

  QStringList Collect;

  QTextEdit *ErrText = new QTextEdit();  //dummy
  QFile NetlistFile;
  QTextStream   Stream;

  Collect.clear();  // clear list for NodeSets, SPICE components etc.

  NetlistFile.setFileName(netlist);
  if(!NetlistFile.open(QIODevice::WriteOnly)) {
    fprintf(stderr, "Error: Could not load netlist %s\n", netlist.ascii());
    return -1;
  }

  Stream.setDevice(&NetlistFile);
  int SimPorts = sch->prepareNetlist(Stream, Collect, ErrText);

  if(SimPorts < -5) {
    NetlistFile.close();
    fprintf(stderr, "Error: Could not prepare the netlist...\n");
    return 1;
  }

  // output NodeSets, SPICE simulations etc.
  for(QStringList::Iterator it = Collect.begin();
  it != Collect.end(); ++it) {
    // don't put library includes into netlist...
    if ((*it).right(4) != ".lst" &&
    (*it).right(5) != ".vhdl" &&
    (*it).right(4) != ".vhd" &&
    (*it).right(2) != ".v") {
      Stream << *it << '\n';
    }
  }

  Stream << '\n';

  QString SimTime = sch->createNetlist(Stream, SimPorts);
  delete(sch);

  NetlistFile.close();

  return 0;
}

int doPrint(QString schematic, QString printFile,
    QString page, int dpi, QString color, QString orientation)
{
  Schematic *sch = openSchematic(schematic);
  if (sch == NULL) {
    return 1;
  }

  sch->Nodes = &(sch->DocNodes);
  sch->Wires = &(sch->DocWires);
  sch->Diagrams = &(sch->DocDiags);
  sch->Paintings = &(sch->DocPaints);
  sch->Components = &(sch->DocComps);
  sch->reloadGraphs();

  qDebug() << "*** try to print file  :" << printFile;

  // determine filetype
  if (printFile.endsWith(".pdf")) {
    //initial printer
    QPrinter *Printer = new QPrinter(QPrinter::HighResolution);
    Printer->setOptionEnabled(QPrinter::PrintSelection, true);
    Printer->setOptionEnabled(QPrinter::PrintPageRange, false);
    Printer->setOptionEnabled(QPrinter::PrintToFile, true);
    Printer->setFullPage(true);

    //set property
    Printer->setOutputFileName(printFile);

    //page size
    if (page == "A3") {
      Printer->setPaperSize(QPrinter::A3);
    } else if (page == "B4") {
      Printer->setPaperSize(QPrinter::B4);
    } else if (page == "B5") {
      Printer->setPaperSize(QPrinter::B5);
    } else {
      Printer->setPaperSize(QPrinter::A4);
    }
    //dpi
    Printer->setResolution(dpi);
    //color
    if (color == "BW") {
      Printer->setColorMode(QPrinter::GrayScale);
    } else {
      Printer->setColorMode(QPrinter::Color);
    }
    //orientation
    if (orientation == "landscape") {
      Printer->setOrientation(QPrinter::Landscape);
    } else {
      Printer->setOrientation(QPrinter::Portrait);
    }

    QPainter Painter(Printer);
    if(!Painter.device()) {      // valid device available ?
      qDebug() << "Error: Printer Error.";
      return -1;
    }

    bool fitToPage = true;
    ((QucsDoc *)sch)->print(Printer, &Painter,
      Printer->printRange() == QPrinter::AllPages, fitToPage);
  } else {

    const int bourder = 30;
    int w,h,wsel,hsel,
        xmin, ymin, xmin_sel, ymin_sel;

    sch->getSchWidthAndHeight(w, h, xmin, ymin);
    sch->getSelAreaWidthAndHeight(wsel, hsel, xmin_sel, ymin_sel);
    w += bourder;
    h += bourder;
    wsel += bourder;
    hsel += bourder;
    float scal = 1.0;

    if (printFile.endsWith(".svg") || printFile.endsWith(".eps")) {
      QSvgGenerator* svg1 = new QSvgGenerator();

      QString tempfile = printFile + ".tmp.svg";
      if (!printFile.endsWith(".svg")) {
          svg1->setFileName(tempfile);
      } else {
          svg1->setFileName(printFile);
      }
      // this seems not work as expected
      //svg1->setResolution(dpi);

      svg1->setSize(QSize(1.12*w, h));
      QPainter *p = new QPainter(svg1);
      p->fillRect(0, 0, svg1->size().width(), svg1->size().height(), Qt::white);
      ViewPainter *vp = new ViewPainter(p);
      vp->init(p, 1.0, 0, 0, xmin-bourder/2, ymin-bourder/2, 1.0, 1.0);

      sch->paintSchToViewpainter(vp,true,true);

      delete vp;
      delete p;
      delete svg1;

      if (!printFile.endsWith(".svg")) {
          QString cmd = "inkscape -z -D --file=";
          cmd += tempfile + " ";

          if (printFile.endsWith(".eps")) {
            cmd += "--export-eps=" + printFile;
          }

          int result = QProcess::execute(cmd);

          if (result!=0) {
              QMessageBox* msg =  new QMessageBox(QMessageBox::Critical,"Export to image", "Inkscape start error!", QMessageBox::Ok);
              msg->exec();
              delete msg;
          }
          QFile::remove(tempfile);
      }

    } else if (printFile.endsWith(".png")) {
      QImage* img;
      if (color == "BW") {
        img = new QImage(w, h, QImage::Format_Mono);
      } else {
        img = new QImage(w, h, QImage::Format_RGB888);
      }

      QPainter* p = new QPainter(img);
      p->fillRect(0, 0, w, h, Qt::white);
      ViewPainter* vp = new ViewPainter(p);
      vp->init(p, scal, 0, 0, xmin*scal-bourder/2, ymin*scal-bourder/2, scal,scal);

      sch->paintSchToViewpainter(vp,true,true);

      img->save(printFile);

      delete vp;
      delete p;
      delete img;
    } else {
      fprintf(stderr, "Unsupported format of output file. \n"
          "Use PNG, SVG or PDF format!\n");
      return -1;
    }
  }
  return 0;
}


// #########################################################################
// ##########                                                     ##########
// ##########                  Program Start                      ##########
// ##########                                                     ##########
// #########################################################################
int main(int argc, char *argv[])
{
  // apply default settings
  QucsSettings.font = QFont("Helvetica", 12);
  QucsSettings.largeFontSize = 16.0;
  QucsSettings.maxUndo = 20;
  QucsSettings.NodeWiring = 0;

  // initially center the application
  QApplication a(argc, argv);
  QDesktopWidget *d = a.desktop();
  int w = d->width();
  int h = d->height();
  QucsSettings.x = w/8;
  QucsSettings.y = h/8;
  QucsSettings.dx = w*3/4;
  QucsSettings.dy = h*3/4;

  // check for relocation env variable
  char* var = getenv("QUCSDIR");
  QDir QucsDir;
  if (var!= NULL)
  {
      QucsDir = QDir(QString(var));
      qDebug() << "QUCSDIR set: " << QucsDir.absolutePath();
  }
  else
  {
     QString QucsApplicationPath = QCoreApplication::applicationDirPath();
     #ifdef __APPLE__
     QucsDir = QDir(QucsApplicationPath.section("/bin",0,0));
     #else
     QucsDir = QDir(QucsApplicationPath);
     QucsDir.cdUp();
     #endif

  }

  QucsSettings.BinDir =      QucsDir.absolutePath() + "/bin/";
  QucsSettings.LangDir =     QucsDir.canonicalPath() + "/share/qucs/lang/";
  QucsSettings.LibDir =      QucsDir.canonicalPath() + "/share/qucs/library/";
  QucsSettings.OctaveDir =   QucsDir.canonicalPath() + "/share/qucs/octave/";
  QucsSettings.ExamplesDir = QucsDir.canonicalPath() + "/share/qucs/docs/examples/";
  QucsSettings.DocDir =      QucsDir.canonicalPath() + "/share/qucs/docs/";

  QucsSettings.Editor = "qucs";
  QucsSettings.QucsHomeDir.setPath(QDir::homeDirPath()+QDir::convertSeparators ("/.qucs"));
  QucsSettings.QucsWorkDir.setPath(QucsSettings.QucsHomeDir.canonicalPath());


  var = getenv("ADMSXMLBINDIR");
  if(var != NULL) {
      QucsSettings.AdmsXmlBinDir.setPath(QString(var));
  }
  else {
      // default admsXml bindir same as Qucs
      QString admsExec;
#ifdef __MINGW32__
      admsExec = QDir::toNativeSeparators(QucsSettings.BinDir+"/"+"admsXml.exe");
#else
      admsExec = QDir::toNativeSeparators(QucsSettings.BinDir+"/"+"admsXml");
#endif
      QFile adms(admsExec);
      if(adms.exists())
        QucsSettings.AdmsXmlBinDir.setPath(QucsSettings.BinDir);
  }

  var = getenv("ASCOBINDIR");
  if(var != NULL)  {
      QucsSettings.AscoBinDir.setPath(QString(var));
  }
  else  {
      // default ASCO bindir same as Qucs
      QString ascoExec;
#ifdef __MINGW32__
      ascoExec = QDir::toNativeSeparators(QucsSettings.BinDir+"/"+"asco.exe");
#else
      ascoExec = QDir::toNativeSeparators(QucsSettings.BinDir+"/"+"asco");
#endif
      QFile asco(ascoExec);
      if(asco.exists())
        QucsSettings.AscoBinDir.setPath(QucsSettings.BinDir);
  }

  var = getenv("OCTAVEBINDIR");
  if(var != NULL)  {
      QucsSettings.OctaveBinDir.setPath(QString(var));
  }
  else  {
#ifdef __MINGW32__
      QucsSettings.OctaveBinDir.setPath(QString("C:/Software/Octave-3.6.4/bin/"));
#else
      QFile octaveExec("/usr/bin/octave");
      if(octaveExec.exists())QucsSettings.OctaveBinDir.setPath(QString("/usr/bin/"));
      QFile octaveExec1("/usr/local/bin/octave");
      if(octaveExec1.exists()) QucsSettings.OctaveBinDir.setPath(QString("/usr/local/bin/"));
#endif
  }
  loadSettings();

  if(!QucsSettings.BGColor.isValid())
    QucsSettings.BGColor.setRgb(255, 250, 225);

  // syntax highlighting
  if(!QucsSettings.Comment.isValid())
    QucsSettings.Comment = Qt::gray;
  if(!QucsSettings.String.isValid())
    QucsSettings.String = Qt::red;
  if(!QucsSettings.Integer.isValid())
    QucsSettings.Integer = Qt::blue;
  if(!QucsSettings.Real.isValid())
    QucsSettings.Real = Qt::darkMagenta;
  if(!QucsSettings.Character.isValid())
    QucsSettings.Character = Qt::magenta;
  if(!QucsSettings.Type.isValid())
    QucsSettings.Type = Qt::darkRed;
  if(!QucsSettings.Attribute.isValid())
    QucsSettings.Attribute = Qt::darkCyan;
  if(!QucsSettings.Directive.isValid())
    QucsSettings.Directive = Qt::darkCyan;
  if(!QucsSettings.Task.isValid())
    QucsSettings.Task = Qt::darkRed;


  a.setFont(QucsSettings.font);

  // set codecs
  QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
  QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

  QTranslator tor( 0 );
  QString lang = QucsSettings.Language;
  if(lang.isEmpty())
    lang = QTextCodec::locale();
  tor.load( QString("qucs_") + lang, QucsSettings.LangDir);
  a.installTranslator( &tor );

  // This seems to be neccessary on a few system to make strtod()
  // work properly !???!
  setlocale (LC_NUMERIC, "C");

  QString inputfile;
  QString outputfile;

  bool netlist_flag = false;
  bool print_flag = false;
  QString page = "A4";
  int dpi = 96;
  QString color = "RGB";
  QString orientation = "portraid";

  // simple command line parser
  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
      fprintf(stdout,
  "Usage: %s [-hv] \n"
  "       qucs -n -i FILENAME -o FILENAME\n"
  "       qucs -p -i FILENAME -o FILENAME.[pdf|png|svg|eps] \n\n"
  "  -h, --help     display this help and exit\n"
  "  -v, --version  display version information and exit\n"
  "  -n, --netlist  convert Qucs schematic into netlist\n"
  "  -p, --print    print Qucs schematic to file (eps needs inkscape)\n"
  "    --page [A4|A3|B4|B5]         set print page size (default A4)\n"
  "    --dpi NUMBER                 set dpi value (default 96)\n"
  "    --color [RGB|RGB]            set color mode (default RGB)\n"
  "    --orin [portraid|landscape]  set orientation (default portraid)\n"
  "  -i FILENAME    use file as input schematic\n"
  "  -o FILENAME    use file as output netlist\n"
  , argv[0]);
      return 0;
    }
    else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")) {
#ifdef GIT
      fprintf(stdout, "Qucs " PACKAGE_VERSION " ("GIT")" "\n");
#else
      fprintf(stdout, "Qucs " PACKAGE_VERSION "\n");
#endif
      return 0;
    }
    else if (!strcmp(argv[i], "-n") || !strcmp(argv[i], "--netlist")) {
      netlist_flag = true;
    }
    else if (!strcmp(argv[i], "-p") || !strcmp(argv[i], "--print")) {
      print_flag = true;
    }
    else if (!strcmp(argv[i], "--page")) {
      page = argv[++i];
    }
    else if (!strcmp(argv[i], "--dpi")) {
      dpi = QString(argv[++i]).toInt();
    }
    else if (!strcmp(argv[i], "--color")) {
      color = argv[++i];
    }
    else if (!strcmp(argv[i], "--orin")) {
      orientation = argv[++i];
    }
    else if (!strcmp(argv[i], "-i")) {
      inputfile = argv[++i];
    }
    else if (!strcmp(argv[i], "-o")) {
      outputfile = argv[++i];
    }
    else {
      fprintf(stderr, "Error: Unknown option: %s\n", argv[i]);
      return -1;
    }
  }

  // check operation and its required arguments
  if (netlist_flag and print_flag) {
    fprintf(stderr, "Error: --print and --netlist cannot be used together\n");
    return -1;
  } else if (netlist_flag or print_flag) {
    if (inputfile.isEmpty()) {
      fprintf(stderr, "Error: Expected input file.\n");
      return -1;
    }
    if (outputfile.isEmpty()) {
      fprintf(stderr, "Error: Expected output file.\n");
      return -1;
    }
    // create netlist from schematic
    if (netlist_flag) {
      return doNetlist(inputfile, outputfile);
    } else if (print_flag) {
      return doPrint(inputfile, outputfile,
          page, dpi, color, orientation);
    }
  }

  QucsMain = new QucsApp();
  a.setMainWidget(QucsMain);
  qInstallMsgHandler(qucsMessageOutput);
  QucsMain->show();
  int result = a.exec();
  //saveApplSettings(QucsMain);
  return result;
}
