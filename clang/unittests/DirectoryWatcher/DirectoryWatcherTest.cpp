//===- unittests/DirectoryWatcher/DirectoryWatcherTest.cpp ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "clang/DirectoryWatcher/DirectoryWatcher.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Mutex.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include "gtest/gtest.h"
#include <condition_variable>
#include <future>
#include <mutex>
#include <thread>

using namespace llvm;
using namespace llvm::sys;
using namespace llvm::sys::fs;
using namespace clang;

namespace clang {
static bool operator==(const DirectoryWatcher::Event &lhs,
                       const DirectoryWatcher::Event &rhs) {
  return lhs.Filename == rhs.Filename &&
         static_cast<int>(lhs.Kind) == static_cast<int>(rhs.Kind);
}
} // namespace clang

namespace {

typedef DirectoryWatcher::Event::EventKind EventKind;

struct DirectoryWatcherTestFixture {
  std::string TestRootDir;
  std::string TestWatchedDir;

  DirectoryWatcherTestFixture() {
    SmallString<128> pathBuf;
#ifndef NDEBUG
    std::error_code UniqDirRes =
#endif
    createUniqueDirectory("dirwatcher", pathBuf);
    assert(!UniqDirRes);
    TestRootDir = pathBuf.str();
    path::append(pathBuf, "watch");
    TestWatchedDir = pathBuf.str();
#ifndef NDEBUG
    std::error_code CreateDirRes =
#endif
    create_directory(TestWatchedDir, false);
    assert(!CreateDirRes);
  }

  ~DirectoryWatcherTestFixture() { remove_directories(TestRootDir); }

  SmallString<128> getPathInWatched(const std::string &testFile) {
    SmallString<128> pathBuf;
    pathBuf = TestWatchedDir;
    path::append(pathBuf, testFile);
    return pathBuf;
  }

  void addFile(const std::string &testFile) {
    Expected<file_t> ft = openNativeFileForWrite(getPathInWatched(testFile),
                                                 CD_CreateNew, OF_None);
    if (ft) {
      closeFile(*ft);
    } else {
      llvm::errs() << llvm::toString(ft.takeError()) << "\n";
      llvm::errs() << getPathInWatched(testFile) << "\n";
      llvm_unreachable("Couldn't create test file.");
    }
  }

  void deleteFile(const std::string &testFile) {
    std::error_code EC =
        remove(getPathInWatched(testFile), /*IgnoreNonExisting=*/false);
    ASSERT_FALSE(EC);
  }
};

std::string eventKindToString(const EventKind K) {
  switch (K) {
  case EventKind::Removed:
    return "Removed";
  case EventKind::Modified:
    return "Modified";
  case EventKind::WatchedDirRemoved:
    return "WatchedDirRemoved";
  case EventKind::WatcherGotInvalidated:
    return "WatcherGotInvalidated";
  }
  llvm_unreachable("unknown event kind");
}

struct VerifyingConsumer {
  std::vector<DirectoryWatcher::Event> ExpectedInitial;
  const std::vector<DirectoryWatcher::Event> ExpectedInitialCopy;
  std::vector<DirectoryWatcher::Event> ExpectedNonInitial;
  const std::vector<DirectoryWatcher::Event> ExpectedNonInitialCopy;
  std::vector<DirectoryWatcher::Event> OptionalNonInitial;
  std::vector<DirectoryWatcher::Event> UnexpectedInitial;
  std::vector<DirectoryWatcher::Event> UnexpectedNonInitial;
  std::mutex Mtx;
  std::condition_variable ResultIsReady;

  VerifyingConsumer(
      const std::vector<DirectoryWatcher::Event> &ExpectedInitial,
      const std::vector<DirectoryWatcher::Event> &ExpectedNonInitial,
      const std::vector<DirectoryWatcher::Event> &OptionalNonInitial = {})
      : ExpectedInitial(ExpectedInitial), ExpectedInitialCopy(ExpectedInitial),
        ExpectedNonInitial(ExpectedNonInitial), ExpectedNonInitialCopy(ExpectedNonInitial),
        OptionalNonInitial(OptionalNonInitial) {}

  // This method is used by DirectoryWatcher.
  void consume(DirectoryWatcher::Event E, bool IsInitial) {
    if (IsInitial)
      consumeInitial(E);
    else
      consumeNonInitial(E);
  }

  void consumeInitial(DirectoryWatcher::Event E) {
    std::unique_lock<std::mutex> L(Mtx);
    auto It = std::find(ExpectedInitial.begin(), ExpectedInitial.end(), E);
    if (It == ExpectedInitial.end()) {
      UnexpectedInitial.push_back(E);
    } else {
      ExpectedInitial.erase(It);
    }
    if (result()) {
      L.unlock();
      ResultIsReady.notify_one();
    }
  }

