/*
  Simple Logger for Qt

  Mario Ban, 05.2015
  https://github.com/Mokolea/SimpleQtLogger

  Design goals - simple:
   - straightforward usage
   - single source-file
   - no configuration-file

  Facts:
   - supported sinks:
      - rolling file appender
      - console, colored (ANSI escape codes)
      - qDebug
      - signal (forwarding)
   - log-levels, function-log (stack-trace)
   - log-filters using regular expressions (needs Qt 5.0)
   - thread-safe use of log-macros
   - specify log-format
   - log-file encoding: utf-8

  Restrictions:
   - log-file name has to end with: ".log"
   - just one instance per supported sink (except file-log)
   - just one set of predefined log-macros to be used, logging on all sinks

  Usage:
   - moc has to be applied
   - create logger instance in main (set qApp as parent object) and initialize (example):
      simpleqtlogger::SimpleQtLogger::createInstance(qApp)->setLogFileName("testSimpleQtLogger.log", 10*1000*1000, 20);
     or:
      simpleqtlogger::SimpleQtLogger::createInstance(qApp)->setLogFormat_file("<TS> [<LL>] <TEXT> (<FUNC>@<FILE>:<LINE>)", "<TS> [<LL>] <TEXT>");
      simpleqtlogger::SimpleQtLogger::getInstance()->setLogFileName(QDir::home().filePath("Documents/Qt/testSimpleQtLoggerGui.log"), 10*1000, 10);
     or with thread-id (default):
      simpleqtlogger::SimpleQtLogger::createInstance(qApp)->setLogFormat_file("<TS> [<TID>] [<LL>] <TEXT> (<FUNC>@<FILE>:<LINE>)", "<TS> [<TID>] [<LL>] <TEXT>");
      simpleqtlogger::SimpleQtLogger::getInstance()->setLogFileName(QDir::home().filePath("Documents/Qt/testSimpleQtLoggerGui.log"), 10*1000, 10);
   - enable sinks:
      simpleqtlogger::ENABLE_LOG_SINK_FILE = true;
      simpleqtlogger::ENABLE_LOG_SINK_CONSOLE = false;
      simpleqtlogger::ENABLE_LOG_SINK_QDEBUG = false;
      simpleqtlogger::ENABLE_LOG_SINK_SIGNAL = false;
   - set log-features:
      simpleqtlogger::ENABLE_FUNCTION_STACK_TRACE = true;
      simpleqtlogger::ENABLE_CONSOLE_COLOR = true;
   - set log-levels (global):
      simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_INFO = true;
      simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_DEBUG = false;
      simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_FUNCTION = true;
      simpleqtlogger::SimpleQtLogger::getInstance()->setLogLevels_file(simpleqtlogger::ENABLE_LOG_LEVELS);
   - set log-filters using regular expressions (needs Qt 5.0):
      simpleqtlogger::SimpleQtLogger::getInstance()->addLogFilter_file(QRegularExpression("..."));
   - set main task (widget) as parent object for the logger instance (example):
      simpleqtlogger::SimpleQtLogger::getInstance()->setParent(task);
   - see also main.cpp in examples, especially for how to use multiple log-files

  Info:
   - log-level (enble/disable) on 3 layer: compile-time (hard), run-time (soft, global log-macros), run-time per sink
   - a message is only logged if the log-level is enabled in all 3 layers

  Log-format:
  The following TAGs are available and expand to:
   - <TS> --> Time-stamp, "YYYY-MM-DD HH:MM:SS.SSS"
   - <TID> --> Thread-Id, 64bit value in hexadecimal
   - <TID32> --> Thread-Id, 32bit value in hexadecimal
   - <LL> --> Log-level, one character: '!', 'E', 'W', 'N', 'I', 'D' or 'F'
   - <TEXT> --> The log-message
   - <FUNC> --> Function-name
   - <FILE> --> File-name
   - <LINE> --> Line-number

  TODO:
   - enhance log-format (TAG <TEXT>) by adding a flexible layout configurable with pattern string
   - trimming (removing of whitespace) selectable, at least for at the start of log-message
   - idea: have messages optional prefixed with current stack-depth "...|" if ENABLE_FUNCTION_STACK_TRACE enabled --> maybe not
   - provide to add/remove user defined sinks
   - optimize function-log macro to not create a stack-object according to ENABLE_FUNCTION_STACK_TRACE
   - enable pedantic-errors, check for no compiler warnings, use e.g.: -Wall -Wextra -Werror -pedantic-errors -Wwrite-strings
   - maybe allow message-buffering, processing on idle-time
   - maybe flush periodically on idle-time
   - maybe do file rolling (check file size periodically) on idle-time
   - maybe do all file-operations in worker-thread

  Done:
   - log forwarding by emitting a Qt signal
   - think about environment variable to specify log-file directory, os independent solution
      --> do it outside of SimpleQtLogger by using the QProcessEnvironment class
   - have multiple log-files (sink rolling file appender) with different log-levels, rotation-size, ...
   - set (enable/disable) log-levels per sink
   - add different set of log macros to compose messages using stream operator
   - continuous integration using Travis CI: created a .travis.yml

  Tested using:
   - Qt 5.6.0 (Community Open Source), Clang 7.3 (Apple) 64 bit
   - Qt 5.4.2 (Community Open Source), Clang 6.0 (Apple) 64 bit
   - Qt 4.8.6 Debian 8, gcc version 4.9.2 (Debian 4.9.2-10), 64 bit
   - Travis CI: Qt 4.8.1 x86_64-linux-gnu, gcc Ubuntu/Linaro 4.6.3-1ubuntu5
   - Qt 5.5.0, Visual Studio 2013, 32 bit

  GNU Lesser General Public License v2.1
  Copyright (C) 2017 Mario Ban
*/

