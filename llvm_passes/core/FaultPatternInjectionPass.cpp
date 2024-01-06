//===- faultinjectionpass.cpp - Fault Injection Pass -==//
//
//                     LLFI Distribution
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This pass instruments selected registers of instruction with calls to fault
// injection libraries
//
// The fault injection function is a C function which performs the fault
// injection at runtime
// See faultinjection_lib.c injectFunc() function for more details on the 
// fault injection function. This function definition is linked to the 
// instrumented bitcode file (after this pass). 
//===----------------------------------------------------------------------===//
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InlineAsm.h"

#include <vector>

#include "FaultPatternInjectionPass.h"
#include "Controller.h"
#include "Utils.h"


// ------------------------------------------------------

namespace llfi {

bool FaultPatternInjectionPass::runOnModule(Module &M) {
  checkforMainFunc(M);

  std::map<Instruction*, std::list< int >* > *fi_inst_regs_map;
  Controller *ctrl = Controller::getInstance(M);
  ctrl->getFIInstRegsMap(&fi_inst_regs_map);
  fprintf(stderr, "(_____)\n");
  insertInjectionFuncCall(fi_inst_regs_map, M);

  finalize(M);
  return true;
}

bool FaultPatternInjectionPass::runOnModule(Module &M) {
  LLVMContext &context = M.getContext();

  Function* mainfunc = M.getFunction("main_graph");
  std::vector<Instruction*> faultPatternInjectionCalls;

  if (mainfunc) {
    // Loop to find all calls to LLTFIInjectFault and LLTFIInjectFaultMatMul
    // functions.
    for (BasicBlock &bb : *mainfunc) {
      for (BasicBlock::const_iterator i = bb.begin(); i != bb.end(); ++i) {

        Instruction* inst = const_cast<llvm::Instruction*>(&*i);

        if (inst->getOpcode() == Instruction::Call){

          CallInst* callinst = dyn_cast<CallInst>(inst);
          // If this the LLTFI's fault pattern injection function.
          if ((callinst->getCalledFunction())->getName() == "LLTFIInjectFault"){
						(callinst->getCalledFunction())->setName("LLTFIInjectFaultProf");
            faultPatternInjectionCalls.push_back(inst);
					}

					if ((callinst->getCalledFunction())->getName() == "LLTFIInjectFaultMatMul"){
						(callinst->getCalledFunction())->setName("LLTFIInjectFaultMatMulProf");
            faultPatternInjectionCalls.push_back(inst);
					}
        }
      }
    }

    // function declaration
    FunctionCallee profilingfunc = getLLFILibProfilingFunc(M);

    // Loop over all fault pattern injection call instructions.
    for(Instruction* inst : faultPatternInjectionCalls) {

        Instruction *insertptr = inst->getNextNode();

        // prepare for the calling argument and call the profiling function
        std::vector<Value*> profilingarg(1);
        const IntegerType* itype = IntegerType::get(context, 32);

        //LLVM 3.3 Upgrading
        IntegerType* itype_non_const = const_cast<IntegerType*>(itype);
        Value* opcode = ConstantInt::get(itype_non_const, 0);
        profilingarg[0] = opcode;
        ArrayRef<Value*> profilingarg_array_ref(profilingarg);

        CallInst::Create(profilingfunc, profilingarg_array_ref,
                         "", insertptr);
    }
  }

  addEndProfilingFuncCall(M);
  return true;
}

void FaultPatternInjectionPass::addEndProfilingFuncCall(Module &M) {
  Function* mainfunc = M.getFunction("main");
  if (mainfunc != NULL) {
    FunctionCallee endprofilefunc = getLLFILibEndProfilingFunc(M);

    // function call
    std::set<Instruction*> exitinsts;
    getProgramExitInsts(M, exitinsts);
    assert (exitinsts.size() != 0
            && "Program does not have explicit exit point");

    for (std::set<Instruction*>::iterator it = exitinsts.begin();
         it != exitinsts.end(); ++it) {
      Instruction *term = *it;
      CallInst::Create(endprofilefunc, "", term);
    }
  } else {
    errs() << "ERROR: Function main does not exist, " <<
        "which is required by LLFI\n";
    exit(1);
  }
}

FunctionCallee FaultPatternInjectionPass::getLLFILibProfilingFunc(Module &M) {
  LLVMContext &context = M.getContext();
  std::vector<Type*> paramtypes(1);
  paramtypes[0] = Type::getInt32Ty(context);

  ArrayRef<Type*> paramtypes_array_ref(paramtypes);

  FunctionType* profilingfunctype = FunctionType::get(
      Type::getVoidTy(context), paramtypes_array_ref, false);
  FunctionCallee profilingfunc =
      M.getOrInsertFunction("doProfilingFaultPattern", profilingfunctype);
  return profilingfunc;
}

FunctionCallee FaultPatternInjectionPass::getLLFILibEndProfilingFunc(Module &M) {
  LLVMContext& context = M.getContext();
  FunctionType* endprofilingfunctype = FunctionType::get(
      Type::getVoidTy(context), false);
  FunctionCallee endprofilefunc =
      M.getOrInsertFunction("endProfiling", endprofilingfunctype);
  return endprofilefunc;
}

static RegisterPass<FaultPatternInjectionPass> X("FaultPatternInjectionPass",
                                     "Fault pattern Profiling pass", false, false);

}