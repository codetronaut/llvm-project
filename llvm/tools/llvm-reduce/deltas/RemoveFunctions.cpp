//===- llvm-reduce.cpp - The LLVM Delta Reduction utility -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file is a Specialized Delta Pass, which removes the functions that are
// not in the provided function-chunks.
//
//===----------------------------------------------------------------------===//

#include "RemoveFunctions.h"

using namespace llvm;

/// Removes all the Defined Functions (as well as their calls)
/// that aren't inside any of the desired Chunks.
/// @returns the Module stripped of out-of-chunk functions
std::unique_ptr<Module>
RemoveFunctions::extractChunksFromModule(std::vector<Chunk> ChunksToKeep,
                                         Module *Program) {
  std::unique_ptr<Module> Clone = CloneModule(*Program);

  // Get functions inside desired chunks
  std::set<Function *> FuncsToKeep;
  int I = 0, FunctionCount = 1;
  for (auto &F : *Clone) {
    if (!F.isDeclaration()) {
      if (FunctionCount >= ChunksToKeep[I].begin &&
          FunctionCount <= ChunksToKeep[I].end) {
        FuncsToKeep.insert(&F);
      }
      if (FunctionCount >= ChunksToKeep[I].end)
        ++I;
      ++FunctionCount;
    }
  }

  // Delete out-of-chunk functions, and replace their calls with undef
  std::vector<Function *> FuncsToRemove;
  for (auto &F : *Clone) {
    if (!F.isDeclaration() && !FuncsToKeep.count(&F)) {
      F.replaceAllUsesWith(UndefValue::get(F.getType()));
      FuncsToRemove.push_back(&F);
    }
  }
  for (auto *F : FuncsToRemove)
    F->eraseFromParent();

  // Delete instructions with undef calls
  std::vector<Instruction *> InstToRemove;
  for (auto &F : *Clone)
    for (auto &BB : F)
      for (auto &I : BB)
        if (auto *Call = dyn_cast<CallInst>(&I))
          if (!Call->getCalledFunction()) {
            // Instruction might be stored / used somewhere else
            I.replaceAllUsesWith(UndefValue::get(I.getType()));
            InstToRemove.push_back(&I);
          }

  for (auto *I : InstToRemove)
    I->eraseFromParent();

  return Clone;
}

/// Counts the amount of non-declaration functions and prints their
/// respective index & name
int RemoveFunctions::getTargetCount(Module *Program) {
  // TODO: Silence index with --quiet flag
  outs() << "----------------------------\n";
  outs() << "Chunk Index Reference:\n";
  int FunctionCount = 0;
  for (auto &F : *Program)
    if (!F.isDeclaration()) {
      ++FunctionCount;
      outs() << "\t" << FunctionCount << ": " << F.getName() << "\n";
    }
  outs() << "----------------------------\n";
  return FunctionCount;
}