#ifndef _SIMPLE_QT_LOGGER_H
#define _SIMPLE_QT_LOGGER_H

#include <QObject>
#include <QString>
#include <QTextStream>
#include <QFile>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QMap>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QRegularExpression>
#endif

#define SQTL_VERSION_STR   "1.2.0"
#define SQTL_VERSION       0x010200   // Version is: (major << 16) + (minor << 8) + patch
// SQTL_VERSION_CHECK can be used like: #if (SQTL_VERSION >= SQTL_VERSION_CHECK(1, 1, 0))
#define SQTL_VERSION_CHECK(major,minor,patch)   ((major<<16)|(minor<<8)|(patch))

// Log-sinks (hard; adjust at pre-processor, compile-time)
#define ENABLE_SQTL_LOG_SINK_FILE      1   // 1: enable, 0: disable; log to file (rolling)
#define ENABLE_SQTL_LOG_SINK_CONSOLE   1   // 1: enable, 0: disable; log to console (colored log-levels)
#define ENABLE_SQTL_LOG_SINK_QDEBUG    0   // 1: enable, 0: disable; log using qDebug; messages are sent to the console, if it is a console application
#define ENABLE_SQTL_LOG_SINK_SIGNAL    1   // 1: enable, 0: disable; log forwarding by emitting a Qt signal

// Log-level (hard; adjust at pre-processor, compile-time)
#define ENABLE_SQTL_LOG_LEVEL_FATAL      1   // 1: enable, 0: disable
#define ENABLE_SQTL_LOG_LEVEL_ERROR      1   // 1: enable, 0: disable
#define ENABLE_SQTL_LOG_LEVEL_WARNING    1   // 1: enable, 0: disable
#define ENABLE_SQTL_LOG_LEVEL_NOTE       1   // 1: enable, 0: disable
#define ENABLE_SQTL_LOG_LEVEL_INFO       1   // 1: enable, 0: disable
#define ENABLE_SQTL_LOG_LEVEL_DEBUG      1   // 1: enable, 0: disable; just for step-by-step testing
#define ENABLE_SQTL_LOG_LEVEL_FUNCTION   1   // 1: enable, 0: disable; stack-trace

