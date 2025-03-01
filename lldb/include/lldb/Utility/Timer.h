//===-- Timer.h -------------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_UTILITY_TIMER_H
#define LLDB_UTILITY_TIMER_H

#include "lldb/lldb-defines.h"
#include "llvm/ADT/ScopeExit.h"
#include "llvm/Support/Chrono.h"
#include "llvm/Support/Signposts.h"
#include <atomic>
#include <cstdint>

namespace llvm {
  class SignpostEmitter;
}

namespace lldb_private {
class Stream;

/// \class Timer Timer.h "lldb/Utility/Timer.h"
/// A timer class that simplifies common timing metrics.

class Timer {
public:
  class Category {
  public:
    explicit Category(const char *category_name);
    llvm::StringRef GetName() { return m_name; }

  private:
    friend class Timer;
    const char *m_name;
    std::atomic<uint64_t> m_nanos;
    std::atomic<uint64_t> m_nanos_total;
    std::atomic<uint64_t> m_count;
    std::atomic<Category *> m_next;

    Category(const Category &) = delete;
    const Category &operator=(const Category &) = delete;
  };

  /// Default constructor.
  Timer(Category &category, const char *format, ...)
      __attribute__((format(printf, 3, 4)));

  /// Destructor
  ~Timer();

  void Dump();

  static void SetDisplayDepth(uint32_t depth);

  static void SetQuiet(bool value);

  static void DumpCategoryTimes(Stream *s);

  static void ResetCategoryTimes();

protected:
  using TimePoint = std::chrono::steady_clock::time_point;
  void ChildDuration(TimePoint::duration dur) { m_child_duration += dur; }

  Category &m_category;
  TimePoint m_total_start;
  TimePoint::duration m_child_duration{0};

  static std::atomic<bool> g_quiet;
  static std::atomic<unsigned> g_display_depth;

private:
  Timer(const Timer &) = delete;
  const Timer &operator=(const Timer &) = delete;
};

llvm::SignpostEmitter &GetSignposts();

} // namespace lldb_private

// Use a format string because LLVM_PRETTY_FUNCTION might not be a string
// literal.
#define LLDB_SCOPED_TIMER()                                                    \
  static ::lldb_private::Timer::Category _cat(LLVM_PRETTY_FUNCTION);           \
  ::lldb_private::Timer _scoped_timer(_cat, "%s", LLVM_PRETTY_FUNCTION);       \
  SIGNPOST_EMITTER_START_INTERVAL(::lldb_private::GetSignposts(),              \
                                  &_scoped_timer, "%s", LLVM_PRETTY_FUNCTION); \
  auto _scoped_signpost = llvm::make_scope_exit([&_scoped_timer]() {           \
    ::lldb_private::GetSignposts().endInterval(&_scoped_timer);                \
  })

#define LLDB_SCOPED_TIMERF(FMT, ...)                                           \
  static ::lldb_private::Timer::Category _cat(LLVM_PRETTY_FUNCTION);           \
  ::lldb_private::Timer _scoped_timer(_cat, FMT, __VA_ARGS__);                 \
  SIGNPOST_EMITTER_START_INTERVAL(::lldb_private::GetSignposts(),              \
                                  &_scoped_timer, FMT, __VA_ARGS__);           \
  auto _scoped_signpost = llvm::make_scope_exit([&_scoped_timer]() {           \
    ::lldb_private::GetSignposts().endInterval(&_scoped_timer);                \
  })

#endif // LLDB_UTILITY_TIMER_H
