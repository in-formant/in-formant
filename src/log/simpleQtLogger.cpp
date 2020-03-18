/*
  Simple Logger for Qt

  Mario Ban, 05.2015
  https://github.com/Mokolea/SimpleQtLogger

  GNU Lesser General Public License v2.1
  Copyright (C) 2017 Mario Ban
*/

#include "simpleQtLogger.h"

#include <QCoreApplication>
#include <QTimer>
#include <QDateTime>
#include <QFileInfo>
#include <QtDebug>

namespace simpleqtlogger {

/* Log-sinks */
bool ENABLE_LOG_SINK_FILE = true;
bool ENABLE_LOG_SINK_CONSOLE = false;
bool ENABLE_LOG_SINK_QDEBUG = false;
bool ENABLE_LOG_SINK_SIGNAL = false;

/* Log-level */
EnableLogLevels::EnableLogLevels()
  : logLevel_FATAL(true)
  , logLevel_ERROR(true)
  , logLevel_WARNING(true)
  , logLevel_NOTE(true)
  , logLevel_INFO(true)
  , logLevel_DEBUG(false)
  , logLevel_FUNCTION(false)
  , logLevel_INTERNAL(true)
{}
bool EnableLogLevels::enabled(LogLevel logLevel) const
{
  if(logLevel == LogLevel_FATAL) return logLevel_FATAL;
  if(logLevel == LogLevel_ERROR) return logLevel_ERROR;
  if(logLevel == LogLevel_WARNING) return logLevel_WARNING;
  if(logLevel == LogLevel_NOTE) return logLevel_NOTE;
  if(logLevel == LogLevel_INFO) return logLevel_INFO;
  if(logLevel == LogLevel_DEBUG) return logLevel_DEBUG;
  if(logLevel == LogLevel_FUNCTION) return logLevel_FUNCTION;
  if(logLevel == LogLevel_INTERNAL) return logLevel_INTERNAL;
  return false;
}
EnableLogLevels ENABLE_LOG_LEVELS;

/* Log-function stack-trace */
bool ENABLE_FUNCTION_STACK_TRACE = true;

/* Sink console color */
bool ENABLE_CONSOLE_COLOR = true;
/* Sink console trimmed messages */
bool ENABLE_CONSOLE_TRIMMED = true;

/* Console */
bool ENABLE_CONSOLE_LOG_FILE_STATE = true;

// -------------------------------------------------------------------------------------------------

Sink::Sink(QObject *parent)
  : QObject(parent)
  , _logFormat(DEFAULT_LOG_FORMAT)
  , _logFormatInt(DEFAULT_LOG_FORMAT_INTERNAL)
{
  // qDebug("Sink::Sink");
}

Sink::~Sink()
{
  // qDebug("Sink::~Sink");
}

void Sink::setLogFormat(const QString& logFormat, const QString& logFormatInt)
{
  // qDebug("Sink::setLogFormat");
  _logFormat = logFormat;
  _logFormatInt = logFormatInt;
}

void Sink::setLogLevels(const EnableLogLevels& enableLogLevels)
{
  // qDebug("Sink::setLogLevels");
  _enableLogLevels = enableLogLevels;
}

EnableLogLevels Sink::getLogLevels() const
{
  // qDebug("Sink::getLogLevels");
  return _enableLogLevels;
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
bool Sink::addLogFilter(const QRegularExpression& re)
{
  // qDebug("Sink::addLogFilter");
  if(!re.isValid()) {
    return false;
  }
  _reList.append(re);
  return true;
}
#endif

QString Sink::getLogFormat() const
{
  // qDebug("Sink::getLogFormat");
  return _logFormat;
}

QString Sink::getLogFormatInt() const
{
  // qDebug("Sink::getLogFormatInt");
  return _logFormatInt;
}

bool Sink::checkLogLevelsEnabled(LogLevel logLevel) const
{
  // qDebug("Sink::checkLogLevelsEnabled");
  return _enableLogLevels.enabled(logLevel);
}

bool Sink::checkFilter(const QString& text) const
{
  // qDebug("Sink::checkFilter");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
  if(_reList.isEmpty()) {
    return true;
  }

  // C++11: for(const QRegularExpression& re : _reList) {...}
  for (int i = 0; i < _reList.size(); ++i) {
    QRegularExpressionMatch match = _reList[i].match(text, 0);
    if(match.hasMatch()) {
      return true;
    }
  }
  return false;
#else
  Q_UNUSED(text);
  return true;
#endif
}

// -------------------------------------------------------------------------------------------------

SinkConsoleLog::SinkConsoleLog(QObject *parent)
  : Sink(parent)
{
  // qDebug("SinkConsoleLog::SinkConsoleLog");
}

SinkConsoleLog::~SinkConsoleLog()
{
  // qDebug("SinkConsoleLog::~SinkConsoleLog");
}

void SinkConsoleLog::slotLog(const QString& ts, const QString& tid, const QString& text, LogLevel logLevel, const QString& functionName, const QString& fileName, unsigned int lineNumber)
{
  // qDebug("SinkConsoleLog::slotLog");

  if(!ENABLE_LOG_SINK_CONSOLE) {
    return;
  }
  if(!checkLogLevelsEnabled(logLevel)) {
    return;
  }
  if(logLevel != LogLevel_INTERNAL && !checkFilter(text)) {
    return;
  }

  QString textIsEmpty("?");
  if(logLevel == LogLevel_FUNCTION) {
    textIsEmpty = "-";
  }

  QTextStream out(stdout);
  // out.setCodec("UTF-8");
  if(ENABLE_CONSOLE_COLOR) {
    // set text colors (foreground/background)
    if(logLevel == LogLevel_FATAL) {
      out << CONSOLE_COLOR_ANSI_ESC_CODES_FATAL_I << CONSOLE_LOG_LEVEL_LABEL_FATAL << CONSOLE_COLOR_ANSI_ESC_CODES_FATAL << ": ";
    }
    else if(logLevel == LogLevel_ERROR) {
      out << CONSOLE_COLOR_ANSI_ESC_CODES_ERROR_I << CONSOLE_LOG_LEVEL_LABEL_ERROR << CONSOLE_COLOR_ANSI_ESC_CODES_ERROR << ": ";
    }
    else if(logLevel == LogLevel_WARNING) {
      out << CONSOLE_COLOR_ANSI_ESC_CODES_WARNING_I << CONSOLE_LOG_LEVEL_LABEL_WARNING << CONSOLE_COLOR_ANSI_ESC_CODES_WARNING << ": ";
    }
    else if(logLevel == LogLevel_NOTE) {
      out << CONSOLE_COLOR_ANSI_ESC_CODES_NOTE_I << CONSOLE_LOG_LEVEL_LABEL_NOTE << CONSOLE_COLOR_ANSI_ESC_CODES_RESET << ": ";
    }
    else if(logLevel == LogLevel_DEBUG) {
      out << CONSOLE_COLOR_ANSI_ESC_CODES_DEBUG_I << CONSOLE_LOG_LEVEL_LABEL_DEBUG << CONSOLE_COLOR_ANSI_ESC_CODES_DEBUG << ": ";
    }
    else if(logLevel == LogLevel_FUNCTION) {
      out << CONSOLE_COLOR_ANSI_ESC_CODES_FUNCTION;
    }
  }
  else {
    if(logLevel == LogLevel_FATAL) {
      out << CONSOLE_LOG_LEVEL_LABEL_FATAL << ": ";
    }
    else if(logLevel == LogLevel_ERROR) {
      out << CONSOLE_LOG_LEVEL_LABEL_ERROR << ": ";
    }
    else if(logLevel == LogLevel_WARNING) {
      out << CONSOLE_LOG_LEVEL_LABEL_WARNING << ": ";
    }
    else if(logLevel == LogLevel_NOTE) {
      out << CONSOLE_LOG_LEVEL_LABEL_NOTE << ": ";
    }
    else if(logLevel == LogLevel_DEBUG) {
      out << CONSOLE_LOG_LEVEL_LABEL_DEBUG << ": ";
    }
  }

  if(logLevel == LogLevel_INTERNAL) {
    out << getLogFormatInt().replace("<TS>", ts).replace("<TID>", tid).replace("<TID32>", tid.right(4*2)).replace("<LL>", QString(LOG_LEVEL_CHAR[logLevel])).replace("<TEXT>", text.isEmpty() ? textIsEmpty : text.trimmed());
  }
  else if(logLevel == LogLevel_FUNCTION) {
    out << getLogFormat().append(DEFAULT_LOG_FORMAT_CONSOLE_FUNCTION_SUFFIX).replace("<TS>", ts).replace("<TID>", tid).replace("<TID32>", tid.right(4*2)).replace("<LL>", QString(LOG_LEVEL_CHAR[logLevel])).replace("<FUNC>", functionName).replace("<FILE>", fileName).replace("<LINE>", QString("%1").arg(lineNumber)).replace("<TEXT>", text.isEmpty() ? textIsEmpty : text.trimmed());
  }
  else {
    out << getLogFormat().replace("<TS>", ts).replace("<TID>", tid).replace("<TID32>", tid.right(4*2)).replace("<LL>", QString(LOG_LEVEL_CHAR[logLevel])).replace("<FUNC>", functionName).replace("<FILE>", fileName).replace("<LINE>", QString("%1").arg(lineNumber)).replace("<TEXT>", text.isEmpty() ? textIsEmpty : ENABLE_CONSOLE_TRIMMED ? text.trimmed() : text);
  }

  if(ENABLE_CONSOLE_COLOR) {
    if(logLevel != LogLevel_NOTE && logLevel != LogLevel_INFO && logLevel != LogLevel_INTERNAL) {
      out << CONSOLE_COLOR_ANSI_ESC_CODES_RESET;
    }
  }
  out << '\n';
}

// -------------------------------------------------------------------------------------------------

SinkQDebugLog::SinkQDebugLog(QObject *parent)
  : Sink(parent)
{
  // qDebug("SinkQDebugLog::SinkQDebugLog");
}

SinkQDebugLog::~SinkQDebugLog()
{
  // qDebug("SinkQDebugLog::~SinkQDebugLog");
}

void SinkQDebugLog::slotLog(const QString& ts, const QString& tid, const QString& text, LogLevel logLevel, const QString& functionName, const QString& fileName, unsigned int lineNumber)
{
  // qDebug("SinkQDebugLog::slotLog");

  if(!ENABLE_LOG_SINK_QDEBUG) {
    return;
  }
  if(!checkLogLevelsEnabled(logLevel)) {
    return;
  }
  if(logLevel != LogLevel_INTERNAL && !checkFilter(text)) {
    return;
  }

  QString textIsEmpty("?");
  if(logLevel == LogLevel_FUNCTION) {
    textIsEmpty = "-";
  }

  if(logLevel == LogLevel_INTERNAL) {
    qDebug("%s", getLogFormatInt().replace("<TS>", ts).replace("<TID>", tid).replace("<TID32>", tid.right(4*2)).replace("<LL>", QString(LOG_LEVEL_CHAR[logLevel])).replace("<TEXT>", text.isEmpty() ? textIsEmpty : text.trimmed()).toStdString().c_str());
  }
  else {
    qDebug("%s", getLogFormat().replace("<TS>", ts).replace("<TID>", tid).replace("<TID32>", tid.right(4*2)).replace("<LL>", QString(LOG_LEVEL_CHAR[logLevel])).replace("<FUNC>", functionName).replace("<FILE>", fileName).replace("<LINE>", QString("%1").arg(lineNumber)).replace("<TEXT>", text.isEmpty() ? textIsEmpty : text.trimmed()).toStdString().c_str());
  }
}

// -------------------------------------------------------------------------------------------------

SinkSignalLog::SinkSignalLog(QObject *parent)
  : Sink(parent)
{
  // qDebug("SinkSignalLog::SinkSignalLog");
}

SinkSignalLog::~SinkSignalLog()
{
  // qDebug("SinkSignalLog::~SinkSignalLog");
}

void SinkSignalLog::slotLog(const QString& ts, const QString& tid, const QString& text, LogLevel logLevel, const QString& functionName, const QString& fileName, unsigned int lineNumber)
{
  // qDebug("SinkSignalLog::slotLog");

  if(!ENABLE_LOG_SINK_SIGNAL) {
    return;
  }
  if(!checkLogLevelsEnabled(logLevel)) {
    return;
  }
  if(logLevel != LogLevel_INTERNAL && !checkFilter(text)) {
    return;
  }

  QString textIsEmpty("?");
  if(logLevel == LogLevel_FUNCTION) {
    textIsEmpty = "-";
  }

  if(logLevel == LogLevel_INTERNAL) {
    emit signalLogMessage(logLevel, getLogFormatInt().replace("<TS>", ts).replace("<TID>", tid).replace("<TID32>", tid.right(4*2)).replace("<LL>", QString(LOG_LEVEL_CHAR[logLevel])).replace("<TEXT>", text.isEmpty() ? textIsEmpty : text.trimmed()));
  }
  else {
    emit signalLogMessage(logLevel, getLogFormat().replace("<TS>", ts).replace("<TID>", tid).replace("<TID32>", tid.right(4*2)).replace("<LL>", QString(LOG_LEVEL_CHAR[logLevel])).replace("<FUNC>", functionName).replace("<FILE>", fileName).replace("<LINE>", QString("%1").arg(lineNumber)).replace("<TEXT>", text.isEmpty() ? textIsEmpty : text.trimmed()));
  }
}

// -------------------------------------------------------------------------------------------------

SinkFileLog::SinkFileLog(QObject *parent, const QString& role)
  : Sink(parent)
  , _role(role)
  , _logFileRotationSize(0)
  , _logFileMaxNumber(0)
  , _logFile(0)
  , _logFileActivity(false)
  , _startMessage(false)
{
  // qDebug("SinkFileLog::SinkFileLog");
}

SinkFileLog::~SinkFileLog()
{
  // qDebug("SinkFileLog::~SinkFileLog");

  // check close log file
  if(_logFile) {
    if(_logFile->isOpen()) {
      _logFile->close();
    }
    delete _logFile;
    _logFile = 0;
  }
}

bool SinkFileLog::setLogFileName(const QString& logFileName, unsigned int logFileRotationSize, unsigned int logFileMaxNumber)
{
  // qDebug("SinkFileLog::setLogFileName");

  // check valid log-file name ending
  if(logFileName.right(4) != ".log") {
    qWarning() << "Name of log-file not ending with '.log'" << logFileName << "role" << _role;
    return false;
  }

  _logFileName = logFileName;
  _logFileRotationSize = logFileRotationSize;
  _logFileMaxNumber = logFileMaxNumber;

  // check valid number ranges
  if(_logFileRotationSize < 100) {
    _logFileRotationSize = 100;
  }
  if(_logFileMaxNumber < 1) {
    _logFileMaxNumber = 1;
  }
  if(_logFileMaxNumber > 99) {
    _logFileMaxNumber = 99;
  }

  return checkLogFileOpen();
}

void SinkFileLog::slotLog(const QString& ts, const QString& tid, const QString& text, LogLevel logLevel, const QString& functionName, const QString& fileName, unsigned int lineNumber)
{
  // qDebug("SinkFileLog::slotLog");

  if(!ENABLE_LOG_SINK_FILE) {
    return;
  }
  if(!checkLogLevelsEnabled(logLevel)) {
    return;
  }
  if(logLevel != LogLevel_INTERNAL && !checkFilter(text)) {
    return;
  }

  QString textIsEmpty("?");
  if(logLevel == LogLevel_FUNCTION) {
    textIsEmpty = "-";
  }

  // stream (append) to log file
  if(_logFile && _logFile->isOpen()) {
    QTextStream out(_logFile);
    out.setCodec("UTF-8");
    if(logLevel == LogLevel_INTERNAL) {
      out << getLogFormatInt().replace("<TS>", ts).replace("<TID>", tid).replace("<TID32>", tid.right(4*2)).replace("<LL>", QString(LOG_LEVEL_CHAR[logLevel])).replace("<TEXT>", text.isEmpty() ? textIsEmpty : text.trimmed()) << '\n';
    }
    else {
      out << getLogFormat().replace("<TS>", ts).replace("<TID>", tid).replace("<TID32>", tid.right(4*2)).replace("<LL>", QString(LOG_LEVEL_CHAR[logLevel])).replace("<FUNC>", functionName).replace("<FILE>", fileName).replace("<LINE>", QString("%1").arg(lineNumber)).replace("<TEXT>", text.isEmpty() ? textIsEmpty : text.trimmed()) << '\n';
    }
    _logFileActivity = true;
  }
}

bool SinkFileLog::checkLogFileOpen()
{
  // qDebug("SinkFileLog::checkLogFileOpen");

  // check close and open log file
  if(_logFile) {
    if(_logFile->isOpen()) {
      _logFile->close();
    }
    delete _logFile;
  }

  // open log-file
  _logFile = new QFile(_logFileName);
  if(!_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
    delete _logFile;
    _logFile = 0;
    qWarning() << "Open log-file failed!" << _logFileName << "role" << _role;
  }

  if(!_logFile) {
    return false;
  }

  if(ENABLE_CONSOLE_LOG_FILE_STATE) {
    qDebug() << "Current log-file:" << _logFileName << "role" << _role;
  }

  QTimer::singleShot(CHECK_LOG_FILE_ACTIVITY_INTERVAL, this, SLOT(slotCheckLogFileActivity()));

  if(!_startMessage) {
    _startMessage = true;
    slotLog(SimpleQtLogger::timeStamp(), SimpleQtLogger::threadId(), QString("Start file-log '%1'").arg(_role), LogLevel_INTERNAL, "", "", 0);
  }

  return true;
}

void SinkFileLog::checkLogFileRolling()
{
  // qDebug("SinkFileLog::checkLogFileRolling");

  if(!_logFile) {
    return;
  }

  // check current log-file size
  QFileInfo logFileInfo(*_logFile);
  qint64 logFileSize = logFileInfo.size();

  if(logFileSize < _logFileRotationSize) {
    QTimer::singleShot(CHECK_LOG_FILE_ACTIVITY_INTERVAL, this, SLOT(slotCheckLogFileActivity()));
    return;
  }
  slotLog(SimpleQtLogger::timeStamp(), SimpleQtLogger::threadId(), QString("Current log-file '%1' size=%2 (rotation-size=%3) --> rolling").arg(_role).arg(logFileSize).arg(_logFileRotationSize), LogLevel_INTERNAL, "", "", 0);

  QTime timeRolling;
  timeRolling.start();

  // handle file rolling

  // delete last file
  QString logFileName = _logFileName;
  logFileName.replace(".log", QString("_%1.log").arg(_logFileMaxNumber, 2, 10, QLatin1Char('0')));
  if(QFile::exists(logFileName)) {
    if(QFile::remove(logFileName)) {
      if(ENABLE_CONSOLE_LOG_FILE_STATE) {
        qDebug() << "Removed" << logFileName << "role" << _role;
      }
    }
    else {
      qWarning() << "ERROR: Remove" << logFileName << "role" << _role;
    }
  }

  // rolling files
  for(int i=_logFileMaxNumber-1; i>0; --i) {
    QString logFileNameFrom = _logFileName;
    logFileNameFrom.replace(".log", QString("_%1.log").arg(i, 2, 10, QLatin1Char('0')));
    QString logFileNameTo = _logFileName;
    logFileNameTo.replace(".log", QString("_%1.log").arg(i+1, 2, 10, QLatin1Char('0')));
    if(QFile::exists(logFileNameFrom)) {
      if(QFile::rename(logFileNameFrom, logFileNameTo)) {
        if(ENABLE_CONSOLE_LOG_FILE_STATE) {
          qDebug() << "Moved" << logFileNameFrom << "to" << logFileNameTo << "role" << _role;
        }
      }
      else {
        qWarning() << "ERROR: Move" << logFileNameFrom << "to" << logFileNameTo << "role" << _role;
      }
    }
  }

  _logFile->close();

  // move first file
  QString logFileNameTo = _logFileName;
  logFileNameTo.replace(".log", QString("_%1.log").arg(1, 2, 10, QLatin1Char('0')));
  if(QFile::exists(_logFileName)) {
    if(QFile::rename(_logFileName, logFileNameTo)) {
      if(ENABLE_CONSOLE_LOG_FILE_STATE) {
        qDebug() << "Moved" << _logFileName << "to" << logFileNameTo << "role" << _role;
      }
    }
    else {
      qWarning() << "ERROR: Move" << _logFileName << "to" << logFileNameTo << "role" << _role;
    }
  }

  checkLogFileOpen();

  slotLog(SimpleQtLogger::timeStamp(), SimpleQtLogger::threadId(), QString("Log-file '%1' rolling done (time elapsed: %2 ms)").arg(_role).arg(timeRolling.elapsed()), LogLevel_INTERNAL, "", "", 0);
}

void SinkFileLog::slotCheckLogFileActivity()
{
  // qDebug("SinkFileLog::slotCheckLogFileActivity");

  if(!_logFile) {
    return;
  }

  if(_logFileActivity) {
    _logFileActivity = false;
    checkLogFileRolling();
    return;
  }

  QTimer::singleShot(CHECK_LOG_FILE_ACTIVITY_INTERVAL, this, SLOT(slotCheckLogFileActivity()));
}

// -------------------------------------------------------------------------------------------------

SimpleQtLogger* SimpleQtLogger::instance = 0;

SimpleQtLogger* SimpleQtLogger::createInstance(QObject *parent)
{
  if(instance) {
    delete instance;
  }
  instance = new SimpleQtLogger(parent);
  return instance;
}

SimpleQtLogger* SimpleQtLogger::getInstance()
{
  return instance;
}

SimpleQtLogger::SimpleQtLogger(QObject *parent)
  : QObject(parent)
{
  // qDebug("SimpleQtLogger::SimpleQtLogger");

  qRegisterMetaType<LogLevel>("LogLevel"); // to use type in Qt::QueuedConnection

  _sinkConsoleLog = new SinkConsoleLog(this);
  _sinkQDebugLog = new SinkQDebugLog(this);
  _sinkSignalLog = new SinkSignalLog(this);

  _sinkConsoleLog->setLogFormat(DEFAULT_LOG_FORMAT_CONSOLE, DEFAULT_LOG_FORMAT_INTERNAL);

  // Qt::ConnectionType is Qt::AutoConnection (Default)
  // If the receiver lives in the thread that emits the signal, Qt::DirectConnection is used.
  // Otherwise, Qt::QueuedConnection is used. The connection type is determined when the signal is emitted.
#if ENABLE_SQTL_LOG_SINK_CONSOLE > 0
  QObject::connect(this, SIGNAL(signalLog(const QString&, const QString&, const QString&, LogLevel, const QString&, const QString&, unsigned int)),
    _sinkConsoleLog, SLOT(slotLog(const QString&, const QString&, const QString&, LogLevel, const QString&, const QString&, unsigned int)));
#endif
#if ENABLE_SQTL_LOG_SINK_QDEBUG > 0
  QObject::connect(this, SIGNAL(signalLog(const QString&, const QString&, const QString&, LogLevel, const QString&, const QString&, unsigned int)),
    _sinkQDebugLog, SLOT(slotLog(const QString&, const QString&, const QString&, LogLevel, const QString&, const QString&, unsigned int)));
#endif
#if ENABLE_SQTL_LOG_SINK_SIGNAL > 0
  QObject::connect(this, SIGNAL(signalLog(const QString&, const QString&, const QString&, LogLevel, const QString&, const QString&, unsigned int)),
    _sinkSignalLog, SLOT(slotLog(const QString&, const QString&, const QString&, LogLevel, const QString&, const QString&, unsigned int)));
#endif

  addSinkFileLog("main");
}

SimpleQtLogger::~SimpleQtLogger()
{
  // qDebug("SimpleQtLogger::~SimpleQtLogger");
}

void SimpleQtLogger::addSinkFileLog(const QString& role)
{
  // qDebug("SimpleQtLogger::addSinkFileLog");

  if(_sinkFileLogMap.contains(role)) {
    return;
  }

  _sinkFileLogMap[role] = new SinkFileLog(this, role);

#if ENABLE_SQTL_LOG_SINK_FILE > 0
  QObject::connect(this, SIGNAL(signalLog(const QString&, const QString&, const QString&, LogLevel, const QString&, const QString&, unsigned int)),
    _sinkFileLogMap[role], SLOT(slotLog(const QString&, const QString&, const QString&, LogLevel, const QString&, const QString&, unsigned int)));
#endif
}

void SimpleQtLogger::setLogFormat_file(const QString& logFormat, const QString& logFormatInt)
{
  // qDebug("SimpleQtLogger::setLogFormat_file");
  setLogFormat_file("main", logFormat, logFormatInt);
}

void SimpleQtLogger::setLogFormat_file(const QString& role, const QString& logFormat, const QString& logFormatInt)
{
  // qDebug("SimpleQtLogger::setLogFormat_file");
  if(_sinkFileLogMap.contains(role)) {
    _sinkFileLogMap[role]->setLogFormat(logFormat, logFormatInt);
  }
}

void SimpleQtLogger::setLogFormat_console(const QString& logFormat, const QString& logFormatInt)
{
  // qDebug("SimpleQtLogger::setLogFormat_console");
  _sinkConsoleLog->setLogFormat(logFormat, logFormatInt);
}

void SimpleQtLogger::setLogFormat_qDebug(const QString& logFormat, const QString& logFormatInt)
{
  // qDebug("SimpleQtLogger::setLogFormat_qDebug");
  _sinkQDebugLog->setLogFormat(logFormat, logFormatInt);
}

void SimpleQtLogger::setLogFormat_signal(const QString& logFormat, const QString& logFormatInt)
{
  // qDebug("SimpleQtLogger::setLogFormat_signal");
  _sinkSignalLog->setLogFormat(logFormat, logFormatInt);
}

void SimpleQtLogger::setLogLevels_file(const EnableLogLevels& enableLogLevels)
{
  // qDebug("SimpleQtLogger::setLogLevels_file");
  setLogLevels_file("main", enableLogLevels);
}

void SimpleQtLogger::setLogLevels_file(const QString& role, const EnableLogLevels& enableLogLevels)
{
  // qDebug("SimpleQtLogger::setLogLevels_file");
  if(_sinkFileLogMap.contains(role)) {
    _sinkFileLogMap[role]->setLogLevels(enableLogLevels);
  }
}

void SimpleQtLogger::setLogLevels_console(const EnableLogLevels& enableLogLevels)
{
  // qDebug("SimpleQtLogger::setLogLevels_console");
  _sinkConsoleLog->setLogLevels(enableLogLevels);
}

void SimpleQtLogger::setLogLevels_qDebug(const EnableLogLevels& enableLogLevels)
{
  // qDebug("SimpleQtLogger::setLogLevels_qDebug");
  _sinkQDebugLog->setLogLevels(enableLogLevels);
}

void SimpleQtLogger::setLogLevels_signal(const EnableLogLevels& enableLogLevels)
{
  // qDebug("SimpleQtLogger::setLogLevels_signal");
  _sinkSignalLog->setLogLevels(enableLogLevels);
}

EnableLogLevels SimpleQtLogger::getLogLevels_file() const // main
{
  // qDebug("SimpleQtLogger::getLogLevels_file");
  return getLogLevels_file("main");
}

EnableLogLevels SimpleQtLogger::getLogLevels_file(const QString& role) const
{
  // qDebug("SimpleQtLogger::getLogLevels_file");
  if(_sinkFileLogMap.contains(role)) {
    return _sinkFileLogMap[role]->getLogLevels();
  }
  else {
    return ENABLE_LOG_LEVELS;
  }
}

EnableLogLevels SimpleQtLogger::getLogLevels_console() const
{
  // qDebug("SimpleQtLogger::getLogLevels_console");
  return _sinkConsoleLog->getLogLevels();
}

EnableLogLevels SimpleQtLogger::getLogLevels_qDebug() const
{
  // qDebug("SimpleQtLogger::getLogLevels_qDebug");
  return _sinkQDebugLog->getLogLevels();
}

EnableLogLevels SimpleQtLogger::getLogLevels_signal() const
{
  // qDebug("SimpleQtLogger::getLogLevels_signal");
  return _sinkSignalLog->getLogLevels();
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
bool SimpleQtLogger::addLogFilter_file(const QRegularExpression& re)
{
  // qDebug("SimpleQtLogger::addLogFilter_file");
  return addLogFilter_file("main", re);
}

bool SimpleQtLogger::addLogFilter_file(const QString& role, const QRegularExpression& re)
{
  // qDebug("SimpleQtLogger::addLogFilter_file");
  if(!re.isValid()) {
    return false;
  }
  if(_sinkFileLogMap.contains(role)) {
    return _sinkFileLogMap[role]->addLogFilter(re);
  }
  return false;
}

bool SimpleQtLogger::addLogFilter_console(const QRegularExpression& re)
{
  // qDebug("SimpleQtLogger::addLogFilter_console");
  if(!re.isValid()) {
    return false;
  }
  return _sinkConsoleLog->addLogFilter(re);
}

bool SimpleQtLogger::addLogFilter_qDebug(const QRegularExpression& re)
{
  // qDebug("SimpleQtLogger::addLogFilter_qDebug");
  if(!re.isValid()) {
    return false;
  }
  return _sinkQDebugLog->addLogFilter(re);
}

bool SimpleQtLogger::addLogFilter_signal(const QRegularExpression& re)
{
  // qDebug("SimpleQtLogger::addLogFilter_signal");
  if(!re.isValid()) {
    return false;
  }
  return _sinkSignalLog->addLogFilter(re);
}
#endif

bool SimpleQtLogger::setLogFileName(const QString& logFileName, unsigned int logFileRotationSize, unsigned int logFileMaxNumber)
{
  // qDebug("SimpleQtLogger::setLogFileName");
  return setLogFileName("main", logFileName, logFileRotationSize, logFileMaxNumber);
}

bool SimpleQtLogger::setLogFileName(const QString& role, const QString& logFileName, unsigned int logFileRotationSize, unsigned int logFileMaxNumber)
{
  // qDebug("SimpleQtLogger::setLogFileName");
  if(_sinkFileLogMap.contains(role) && _sinkFileLogMap[role]->setLogFileName(logFileName, logFileRotationSize, logFileMaxNumber)) {
    // log(QString("Start file-log '%1'").arg(role), LogLevel_INTERNAL, "", "", 0);
    return true;
  }
  return false;
}

bool SimpleQtLogger::connectSinkSignalLog(const QObject* receiver, const char* method)
{
  // qDebug("SimpleQtLogger::connectSinkSignalLog");
  return QObject::connect(_sinkSignalLog, SIGNAL(signalLogMessage(simpleqtlogger::LogLevel, const QString&)), receiver, method);
}

QString SimpleQtLogger::timeStamp()
{
  // qDebug("SimpleQtLogger::timeStamp");

  // time-stamp
  QDateTime dateTime = QDateTime::currentDateTime(); // or better use QDateTime::currentDateTimeUtc() instead
  return dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
}

QString SimpleQtLogger::threadId()
{
  // qDebug("SimpleQtLogger::threadId");

  // thread-id in hexadecimal
  return QString("%1").arg((unsigned long int)QThread::currentThreadId(),16,16,QLatin1Char('0')); // field-with for 64bit
}

void SimpleQtLogger::log(const QString& text, LogLevel logLevel, const QString& functionName, const char* fileName, unsigned int lineNumber)
{
  // qDebug("SimpleQtLogger::log");

  // thread-safe

  emit signalLog(timeStamp(), threadId(), text, logLevel, functionName, fileName, lineNumber);
}

#if ENABLE_SQTL_LOG_LEVEL_FUNCTION > 0
void SimpleQtLogger::logFuncBegin(const QString& text, const QString& functionName, const QString& fileName, unsigned int lineNumber)
{
  // qDebug("SimpleQtLogger::logFuncBegin");

  // thread-safe

  if(!ENABLE_FUNCTION_STACK_TRACE) {
    log(QString("%1").arg(text), LogLevel_FUNCTION, functionName, fileName.toStdString().c_str(), lineNumber);
    return;
  }

  unsigned int stackDepthThread;
  {
    QMutexLocker locker(&_mutex);
    // adjust stack-trace depth (++ before log)
    unsigned int& value = _stackDepth[(unsigned long int)QThread::currentThreadId()];
    stackDepthThread = ++value;
  }

  QString stackDepth("");
  for(unsigned int i=1; i<stackDepthThread; ++i) {
    stackDepth += STACK_DEPTH_CHAR;
  }
  if(text.isEmpty()) {
    log(QString("%1\\").arg(stackDepth), LogLevel_FUNCTION, functionName, fileName.toStdString().c_str(), lineNumber);
  }
  else {
    log(QString("%1\\ %2").arg(stackDepth).arg(text), LogLevel_FUNCTION, functionName, fileName.toStdString().c_str(), lineNumber);
  }
}

void SimpleQtLogger::logFuncEnd(const QString& text, const QString& functionName, const QString& fileName, unsigned int lineNumber)
{
  // qDebug("SimpleQtLogger::logFuncEnd");

  // thread-safe

  if(!ENABLE_FUNCTION_STACK_TRACE) {
    return;
  }

  unsigned int stackDepthThread;
  {
    QMutexLocker locker(&_mutex);
    // adjust stack-trace depth (-- after log)
    unsigned int& value = _stackDepth[(unsigned long int)QThread::currentThreadId()];
    stackDepthThread = value--;
  }

  QString stackDepth("");
  for(unsigned int i=1; i<stackDepthThread; ++i) {
    stackDepth += STACK_DEPTH_CHAR;
  }
  if(text.isEmpty()) {
    log(QString("%1/").arg(stackDepth), LogLevel_FUNCTION, functionName, fileName.toStdString().c_str(), lineNumber);
  }
  else {
    log(QString("%1/ %2").arg(stackDepth).arg(text), LogLevel_FUNCTION, functionName, fileName.toStdString().c_str(), lineNumber);
  }
}
#endif

// -------------------------------------------------------------------------------------------------

#if ENABLE_SQTL_LOG_LEVEL_FUNCTION > 0
SimpleQtLoggerFunc::SimpleQtLoggerFunc(const QString& text, const QString& functionName, const QString& fileName, unsigned int lineNumber)
  : _text(text)
  , _functionName(functionName)
  , _fileName(fileName)
  , _lineNumber(lineNumber)
{
  // qDebug("SimpleQtLoggerFunc::SimpleQtLoggerFunc");
  if(ENABLE_SQTL_LOG_LEVEL_FUNCTION && ENABLE_LOG_LEVELS.logLevel_FUNCTION) SimpleQtLogger::getInstance()->logFuncBegin(_text, _functionName, _fileName, _lineNumber);
}

SimpleQtLoggerFunc::~SimpleQtLoggerFunc()
{
  // qDebug("SimpleQtLoggerFunc::~SimpleQtLoggerFunc");
  if(ENABLE_SQTL_LOG_LEVEL_FUNCTION && ENABLE_LOG_LEVELS.logLevel_FUNCTION) SimpleQtLogger::getInstance()->logFuncEnd(_text, _functionName, _fileName, _lineNumber);
}
#endif

} // namespace simpleqtlogger