namespace simpleqtlogger {

const char STACK_DEPTH_CHAR = '.'; // use e.g. ' ' or '.'
const unsigned short CHECK_LOG_FILE_ACTIVITY_INTERVAL = 5000; // [ms]

const QString DEFAULT_LOG_FORMAT          = "<TS> [<TID>] [<LL>] <TEXT> (<FUNC>@<FILE>:<LINE>)";
const QString DEFAULT_LOG_FORMAT_INTERNAL = "<TS> [<TID>] [<LL>] <TEXT>"; // sink file-log: following TAGs are not processed: <FUNC>, <FILE>, <LINE>

const QString DEFAULT_LOG_FORMAT_CONSOLE = "<TEXT>"; // sink console-log: output is prefixed with log-level name (intense color, see ..._I): "<LOG-LEVEL-LABEL>: "
const QString DEFAULT_LOG_FORMAT_CONSOLE_FUNCTION_SUFFIX = " (<FUNC>)"; // appended to DEFAULT_LOG_FORMAT_CONSOLE for LogLevel_FUNCTION

const QString CONSOLE_LOG_LEVEL_LABEL_FATAL   = "FATAL";
const QString CONSOLE_LOG_LEVEL_LABEL_ERROR   = "ERROR";
const QString CONSOLE_LOG_LEVEL_LABEL_WARNING = "WARNING";
const QString CONSOLE_LOG_LEVEL_LABEL_NOTE    = "NOTE";
const QString CONSOLE_LOG_LEVEL_LABEL_DEBUG   = "DEBUG";

// ANSI escape codes to set text colors (foreground/background), http://en.wikipedia.org/wiki/ANSI_escape_code
#if 0
const QString CONSOLE_COLOR_ANSI_ESC_CODES_FATAL     = "\033[0;33m";   // foreground yellow
const QString CONSOLE_COLOR_ANSI_ESC_CODES_FATAL_I   = "\033[0;33;1m"; // foreground yellow (intense)
const QString CONSOLE_COLOR_ANSI_ESC_CODES_ERROR     = "\033[0;31m";   // foreground red
const QString CONSOLE_COLOR_ANSI_ESC_CODES_ERROR_I   = "\033[0;31;1m"; // foreground red (intense)
const QString CONSOLE_COLOR_ANSI_ESC_CODES_WARNING   = "\033[0;36m";   // foreground cyan
const QString CONSOLE_COLOR_ANSI_ESC_CODES_WARNING_I = "\033[0;36;1m"; // foreground cyan (intense)
const QString CONSOLE_COLOR_ANSI_ESC_CODES_NOTE_I    = "\033[0;37;1m"; // foreground white (intense)
const QString CONSOLE_COLOR_ANSI_ESC_CODES_DEBUG     = "\033[0;35m";   // foreground magenta
const QString CONSOLE_COLOR_ANSI_ESC_CODES_DEBUG_I   = "\033[0;35;1m"; // foreground magenta (intense)
const QString CONSOLE_COLOR_ANSI_ESC_CODES_FUNCTION  = "\033[0;32m";   // foreground green
#endif
#if 0
const QString CONSOLE_COLOR_ANSI_ESC_CODES_FATAL     = "\033[0;40;33m";   // background black / foreground yellow
const QString CONSOLE_COLOR_ANSI_ESC_CODES_FATAL_I   = "\033[0;40;33;1m"; // background black / foreground yellow (intense)
const QString CONSOLE_COLOR_ANSI_ESC_CODES_ERROR     = "\033[0;47;31m";   // background white / foreground red
const QString CONSOLE_COLOR_ANSI_ESC_CODES_ERROR_I   = "\033[0;47;31;1m"; // background white / foreground red (intense)
const QString CONSOLE_COLOR_ANSI_ESC_CODES_WARNING   = "\033[0;40;36m";   // background black / foreground cyan
const QString CONSOLE_COLOR_ANSI_ESC_CODES_WARNING_I = "\033[0;40;36;1m"; // background black / foreground cyan (intense)
const QString CONSOLE_COLOR_ANSI_ESC_CODES_NOTE_I    = "\033[0;40;37;1m"; // background black / foreground white (intense)
const QString CONSOLE_COLOR_ANSI_ESC_CODES_DEBUG     = "\033[0;47;35m";   // background white / foreground magenta
const QString CONSOLE_COLOR_ANSI_ESC_CODES_DEBUG_I   = "\033[0;47;35;1m"; // background white / foreground magenta (intense)
const QString CONSOLE_COLOR_ANSI_ESC_CODES_FUNCTION  = "\033[0;40;32m";   // background black / foreground green
#endif
#if 1
const QString CONSOLE_COLOR_ANSI_ESC_CODES_FATAL     = "\033[0;33m";      // background -     / foreground yellow
const QString CONSOLE_COLOR_ANSI_ESC_CODES_FATAL_I   = "\033[0;40;33;1m"; // background black / foreground yellow (intense)
const QString CONSOLE_COLOR_ANSI_ESC_CODES_ERROR     = "\033[0;31m";      // background -     / foreground red
const QString CONSOLE_COLOR_ANSI_ESC_CODES_ERROR_I   = "\033[0;47;31;1m"; // background white / foreground red (intense)
const QString CONSOLE_COLOR_ANSI_ESC_CODES_WARNING   = "\033[0;36m";      // background -     / foreground cyan
const QString CONSOLE_COLOR_ANSI_ESC_CODES_WARNING_I = "\033[0;40;36;1m"; // background black / foreground cyan (intense)
const QString CONSOLE_COLOR_ANSI_ESC_CODES_NOTE_I    = "\033[0;40;37;1m"; // background black / foreground white (intense)
const QString CONSOLE_COLOR_ANSI_ESC_CODES_DEBUG     = "\033[0;35m";      // background -     / foreground magenta
const QString CONSOLE_COLOR_ANSI_ESC_CODES_DEBUG_I   = "\033[0;47;35;1m"; // background white / foreground magenta (intense)
const QString CONSOLE_COLOR_ANSI_ESC_CODES_FUNCTION  = "\033[0;32m";      // background -     / foreground green
#endif
#if 0
// Xterm, KDE's Konsole, as well as all libvte based terminals (including GNOME Terminal) support ISO-8613-3 24-bit foreground and background color setting
const QString CONSOLE_COLOR_ANSI_ESC_CODES_FATAL     = "\033[0;38;2;205;205;0m";   // foreground yellow
const QString CONSOLE_COLOR_ANSI_ESC_CODES_FATAL_I   = "\033[0;38;2;255;255;0m";   // foreground yellow (intense)
const QString CONSOLE_COLOR_ANSI_ESC_CODES_ERROR     = "\033[0;38;2;205;0;0m";     // foreground red
const QString CONSOLE_COLOR_ANSI_ESC_CODES_ERROR_I   = "\033[0;38;2;255;0;0m";     // foreground red (intense)
const QString CONSOLE_COLOR_ANSI_ESC_CODES_WARNING   = "\033[0;38;2;0;205;205m";   // foreground cyan
const QString CONSOLE_COLOR_ANSI_ESC_CODES_WARNING_I = "\033[0;38;2;0;255;255m";   // foreground cyan (intense)
const QString CONSOLE_COLOR_ANSI_ESC_CODES_NOTE_I    = "\033[0;38;2;255;255;255m"; // foreground white (intense)
const QString CONSOLE_COLOR_ANSI_ESC_CODES_DEBUG     = "\033[0;38;2;205;0;205m";   // foreground magenta
const QString CONSOLE_COLOR_ANSI_ESC_CODES_DEBUG_I   = "\033[0;38;2;255;0;255m";   // foreground magenta (intense)
const QString CONSOLE_COLOR_ANSI_ESC_CODES_FUNCTION  = "\033[0;38;2;0;205;0m";     // foreground green
#endif
const QString CONSOLE_COLOR_ANSI_ESC_CODES_RESET     = "\033[0m";      // reset all attributes

// Log-level
typedef enum {
  LogLevel_FATAL = 0, // Fatal error, the program execution has to be aborted
  LogLevel_ERROR,     // An error, that challenges the core operation
  LogLevel_WARNING,   // A warning, signalizing a deformity, without challenging the core operation
  LogLevel_NOTE,      // Analysis information directed to supporters (same as INFO, just label NOTE in console-log)
  LogLevel_INFO,      // Analysis information directed to supporters
  LogLevel_DEBUG,     // Analysis debug information directed to developers
  LogLevel_FUNCTION,  // A trace level for function stack-tracing
  LogLevel_INTERNAL   // Internal messages (start, file rolling, ...)
}
LogLevel;

static const char LOG_LEVEL_CHAR[8] = {'!', 'E', 'W', 'N', 'I', 'D', 'F', '-'}; // MUST correspond to enum LogLevel, unchecked array!!!

// Log-sinks (adjust at run-time)
extern bool ENABLE_LOG_SINK_FILE;    // Log-sink: true: enable, false: disable, default: true
extern bool ENABLE_LOG_SINK_CONSOLE; // Log-sink: true: enable, false: disable, default: false
extern bool ENABLE_LOG_SINK_QDEBUG;  // Log-sink: true: enable, false: disable, default: false
extern bool ENABLE_LOG_SINK_SIGNAL;  // Log-sink: true: enable, false: disable, default: false

// Log-level (adjust at run-time)
struct EnableLogLevels {
  bool logLevel_FATAL;    // Log-level: true: enable, false: disable, default: true
  bool logLevel_ERROR;    // Log-level: true: enable, false: disable, default: true
  bool logLevel_WARNING;  // Log-level: true: enable, false: disable, default: true
  bool logLevel_NOTE;     // Log-level: true: enable, false: disable, default: true
  bool logLevel_INFO;     // Log-level: true: enable, false: disable, default: true
  bool logLevel_DEBUG;    // Log-level: true: enable, false: disable, default: false; just for step-by-step testing
  bool logLevel_FUNCTION; // Log-level: true: enable, false: disable, default: false; stack-trace
  bool logLevel_INTERNAL; // Log-level: true: enable, false: disable, default: true
  EnableLogLevels();
  bool enabled(LogLevel logLevel) const;
};
extern EnableLogLevels ENABLE_LOG_LEVELS;

// Log-function stack-trace
extern bool ENABLE_FUNCTION_STACK_TRACE; // Log-function stack-trace: true: enable, false: disable, default: true

// Sink console color
extern bool ENABLE_CONSOLE_COLOR; // Color for sink console: true: enable, false: disable, default: true
// Sink console trimmed messages
extern bool ENABLE_CONSOLE_TRIMMED; // Whitespaces trimmed for sink console: true: enable, false: disable, default: true

// Console
extern bool ENABLE_CONSOLE_LOG_FILE_STATE; // Console output log-file state: true: enable, false: disable, default: true

// Microsoft Visual C++ compiler specific
#if defined(_MSC_VER)
#define SQTL_MSVC_WARNING_SUPPRESS \
  __pragma(warning(push)) \
  __pragma(warning(disable:4127))   /* suppress "do { ... } while(0)" warning */
#define SQTL_MSVC_WARNING_RESTORE \
  __pragma(warning(pop))
#else
#define SQTL_MSVC_WARNING_SUPPRESS   /* nop */
#define SQTL_MSVC_WARNING_RESTORE    /* nop */
#endif

// Macro body
#define SQTL_L_BODY(text,levelEnabledHard,levelEnabledSoft,level) \
  SQTL_MSVC_WARNING_SUPPRESS \
  do { if(levelEnabledHard && levelEnabledSoft) simpleqtlogger::SimpleQtLogger::getInstance()->log(text, level, __FUNCTION__, __FILE__, __LINE__); } while(0) \
  SQTL_MSVC_WARNING_RESTORE

// Use these macros (thread-safe) to have function-, filename and linenumber set correct
#define L_FATAL(text)   SQTL_L_BODY(text,ENABLE_SQTL_LOG_LEVEL_FATAL,simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_FATAL,simpleqtlogger::LogLevel_FATAL)
#define L_ERROR(text)   SQTL_L_BODY(text,ENABLE_SQTL_LOG_LEVEL_ERROR,simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_ERROR,simpleqtlogger::LogLevel_ERROR)
#define L_WARN(text)    SQTL_L_BODY(text,ENABLE_SQTL_LOG_LEVEL_WARNING,simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_WARNING,simpleqtlogger::LogLevel_WARNING)
#define L_NOTE(text)    SQTL_L_BODY(text,ENABLE_SQTL_LOG_LEVEL_NOTE,simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_NOTE,simpleqtlogger::LogLevel_NOTE)
#define L_INFO(text)    SQTL_L_BODY(text,ENABLE_SQTL_LOG_LEVEL_INFO,simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_INFO,simpleqtlogger::LogLevel_INFO)
#define L_DEBUG(text)   SQTL_L_BODY(text,ENABLE_SQTL_LOG_LEVEL_DEBUG,simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_DEBUG,simpleqtlogger::LogLevel_DEBUG)
#if ENABLE_SQTL_LOG_LEVEL_FUNCTION > 0
#define L_FUNC(text)    simpleqtlogger::SimpleQtLoggerFunc _simpleQtLoggerFunc_(text, __FUNCTION__, __FILE__, __LINE__)
#else
#define L_FUNC(text)    /* nop */
#endif

// Macro body
#define SQTL_LS_BODY(text,levelEnabledHard,levelEnabledSoft,level) \
  SQTL_MSVC_WARNING_SUPPRESS \
  do { if(levelEnabledHard && levelEnabledSoft) { QString s; QTextStream ts(&s); ts << text; \
    simpleqtlogger::SimpleQtLogger::getInstance()->log(s, level, __FUNCTION__, __FILE__, __LINE__); } } while(0) \
  SQTL_MSVC_WARNING_RESTORE

// Support use of streaming operators
#define LS_FATAL(text)   SQTL_LS_BODY(text,ENABLE_SQTL_LOG_LEVEL_FATAL,simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_FATAL,simpleqtlogger::LogLevel_FATAL)
#define LS_ERROR(text)   SQTL_LS_BODY(text,ENABLE_SQTL_LOG_LEVEL_ERROR,simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_ERROR,simpleqtlogger::LogLevel_ERROR)
#define LS_WARN(text)    SQTL_LS_BODY(text,ENABLE_SQTL_LOG_LEVEL_WARNING,simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_WARNING,simpleqtlogger::LogLevel_WARNING)
#define LS_NOTE(text)    SQTL_LS_BODY(text,ENABLE_SQTL_LOG_LEVEL_NOTE,simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_NOTE,simpleqtlogger::LogLevel_NOTE)
#define LS_INFO(text)    SQTL_LS_BODY(text,ENABLE_SQTL_LOG_LEVEL_INFO,simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_INFO,simpleqtlogger::LogLevel_INFO)
#define LS_DEBUG(text)   SQTL_LS_BODY(text,ENABLE_SQTL_LOG_LEVEL_DEBUG,simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_DEBUG,simpleqtlogger::LogLevel_DEBUG)
#if ENABLE_SQTL_LOG_LEVEL_FUNCTION > 0
#define LS_FUNC(text)    QString _s_; { QTextStream ts(&_s_); ts << text; } simpleqtlogger::SimpleQtLoggerFunc _simpleQtLoggerFunc_(_s_, __FUNCTION__, __FILE__, __LINE__)
#else
#define LS_FUNC(text)    /* nop */
#endif

// -------------------------------------------------------------------------------------------------

class Sink : public QObject
{
  Q_OBJECT

public:
  explicit Sink(QObject *parent);
  virtual ~Sink();

