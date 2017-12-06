/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef binder_LOGGER_H_
#define binder_LOGGER_H_

#include <boost/optional.hpp>

#include <memory.h>
#include <string>

#include "binderd/common/do_not_copy_or_move.h"
#include "binderd/common/utils.h"

namespace binderd {
// A Logger enables persisting of messages describing & explaining the
// state of the system.
class Logger : public common::DoNotCopyOrMove {
 public:
  // Severity enumerates all known severity levels
  // applicable to log messages.
  enum class Severity { kTrace,
                        kDebug,
                        kInfo,
                        kWarning,
                        kError,
                        kFatal };

  // A Location describes the origin of a log message.
  struct Location {
    std::string file;      // The name of the file that contains the log message.
    std::string function;  // The function that contains the log message.
    std::uint32_t line;    // The line in file that resulted in the log message.
  };

  virtual void Init(const Severity& severity = Severity::kWarning) = 0;

  bool SetSeverityFromString(const std::string &severity);
  virtual void SetSeverity(const Severity& severity) = 0;

  virtual void Log(Severity severity, const std::string& message,
                   const boost::optional<Location>& location) = 0;

  virtual void Trace(
      const std::string& message,
      const boost::optional<Location>& location = boost::optional<Location>{});
  virtual void Debug(
      const std::string& message,
      const boost::optional<Location>& location = boost::optional<Location>{});
  virtual void Info(
      const std::string& message,
      const boost::optional<Location>& location = boost::optional<Location>{});
  virtual void Warning(
      const std::string& message,
      const boost::optional<Location>& location = boost::optional<Location>{});
  virtual void Error(
      const std::string& message,
      const boost::optional<Location>& location = boost::optional<Location>{});
  virtual void Fatal(
      const std::string& message,
      const boost::optional<Location>& location = boost::optional<Location>{});

  template <typename... T>
  void Tracef(const boost::optional<Location>& location,
              const std::string& pattern, T&&... args) {
    Trace(utils::string_format(pattern, std::forward<T>(args)...), location);
  }

  template <typename... T>
  void Debugf(const boost::optional<Location>& location,
              const std::string& pattern, T&&... args) {
    Debug(utils::string_format(pattern, std::forward<T>(args)...), location);
  }

  template <typename... T>
  void Infof(const boost::optional<Location>& location,
             const std::string& pattern, T&&... args) {
    Info(utils::string_format(pattern, std::forward<T>(args)...), location);
  }

  template <typename... T>
  void Warningf(const boost::optional<Location>& location,
                const std::string& pattern, T&&... args) {
    Warning(utils::string_format(pattern, std::forward<T>(args)...), location);
  }

  template <typename... T>
  void Errorf(const boost::optional<Location>& location,
              const std::string& pattern, T&&... args) {
    Error(utils::string_format(pattern, std::forward<T>(args)...), location);
  }

  template <typename... T>
  void Fatalf(const boost::optional<Location>& location,
              const std::string& pattern, T&&... args) {
    Fatal(utils::string_format(pattern, std::forward<T>(args)...), location);
  }

 protected:
  Logger() = default;
};

// operator<< inserts severity into out.
std::ostream& operator<<(std::ostream& out, Logger::Severity severity);

// operator<< inserts location into out.
std::ostream& operator<<(std::ostream& out, const Logger::Location& location);

// Log returns the mcs-wide configured logger instance.
// Save to call before/after main.
Logger& Log();
// SetLog installs the given logger as mcs-wide default logger.
void SetLogger(const std::shared_ptr<Logger>& logger);
}

#define TRACE(...)     \
  binderd::Log().Tracef( \
      binderd::Logger::Location{__FILE__, __FUNCTION__, __LINE__}, __VA_ARGS__)
#define DEBUG(...)     \
  binderd::Log().Debugf( \
      binderd::Logger::Location{__FILE__, __FUNCTION__, __LINE__}, __VA_ARGS__)
#define INFO(...)     \
  binderd::Log().Infof( \
      binderd::Logger::Location{__FILE__, __FUNCTION__, __LINE__}, __VA_ARGS__)
#define WARNING(...)     \
  binderd::Log().Warningf( \
      binderd::Logger::Location{__FILE__, __FUNCTION__, __LINE__}, __VA_ARGS__)
#define ERROR(...)     \
  binderd::Log().Errorf( \
      binderd::Logger::Location{__FILE__, __FUNCTION__, __LINE__}, __VA_ARGS__)
#define FATAL(...)     \
  binderd::Log().Fatalf( \
      binderd::Logger::Location{__FILE__, __FUNCTION__, __LINE__}, __VA_ARGS__)

#endif
