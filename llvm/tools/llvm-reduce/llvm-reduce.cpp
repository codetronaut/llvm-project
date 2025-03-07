//===- llvm-reduce.cpp - The LLVM Delta Reduction utility -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This program tries to reduce an IR test case for a given interesting-ness
// test. It runs multiple delta debugging passes in order to minimize the input
// file. It's worth noting that this is a *temporary* tool that will eventually
// be integrated into the bugpoint tool itself.
//
//===----------------------------------------------------------------------===//

#include "DeltaManager.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include <system_error>
#include <vector>

using namespace llvm;

static cl::opt<bool> Help("h", cl::desc("Alias for -help"), cl::Hidden);
static cl::opt<bool> Version("v", cl::desc("Alias for -version"), cl::Hidden);

static cl::opt<std::string> InputFilename(cl::Positional, cl::Required,
                                          cl::desc("<input llvm ll/bc file>"));

static cl::opt<std::string>
    TestFilename("test", cl::Required,
                 cl::desc("Name of the interesting-ness test to be run"));

static cl::list<std::string>
    TestArguments("test-arg", cl::ZeroOrMore,
                  cl::desc("Arguments passed onto the interesting-ness test"));

static cl::opt<std::string>
    OutputFilename("output",
                   cl::desc("Specify the output file. default: reduced.ll"));
static cl::alias OutputFileAlias("o", cl::desc("Alias for -output"),
                                 cl::aliasopt(OutputFilename));

static cl::opt<bool>
    ReplaceInput("in-place",
                 cl::desc("WARNING: This option will replace your input file"
                          "with the reduced version!"));

// Parses IR into a Module and verifies it
static std::unique_ptr<Module> parseInputFile(StringRef Filename,
                                              LLVMContext &Ctxt) {
  SMDiagnostic Err;
  std::unique_ptr<Module> Result = parseIRFile(Filename, Err, Ctxt);
  if (!Result) {
    Err.print("llvm-reduce", errs());
    return Result;
  }

  if (verifyModule(*Result, &errs())) {
    errs() << "Error: " << Filename << " - input module is broken!\n";
    return std::unique_ptr<Module>();
  }

  return Result;
}

/// Gets Current Working Directory and tries to create a Tmp Directory
static SmallString<128> initializeTmpDirectory() {
  SmallString<128> CWD;
  if (std::error_code EC = sys::fs::current_path(CWD)) {
    errs() << "Error getting current directory: " << EC.message() << "!\n";
    exit(1);
  }

  SmallString<128> TmpDirectory;
  sys::path::append(TmpDirectory, CWD, "tmp");
  if (std::error_code EC = sys::fs::create_directory(TmpDirectory))
    errs() << "Error creating tmp directory: " << EC.message() << "!\n";

  return TmpDirectory;
}

int main(int argc, char **argv) {
  InitLLVM X(argc, argv);

  cl::ParseCommandLineOptions(argc, argv, "LLVM automatic testcase reducer.\n");

  LLVMContext Context;
  std::unique_ptr<Module> OriginalProgram =
      parseInputFile(InputFilename, Context);

  // Initialize test environment
  SmallString<128> TmpDirectory = initializeTmpDirectory();
  TestRunner Tester(TestFilename, TestArguments, InputFilename, TmpDirectory);
  Tester.setProgram(std::move(OriginalProgram));

  // Try to reduce code
  runDeltaPasses(Tester);
  StringRef ReducedFilename = sys::path::filename(Tester.getReducedFilepath());

  if (ReducedFilename == InputFilename) {
    outs() << "\nCouldnt reduce input :/\n";
  } else {
    if (ReplaceInput) // In-place
      OutputFilename = InputFilename.c_str();
    else if (OutputFilename.empty())
      OutputFilename = "reduced.ll";
    else
      OutputFilename += ".ll";

    sys::fs::copy_file(Tester.getReducedFilepath(), OutputFilename);
    outs() << "\nDone reducing! Reduced IR to file: " << OutputFilename << "\n";
  }

  return 0;
}
