#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include "core/FaultPatternProfilingPass.h"
#include "core/ProfilingPass.h"
#include "core/GenLLFIIndexPass.h"
#include "core/FaultInjectionPass.h"
#include "core/LLFIDotGraphPass.h"
#include "core/InstTracePass.h"

using namespace llvm;

namespace llfi {

  //-----------------------------------------------------------------------------
  // New PM Registration
  //-----------------------------------------------------------------------------
  llvm::PassPluginLibraryInfo getLLFIPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "llfi_passes", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {

              // For GenLLFIIndexPass
              PB.registerPipelineParsingCallback(
                  [](StringRef Name, ModulePassManager &MPM,
                     ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "genllfiindexpass") {
                      MPM.addPass(llfi::GenLLFIIndexPass());
                      return true;
                    }
                    return false;
                  });

              // For ProfilingPass
              PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "profilingpass") {
                    MPM.addPass(llfi::ProfilingPass());
                    return true;
                  }
                  return false;
                });

              PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "patternprofilingpass") {
                    MPM.addPass(llfi::ProfilingPass());
                    return true;
                  }
                  return false;
                });
                
              // For FaultInjectionPass
              PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "faultinjectionpass") {
                    MPM.addPass(llfi::NewFaultInjectionPass());
                    return true;
                  }
                  return false;
                });

              // For DotGraphPass
              PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "dotgraphpass") {
                    MPM.addPass(llfi::NewLLFIDotGraph());
                    return true;
                  }
                  return false;
                });

              // For InstructionTracePass
              PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "insttracepass") {
                    MPM.addPass(llfi::NewInstTrace());
                    return true;
                  }
                  return false;
                });
            }};
  }

  extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
  llvmGetPassPluginInfo() {
    return getLLFIPassPluginInfo();
  }
}
