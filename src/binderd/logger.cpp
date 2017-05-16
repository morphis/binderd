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

#include <thread>

#include "binderd/common/utils.h"
#include "binderd/logger.h"

#include <boost/filesystem.hpp>

#include <iostream>
#include <iomanip>

namespace {
struct StdLogger : public binderd::Logger {
  StdLogger() : initialized_(false) {}

  void Init(const Severity& severity = Severity::kWarning) override {
    if (initialized_) return;

    severity_ = severity;
    initialized_ = true;
  }

  void SetSeverity(const Severity& severity) override {
    severity_ = severity;
  }

  void Log(Severity severity, const std::string& message,
           const boost::optional<Location>& location) override {

    if (severity < severity_)
      return;

    const auto now = std::chrono::system_clock::now();
    const auto t = std::chrono::system_clock::to_time_t(now);
    std::cout << binderd::utils::string_format(
                   "[%s %s] [%s] %s",
                   severity,
                   std::put_time(std::localtime(&t), "%Y-%m-%d %X"),
                   location.get(), message)
              << std::endl;
  }

 private:
  Severity severity_;
  bool initialized_;
};

std::shared_ptr<binderd::Logger>& MutableInstance() {
  static std::shared_ptr<binderd::Logger> instance{new StdLogger()};
  return instance;
}

void SetInstance(const std::shared_ptr<binderd::Logger>& logger) {
  MutableInstance() = logger;
}
}

namespace binderd {

bool Logger::SetSeverityFromString(const std::string& severity) {
  if (severity == "trace")
    SetSeverity(Severity::kTrace);
  else if (severity == "debug")
    SetSeverity(Severity::kDebug);
  else if (severity == "info")
    SetSeverity(Severity::kInfo);
  else if (severity == "warning")
    SetSeverity(Severity::kWarning);
  else if (severity == "error")
    SetSeverity(Severity::kError);
  else if (severity == "fatal")
    SetSeverity(Severity::kFatal);
  else
    return false;
  return true;
}

void Logger::Trace(const std::string& message,
                   const boost::optional<Location>& location) {
  Log(Severity::kTrace, message, location);
}

void Logger::Debug(const std::string& message,
                   const boost::optional<Location>& location) {
  Log(Severity::kDebug, message, location);
}

void Logger::Info(const std::string& message,
                  const boost::optional<Location>& location) {
  Log(Severity::kInfo, message, location);
}

void Logger::Warning(const std::string& message,
                     const boost::optional<Location>& location) {
  Log(Severity::kWarning, message, location);
}

void Logger::Error(const std::string& message,
                   const boost::optional<Location>& location) {
  Log(Severity::kError, message, location);
}

void Logger::Fatal(const std::string& message,
                   const boost::optional<Location>& location) {
  Log(Severity::kFatal, message, location);
}

std::ostream& operator<<(std::ostream& strm, binderd::Logger::Severity severity) {
  switch (severity) {
    case binderd::Logger::Severity::kTrace:
      return strm << "TT";
    case binderd::Logger::Severity::kDebug:
      return strm << "DD";
    case binderd::Logger::Severity::kInfo:
      return strm << "II";
    case binderd::Logger::Severity::kWarning:
      return strm << "WW";
    case binderd::Logger::Severity::kError:
      return strm << "EE";
    case binderd::Logger::Severity::kFatal:
      return strm << "FF";
    default:
      return strm << static_cast<uint>(severity);
  }
}

std::ostream& operator<<(std::ostream& out, const Logger::Location& location) {
  return out << utils::string_format(
             "%s:%d@%s",
             boost::filesystem::path(location.file).filename().string(),
             location.line, location.function);
}

Logger& Log() { return *MutableInstance(); }

void SetLogger(const std::shared_ptr<Logger>& logger) { SetInstance(logger); }

}  // namespace binderd