  void setLogFormat(const QString& logFormat, const QString& logFormatInt);
  void setLogLevels(const EnableLogLevels& enableLogLevels);
  EnableLogLevels getLogLevels() const;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
  bool addLogFilter(const QRegularExpression& re);
#endif

protected:
  QString getLogFormat() const;
  QString getLogFormatInt() const;
  bool checkLogLevelsEnabled(LogLevel logLevel) const;
  bool checkFilter(const QString& text) const;

private:
  // implicitly implemented, not to be used
  Sink(const Sink&);
  Sink& operator=(const Sink&);

  QString _logFormat;
  QString _logFormatInt;
  EnableLogLevels _enableLogLevels;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
  QList<QRegularExpression> _reList;
#endif
};

// -------------------------------------------------------------------------------------------------

class SinkFileLog : public Sink
{
  Q_OBJECT

public:
  explicit SinkFileLog(QObject *parent, const QString& role);
  virtual ~SinkFileLog();

  bool setLogFileName(const QString& logFileName, unsigned int logFileRotationSize, unsigned int logFileMaxNumber);

private slots:
  void slotLog(const QString& ts, const QString& tid, const QString& text, LogLevel logLevel, const QString& functionName, const QString& fileName, unsigned int lineNumber);
  void slotCheckLogFileActivity();

private:
  // implicitly implemented, not to be used
  SinkFileLog(const SinkFileLog&);
  SinkFileLog& operator=(const SinkFileLog&);