  void consumeNonInitial(DirectoryWatcher::Event E) {
    std::unique_lock<std::mutex> L(Mtx);
    auto It =
        std::find(ExpectedNonInitial.begin(), ExpectedNonInitial.end(), E);
    if (It == ExpectedNonInitial.end()) {
      auto OptIt =
          std::find(OptionalNonInitial.begin(), OptionalNonInitial.end(), E);
      if (OptIt != OptionalNonInitial.end()) {
        OptionalNonInitial.erase(OptIt);
      } else {
        UnexpectedNonInitial.push_back(E);
      }
    } else {
      ExpectedNonInitial.erase(It);
    }
    if (result()) {
      L.unlock();
      ResultIsReady.notify_one();
    }
  }

  // This method is used by DirectoryWatcher.
  void consume(llvm::ArrayRef<DirectoryWatcher::Event> Es, bool IsInitial) {
    for (const auto &E : Es)
      consume(E, IsInitial);
  }

  // Not locking - caller has to lock Mtx.
  llvm::Optional<bool> result() const {
    if (ExpectedInitial.empty() && ExpectedNonInitial.empty() &&
        UnexpectedInitial.empty() && UnexpectedNonInitial.empty())
      return true;
    if (!UnexpectedInitial.empty() || !UnexpectedNonInitial.empty())
      return false;
    return llvm::None;
  }

  // This method is used by tests.
  // \returns true on success
  bool blockUntilResult() {
    std::unique_lock<std::mutex> L(Mtx);
    while (true) {
      if (result())
        return *result();

      ResultIsReady.wait(L, [this]() { return result().hasValue(); });
    }
    return false; // Just to make compiler happy.
  }

  void printUnmetExpectations(llvm::raw_ostream &OS) {
    // If there was any issue, print the expected state
    if (
      !ExpectedInitial.empty()
      ||
      !ExpectedNonInitial.empty()
      ||
      !UnexpectedInitial.empty()
      ||
      !UnexpectedNonInitial.empty()
    ) {
      OS << "Expected initial events: \n";
      for (const auto &E : ExpectedInitialCopy) {
        OS << eventKindToString(E.Kind) << " " << E.Filename << "\n";
      }
      OS << "Expected non-initial events: \n";
      for (const auto &E : ExpectedNonInitialCopy) {
        OS << eventKindToString(E.Kind) << " " << E.Filename << "\n";
      }
    }

    if (!ExpectedInitial.empty()) {
      OS << "Expected but not seen initial events: \n";
      for (const auto &E : ExpectedInitial) {
        OS << eventKindToString(E.Kind) << " " << E.Filename << "\n";
      }
    }
    if (!ExpectedNonInitial.empty()) {
      OS << "Expected but not seen non-initial events: \n";
      for (const auto &E : ExpectedNonInitial) {
        OS << eventKindToString(E.Kind) << " " << E.Filename << "\n";
      }
    }
    if (!UnexpectedInitial.empty()) {
      OS << "Unexpected initial events seen: \n";
      for (const auto &E : UnexpectedInitial) {
        OS << eventKindToString(E.Kind) << " " << E.Filename << "\n";
      }
    }
    if (!UnexpectedNonInitial.empty()) {
      OS << "Unexpected non-initial events seen: \n";
      for (const auto &E : UnexpectedNonInitial) {
        OS << eventKindToString(E.Kind) << " " << E.Filename << "\n";
      }
    }
  }
};

void checkEventualResultWithTimeout(VerifyingConsumer &TestConsumer) {
  std::packaged_task<int(void)> task(
      [&TestConsumer]() { return TestConsumer.blockUntilResult(); });
  std::future<int> WaitForExpectedStateResult = task.get_future();
  std::thread worker(std::move(task));
  worker.detach();

  EXPECT_TRUE(WaitForExpectedStateResult.wait_for(std::chrono::seconds(3)) ==
              std::future_status::ready)
      << "The expected result state wasn't reached before the time-out.";
  std::unique_lock<std::mutex> L(TestConsumer.Mtx);
  EXPECT_TRUE(TestConsumer.result().hasValue());
  if (TestConsumer.result().hasValue()) {
    EXPECT_TRUE(*TestConsumer.result());
  }
  if ((TestConsumer.result().hasValue() && !TestConsumer.result().getValue()) ||
      !TestConsumer.result().hasValue())
    TestConsumer.printUnmetExpectations(llvm::outs());
}
} // namespace

TEST(DirectoryWatcherTest, InitialScanSync) {
  DirectoryWatcherTestFixture fixture;

  fixture.addFile("a");
  fixture.addFile("b");
  fixture.addFile("c");

  VerifyingConsumer TestConsumer{
      {{EventKind::Modified, "a"},
       {EventKind::Modified, "b"},
       {EventKind::Modified, "c"}},
      {},
      // We have to ignore these as it's a race between the test process
      // which is scanning the directory and kernel which is sending
      // notification.
      {{EventKind::Modified, "a"},
       {EventKind::Modified, "b"},
       {EventKind::Modified, "c"}}
      };

  auto DW = DirectoryWatcher::create(
      fixture.TestWatchedDir,
      [&TestConsumer](llvm::ArrayRef<DirectoryWatcher::Event> Events,
                      bool IsInitial) {
        TestConsumer.consume(Events, IsInitial);
      },
      /*waitForInitialSync=*/true);
  if (!DW) return;

  checkEventualResultWithTimeout(TestConsumer);
}

