//==----------- common.cpp -----------------------------------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <sycl/detail/common.hpp>
#include <sycl/detail/common_info.hpp>
#include <detail/xpti_registry.hpp>

namespace sycl {
__SYCL_INLINE_VER_NAMESPACE(_V1) {
namespace detail {

/// Event to be used by SYCL API layer related activities
xpti_td *GAPICallEvent = nullptr;

const char *stringifyErrorCode(pi_int32 error) {
  switch (error) {
#define _PI_ERRC(NAME, VAL)                                                    \
  case NAME:                                                                   \
    return #NAME;
#define _PI_ERRC_WITH_MSG(NAME, VAL, MSG)                                      \
  case NAME:                                                                   \
    return MSG;
#include <sycl/detail/pi_error.def>
#undef _PI_ERRC
#undef _PI_ERRC_WITH_MSG

  default:
    return "Unknown error code";
  }
}

std::vector<std::string> split_string(const std::string &str, char delimeter) {
  std::vector<std::string> result;
  size_t beg = 0;
  size_t length = 0;
  for (const auto &x : str) {
    if (x == delimeter) {
      result.push_back(str.substr(beg, length));
      beg += length + 1;
      length = 0;
      continue;
    }
    length++;
  }
  if (length != 0) {
    result.push_back(str.substr(beg, length));
  }
  return result;
}

// Implementation of the SYCL API call tracing methods that use XPTI
// framework to emit these traces that will be used by tools.
uint64_t emitFunctionBeginTrace(const char *FName) {
  uint64_t CorrelationID = 0;
#ifdef XPTI_ENABLE_INSTRUMENTATION
  // The function_begin and function_end trace point types are defined to
  // trace library API calls and they are currently enabled here for support
  // tools that need the API scope. The methods emitFunctionBeginTrace() and
  // emitFunctionEndTrace() can be extended to also trace the arguments of the
  // SYCL API call using a trace point type the extends the predefined trace
  // point types.
  //
  // You can use the sample collector in llvm/xptifw/samples/syclpi_collector
  // to print the API traces and also extend them to support  arguments that
  // may be traced later.
  //
  /// Example Usage:
  /// \code{cpp}
  /// // Two diagnostic trace types defined for function begin and function end
  /// // with different semantics than the one in the default trace type list.
  /// typedef enum {
  ///   diagnostic_func_begin = XPTI_TRACE_POINT_BEGIN(0),
  ///   diagnostic_func_end = XPTI_TRACE_POINT_END(0),
  /// }syclpi_extension_t;
  /// ...
  /// uint16_t pi_func_begin =
  ///     xptiRegisterUserDefinedTracePoint("sycl.api", func_begin);
  /// uint16_t pi_func_end =
  ///     xptiRegisterUserDefinedTracePoint("sycl.api", func_end);
  /// ...
  /// // Setup argument data for the function being traced
  /// ...
  /// xptiNotifySubscribers(stream_id, pi_func_begin, parent, event, instance,
  ///                       (void *)argument_data);
  /// \endcode
  if (xptiTraceEnabled()) {
    uint8_t StreamID = xptiRegisterStream(SYCL_APIDEBUGCALL_STREAM_NAME);
    CorrelationID = xptiGetUniqueId();
    xptiNotifySubscribers(
        StreamID, (uint16_t)xpti::trace_point_type_t::function_begin,
        GAPICallEvent, nullptr, CorrelationID, static_cast<const void *>(FName));
  }
#endif // XPTI_ENABLE_INSTRUMENTATION
  return CorrelationID;
}

void emitFunctionEndTrace(uint64_t CorrelationID, const char *FName) {
#ifdef XPTI_ENABLE_INSTRUMENTATION
  if (xptiTraceEnabled()) {
    // CorrelationID is the unique ID that ties together a function_begin and
    // function_end pair of trace calls. The splitting of a scoped_notify into
    // two function calls incurs an additional overhead as the StreamID must
    // be looked up twice.
    uint8_t StreamID = xptiRegisterStream(SYCL_APIDEBUGCALL_STREAM_NAME);
    xptiNotifySubscribers(
        StreamID, (uint16_t)xpti::trace_point_type_t::function_end,
        GAPICallEvent, nullptr, CorrelationID, static_cast<const void *>(FName));
  }
#endif // XPTI_ENABLE_INSTRUMENTATION
}



} // namespace detail
} // __SYCL_INLINE_VER_NAMESPACE(_V1)
} // namespace sycl
