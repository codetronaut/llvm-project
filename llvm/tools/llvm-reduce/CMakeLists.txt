set(LLVM_LINK_COMPONENTS
  AllTargetsAsmParsers
  AllTargetsCodeGens
  AllTargetsDescs
  AllTargetsInfos
  IRReader
  Support
  Target
  TransformUtils
  )

# Support plugins.
set(LLVM_NO_DEAD_STRIP 1)

add_llvm_tool(llvm-reduce
  llvm-reduce.cpp
  TestRunner.cpp
  deltas/RemoveFunctions.cpp

  DEPENDS
  intrinsics_gen
  )
export_executable_symbols(llvm-reduce)