  bool checkLogFileOpen();
  void checkLogFileRolling();

  const QString _role;
  QString _logFileName;
  unsigned int _logFileRotationSize; // [bytes] initiate log-file rolling
  unsigned int _logFileMaxNumber; // max number of rolling log-file history, range 1..99

  QFile* _logFile;
  bool _logFileActivity; // track log-file write (append) activity
  bool _startMessage;
};

// -------------------------------------------------------------------------------------------------

class SinkConsoleLog : public Sink
{
  Q_OBJECT

public:
  explicit SinkConsoleLog(QObject *parent);
  virtual ~SinkConsoleLog();

private slots:
  void slotLog(const QString& ts, const QString& tid, const QString& text, LogLevel logLevel, const QString& functionName, const QString& fileName, unsigned int lineNumber);

private:
  // implicitly implemented, not to be used
  SinkConsoleLog(const SinkConsoleLog&);
  SinkConsoleLog& operator=(const SinkConsoleLog&);
};

// -------------------------------------------------------------------------------------------------

class SinkQDebugLog : public Sink
{
  Q_OBJECT

public:
  explicit SinkQDebugLog(QObject *parent);
  virtual ~SinkQDebugLog();

private slots:
  void slotLog(const QString& ts, const QString& tid, const QString& text, LogLevel logLevel, const QString& functionName, const QString& fileName, unsigned int lineNumber);

private:
  // implicitly implemented, not to be used
  SinkQDebugLog(const SinkQDebugLog&);
  SinkQDebugLog& operator=(const SinkQDebugLog&);
};

// -------------------------------------------------------------------------------------------------

class SinkSignalLog : public Sink
{
  Q_OBJECT

public:
  explicit SinkSignalLog(QObject *parent);
  virtual ~SinkSignalLog();

private slots:
  void slotLog(const QString& ts, const QString& tid, const QString& text, LogLevel logLevel, const QString& functionName, const QString& fileName, unsigned int lineNumber);

signals:
  void signalLogMessage(simpleqtlogger::LogLevel logLevel, const QString& logMessage);

private:
  // implicitly implemented, not to be used
  SinkSignalLog(const SinkSignalLog&);
  SinkSignalLog& operator=(const SinkSignalLog&);
};

// -------------------------------------------------------------------------------------------------

class SimpleQtLogger : public QObject
{
  Q_OBJECT

public:
  static SimpleQtLogger* createInstance(QObject *parent);
  static SimpleQtLogger* getInstance(); // may return NULL pointer!
  virtual ~SimpleQtLogger();