TEST(DirectoryWatcherTest, InitialScanAsync) {
  DirectoryWatcherTestFixture fixture;

  fixture.addFile("a");
  fixture.addFile("b");
  fixture.addFile("c");

  VerifyingConsumer TestConsumer{
      {{EventKind::Modified, "a"},
       {EventKind::Modified, "b"},
       {EventKind::Modified, "c"}},
      {},
      // We have to ignore these as it's a race between the test process
      // which is scanning the directory and kernel which is sending
      // notification.
      {{EventKind::Modified, "a"},
       {EventKind::Modified, "b"},
       {EventKind::Modified, "c"}}
       };

  auto DW = DirectoryWatcher::create(
      fixture.TestWatchedDir,
      [&TestConsumer](llvm::ArrayRef<DirectoryWatcher::Event> Events,
                      bool IsInitial) {
        TestConsumer.consume(Events, IsInitial);
      },
      /*waitForInitialSync=*/false);
  if (!DW) return;

  checkEventualResultWithTimeout(TestConsumer);
}

TEST(DirectoryWatcherTest, AddFiles) {
  DirectoryWatcherTestFixture fixture;

  VerifyingConsumer TestConsumer{
      {},
      {{EventKind::Modified, "a"},
       {EventKind::Modified, "b"},
       {EventKind::Modified, "c"}}};

  auto DW = DirectoryWatcher::create(
      fixture.TestWatchedDir,
      [&TestConsumer](llvm::ArrayRef<DirectoryWatcher::Event> Events,
                      bool IsInitial) {
        TestConsumer.consume(Events, IsInitial);
      },
      /*waitForInitialSync=*/true);
  if (!DW) return;

  fixture.addFile("a");
  fixture.addFile("b");
  fixture.addFile("c");

  checkEventualResultWithTimeout(TestConsumer);
}

TEST(DirectoryWatcherTest, ModifyFile) {
  DirectoryWatcherTestFixture fixture;

  fixture.addFile("a");

  VerifyingConsumer TestConsumer{
      {{EventKind::Modified, "a"}},
      {{EventKind::Modified, "a"}},
      {{EventKind::Modified, "a"}}};

  auto DW = DirectoryWatcher::create(
      fixture.TestWatchedDir,
      [&TestConsumer](llvm::ArrayRef<DirectoryWatcher::Event> Events,
                      bool IsInitial) {
        TestConsumer.consume(Events, IsInitial);
      },
      /*waitForInitialSync=*/true);
  if (!DW) return;

  // modify the file
  {
    std::error_code error;
    llvm::raw_fd_ostream bStream(fixture.getPathInWatched("a"), error,
                                 CD_OpenExisting);
    assert(!error);
    bStream << "foo";
  }

  checkEventualResultWithTimeout(TestConsumer);
}

TEST(DirectoryWatcherTest, DeleteFile) {
  DirectoryWatcherTestFixture fixture;

  fixture.addFile("a");

  VerifyingConsumer TestConsumer{
      {{EventKind::Modified, "a"}},
      {{EventKind::Removed, "a"}},
      {{EventKind::Modified, "a"}, {EventKind::Removed, "a"}}};

  auto DW = DirectoryWatcher::create(
      fixture.TestWatchedDir,
      [&TestConsumer](llvm::ArrayRef<DirectoryWatcher::Event> Events,
                      bool IsInitial) {
        TestConsumer.consume(Events, IsInitial);
      },
      /*waitForInitialSync=*/true);
  if (!DW) return;

  fixture.deleteFile("a");

  checkEventualResultWithTimeout(TestConsumer);
}

TEST(DirectoryWatcherTest, DeleteWatchedDir) {
  DirectoryWatcherTestFixture fixture;

  VerifyingConsumer TestConsumer{
      {},
      {{EventKind::WatchedDirRemoved, ""},
       {EventKind::WatcherGotInvalidated, ""}}};

  auto DW = DirectoryWatcher::create(
      fixture.TestWatchedDir,
      [&TestConsumer](llvm::ArrayRef<DirectoryWatcher::Event> Events,
                      bool IsInitial) {
        TestConsumer.consume(Events, IsInitial);
      },
      /*waitForInitialSync=*/true);
  if (!DW) return;

  remove_directories(fixture.TestWatchedDir);

  checkEventualResultWithTimeout(TestConsumer);
}

TEST(DirectoryWatcherTest, InvalidatedWatcher) {
  DirectoryWatcherTestFixture fixture;

  VerifyingConsumer TestConsumer{
      {}, {{EventKind::WatcherGotInvalidated, ""}}};

  {
    auto DW = DirectoryWatcher::create(
        fixture.TestWatchedDir,
        [&TestConsumer](llvm::ArrayRef<DirectoryWatcher::Event> Events,
                        bool IsInitial) {
          TestConsumer.consume(Events, IsInitial);
        },
        /*waitForInitialSync=*/true);
    if (!DW) return;
  } // DW is destructed here.

  checkEventualResultWithTimeout(TestConsumer);
}