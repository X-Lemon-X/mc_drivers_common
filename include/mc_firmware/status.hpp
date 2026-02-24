/*
Copyright (c) 2025 Patryk Dudziński

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
 * Authors: Patryk Dudziński
 */

#pragma once
#include <optional>
#include <string>

/**
 * @file status.hpp
 * @brief  Status class definition.
 */

/**
 * @defgroup Status
 * @brief Return types and function result handling.
 * @{
 */

namespace mcan {

class Status;
/**
 * @brief Macro for returning on error in a single line.
 *
 * This macro is used to return from a function if the result is not OK.
 * usefull to avoid having to write the same check in every function.
 */
#define ARI_RETURN_ON_ERROR(x)                                                           \
  do {                                                                                   \
    Status _x = x.status();                                                              \
    if (!_x.ok())                                                                        \
      return _x;                                                                         \
  } while (false)

/**
 * @brief  Macro for creat new veriable with a value from a result and returning on error
 * in a single line.
 */
#define ARI_ASIGN_OR_RETURN(assign, result)                                              \
  auto _xsar##assign = result;                                                           \
  do {                                                                                   \
    if (!_xsar##assign.ok())                                                             \
      return _xsar##assign.status();                                                     \
  } while (false);                                                                       \
  auto assign = std::move(_xsar##assign.valueOrDie());

/**
 * @brief Macro for assigning a value to a already exisiting veriable from a result and
 * returning on error in a single line.
 *
 */
#define ARI_ASIGN_TO_OR_RETURN(assign, result)                                           \
  do {                                                                                   \
    auto _xsar##assign = result;                                                         \
    if (!_xsar##assign.ok())                                                             \
      return _xsar##assign.status();                                                     \
    assign = std::move(_xsar##assign.valueOrDie());                                      \
  } while (false);

/**
 * @enum StatusCode
 * @brief Enumeration representing various statuses. Inspired by ApacheArrow.
 *
 * @var StatusCode::OK
 * Operation was successful.
 *
 * @var StatusCode::OutOfMemory
 * Operation failed due to lack of memory.
 *
 * @var StatusCode::KeyError
 * Operation failed due to a key error.
 *
 * @var StatusCode::TypeError
 * Operation failed due to a type error.
 *
 * @var StatusCode::Invalid
 * Operation failed due to an invalid operation.
 *
 * @var StatusCode::IOError
 * Operation failed due to an I/O error.
 *
 * @var StatusCode::CapacityError
 * Operation failed due to a capacity error.
 *
 * @var StatusCode::IndexError
 * Operation failed due to an index error.
 *
 * @var StatusCode::Cancelled
 * Operation was cancelled.
 *
 * @var StatusCode::UnknownError
 * Operation failed due to an unknown error.
 *
 * @var StatusCode::NotImplemented
 * Operation is not implemented.
 *
 * @var StatusCode::SerializationError
 * Operation failed due to a serialization error.
 *
 * @var StatusCode::RError
 * Operation failed due to an R error.
 *
 * @var StatusCode::CodeGenError
 * Operation failed due to a code generation error.
 *
 * @var StatusCode::ExpressionValidationError
 * Operation failed due to an expression validation error.
 *
 * @var StatusCode::ExecutionError
 * Operation failed due to an execution error.
 *
 * @var StatusCode::AlreadyExists
 * Operation failed because the object already exists.
 *
 * @var StatusCode::TimeOut
 * Operation failed due to a timeout.
 */
enum class StatusCode : char
{
  OK = 0,
  OutOfMemory = 1,
  KeyError = 2,
  TypeError = 3,
  Invalid = 4,
  IOError = 5,
  CapacityError = 6,
  IndexError = 7,
  Cancelled = 8,
  UnknownError = 9,
  NotImplemented = 10,
  SerializationError = 11,
  RError = 13,
  // Gandiva range of errors
  CodeGenError = 40,
  ExpressionValidationError = 41,
  ExecutionError = 42,
  // Continue generic codes.
  AlreadyExists = 45,
  TimeOut = 46,
};

/**
 * @brief Status class used as return type.
 *
 * This class is used as a return type for functions that can fail. It contains a status
 * code and a message. For easy use, there are static methods for creating common statuses
 * with some messages. The Status can be converted to a Result object that contains the
 * status and the value.
 */
class Status
{
 public:
  Status(const Status& status) = default;

  [[nodiscard]] static Status OK() { return Status(StatusCode::OK, "", "OK"); };

  [[nodiscard]] static Status OK(std::string&& msg)
  {
    return Status(StatusCode::OK, std::move(msg), "OK|");
  };

  [[nodiscard]] static Status OutOfMemory(std::string&& msg)
  {
    return Status(StatusCode::OutOfMemory, std::move(msg), "OutOfMemory|");
  };

  [[nodiscard]] static Status KeyError(std::string&& msg)
  {
    return Status(StatusCode::KeyError, std::move(msg), "KeyError|");
  };

  [[nodiscard]] static Status TypeError(std::string&& msg)
  {
    return Status(StatusCode::TypeError, std::move(msg), "TypeError|");
  };

  [[nodiscard]] static Status Invalid(std::string&& msg)
  {
    return Status(StatusCode::Invalid, std::move(msg), "Invalid|");
  };

  [[nodiscard]] static Status IOError(std::string&& msg)
  {
    return Status(StatusCode::IOError, std::move(msg), "IOError|");
  };

  [[nodiscard]] static Status CapacityError(std::string&& msg)
  {
    return Status(StatusCode::CapacityError, std::move(msg), "CapacityError|");
  };

  [[nodiscard]] static Status IndexError(std::string&& msg)
  {
    return Status(StatusCode::IndexError, std::move(msg), "IndexError|");
  };

  [[nodiscard]] static Status Cancelled(std::string&& msg)
  {
    return Status(StatusCode::Cancelled, std::move(msg), "Cancelled|");
  };

  [[nodiscard]] static Status UnknownError(std::string&& msg)
  {
    return Status(StatusCode::UnknownError, std::move(msg), "UnknownError|");
  };

  [[nodiscard]] static Status NotImplemented(std::string&& msg)
  {
    return Status(StatusCode::NotImplemented, std::move(msg), "NotImplemented|");
  };

  [[nodiscard]] static Status SerializationError(std::string&& msg)
  {
    return Status(StatusCode::SerializationError, std::move(msg), "SerializationError|");
  };

  [[nodiscard]] static Status RError(std::string&& msg)
  {
    return Status(StatusCode::RError, std::move(msg), "RError|");
  };

  [[nodiscard]] static Status CodeGenError(std::string&& msg)
  {
    return Status(StatusCode::CodeGenError, std::move(msg), "CodeGenError|");
  };

  [[nodiscard]] static Status ExpressionValidationError(std::string&& msg)
  {
    return Status(StatusCode::ExpressionValidationError,
                  std::move(msg),
                  "ExpressionValidationError|");
  };

  [[nodiscard]] static Status ExecutionError(std::string&& msg)
  {
    return Status(StatusCode::ExecutionError, std::move(msg), "ExecutionError|");
  };

  [[nodiscard]] static Status AlreadyExists(std::string&& msg)
  {
    return Status(StatusCode::AlreadyExists, std::move(msg), "AlreadyExists|");
  };

  [[nodiscard]] static Status TimeOut(std::string&& msg)
  {
    return Status(StatusCode::TimeOut, std::move(msg), "TimeOut|");
  };

  /// @brief get the status
  /// @return 0 if OK or some error code
  [[nodiscard]] StatusCode status_code() { return _status; };

  [[nodiscard]] Status valueOrDie() { return *this; };

  /// @brief check if the status is OK
  bool ok() { return _status == StatusCode::OK; };

  /// @brief get status from status
  [[nodiscard]] Status& status() { return *this; };

  /// @brief get the message of the status
  [[nodiscard]] const std::string to_string() { return _message; };

  bool operator==(const Status& other) const { return _status == other._status; }

  bool operator==(const StatusCode& other) const { return _status == other; }

  bool operator!=(const Status& other) const { return !(*this == other); }

 private:
  Status(StatusCode status, std::string&& message, std::string&& status_message)
    : _status(status)
    , _message(std::move(status_message + message))
  {
  }
  StatusCode _status;
  std::string _message;
};

/**
 * @brief Result class used as return type.
 *
 * This class is used as a return type for functions that can fail. It contains a status
 * and a value. The Result can be converted to a Status object that contains the status
 * and the message.
 *
 */

template<typename T>
struct Result
{
 public:
  Result(const Status& status)
    : _status(std::move(status)){};

  static Result<T> OK(T&& value) { return Result<T>(std::move(value), Status::OK()); }

  static Result<T> Propagate(T&& value, Status&& status)
  {
    return Result<T>(std::move(value), std::move(status));
  }

  [[nodiscard]] T& valueOrDie() { return _value.value(); }

  [[nodiscard]] Status& status() { return _status; }

  [[nodiscard]] bool ok() { return _status.ok(); }

  Result<T>& operator=(const Status& other) { return Result<T>(other); }

 private:
  Result(T&& value, Status&& status)
    : _value(std::move(value))
    , _status(std::move(status)){};

  std::optional<T> _value;
  // T _value;
  Status _status;
};

} // namespace mcan