  void addSinkFileLog(const QString& role); // main is already added

  void setLogFormat_file(const QString& logFormat, const QString& logFormatInt); // main
  void setLogFormat_file(const QString& role, const QString& logFormat, const QString& logFormatInt);
  void setLogFormat_console(const QString& logFormat, const QString& logFormatInt);
  void setLogFormat_qDebug(const QString& logFormat, const QString& logFormatInt);
  void setLogFormat_signal(const QString& logFormat, const QString& logFormatInt);

  void setLogLevels_file(const EnableLogLevels& enableLogLevels); // main
  void setLogLevels_file(const QString& role, const EnableLogLevels& enableLogLevels);
  void setLogLevels_console(const EnableLogLevels& enableLogLevels);
  void setLogLevels_qDebug(const EnableLogLevels& enableLogLevels);
  void setLogLevels_signal(const EnableLogLevels& enableLogLevels);

  EnableLogLevels getLogLevels_file() const; // main
  EnableLogLevels getLogLevels_file(const QString& role) const;
  EnableLogLevels getLogLevels_console() const;
  EnableLogLevels getLogLevels_qDebug() const;
  EnableLogLevels getLogLevels_signal() const;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
  bool addLogFilter_file(const QRegularExpression& re); // main
  bool addLogFilter_file(const QString& role, const QRegularExpression& re);
  bool addLogFilter_console(const QRegularExpression& re);
  bool addLogFilter_qDebug(const QRegularExpression& re);
  bool addLogFilter_signal(const QRegularExpression& re);
#endif

