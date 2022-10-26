//==----------- ze_trace_collector.cpp -------------------------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

/// \file ze_trace_collector.cpp
/// Routines to collect and print Level Zero API calls.

#include "xpti/xpti_trace_framework.h"

#include <sycl/detail/spinlock.hpp>

#include <level_zero/zet_api.h>

#include <iostream>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>

extern sycl::detail::SpinLock GlobalLock;

extern bool HasSYPrinter;
extern bool HasPIPrinter;

enum class ZEApiKind {
#define _ZE_API(call, domain, cb, params_type) call,
#include "../../plugins/level_zero/ze_api.def"
#undef _ZE_API
};

bool PrintSyVerbose = false;

XPTI_CALLBACK_API void syCallback(uint16_t TraceType,
                                  xpti::trace_event_data_t * /*Parent*/,
                                  xpti::trace_event_data_t * /*Event*/,
                                  uint64_t /*Instance*/, const void *UserData) {
  std::lock_guard<sycl::detail::SpinLock> _{GlobalLock};
  if (TraceType == xpti::trace_function_begin) {
    std::cout << "[SYCL]" << static_cast<const char*>(UserData) << "{\n";
    std::cout << std::flush;
  } else if (TraceType == xpti::trace_function_end) {
    std::cout << "} (" << static_cast<const char*>(UserData) << ") ---> require exception check"
              << std::endl;
    std::cout << std::endl;
  }
}

void syPrintersInit() {
  HasSYPrinter = true;

  std::string_view PrinterType(std::getenv("SYCL_TRACE_PRINT_FORMAT"));
  if (PrinterType == "classic") {
    std::cerr << "Classic output is not supported yet for SYCL API\n";
  } else if (PrinterType == "verbose") {
    PrintSyVerbose = true;
  } else if (PrinterType == "compact") {
    PrintSyVerbose = false;
  }
}

// For unification purpose
void syPrintersFinish() {}