  bool setLogFileName(const QString& logFileName, unsigned int logFileRotationSize, unsigned int logFileMaxNumber); // main
  bool setLogFileName(const QString& role, const QString& logFileName, unsigned int logFileRotationSize, unsigned int logFileMaxNumber);

  bool connectSinkSignalLog(const QObject* receiver, const char* method); // You must use the SLOT() macro when specifying the method, e.g. SLOT(mySlotLog(simpleqtlogger::LogLevel, const QString&))

  static QString timeStamp();
  static QString threadId();

  // log-functions used by log-macros are thread-safe
  void log(const QString& text, LogLevel logLevel, const QString& functionName, const char* fileName, unsigned int lineNumber);
#if ENABLE_SQTL_LOG_LEVEL_FUNCTION > 0
  void logFuncBegin(const QString& text, const QString& functionName, const QString& fileName, unsigned int lineNumber);
  void logFuncEnd(const QString& text, const QString& functionName, const QString& fileName, unsigned int lineNumber);
#endif

signals:
  // internal
  void signalLog(const QString& ts, const QString& tid, const QString& text, LogLevel logLevel, const QString& functionName, const QString& fileName, unsigned int lineNumber);

private:
  explicit SimpleQtLogger(QObject *parent);
  static SimpleQtLogger* instance;
  // implicitly implemented, not to be used
  SimpleQtLogger(const SimpleQtLogger&);
  SimpleQtLogger& operator=(const SimpleQtLogger&);

  SinkConsoleLog* _sinkConsoleLog;
  SinkQDebugLog* _sinkQDebugLog;
  SinkSignalLog* _sinkSignalLog;
  QMap<QString, SinkFileLog*> _sinkFileLogMap;

  QMutex _mutex;
  QMap<unsigned long int, unsigned int> _stackDepth; // current stack-depth per thread-id for function-log
};

// -------------------------------------------------------------------------------------------------

#if ENABLE_SQTL_LOG_LEVEL_FUNCTION > 0

class SimpleQtLoggerFunc
{
public:
  SimpleQtLoggerFunc(const QString& text, const QString& functionName, const QString& fileName, unsigned int lineNumber);
  virtual ~SimpleQtLoggerFunc();

private:
  // implicitly implemented, not to be used
  SimpleQtLoggerFunc(const SimpleQtLoggerFunc&);
  SimpleQtLoggerFunc& operator=(const SimpleQtLoggerFunc&);

  QString _text;
  QString _functionName;
  QString _fileName;
  unsigned int _lineNumber;
};

#endif

// -------------------------------------------------------------------------------------------------

} // namespace simpleqtlogger

#endif // _SIMPLE_QT_LOGGER_H
