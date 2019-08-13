#ifndef CFLDUMMYALIASANALYSIS_H
#define CFLDUMMYALIASANALYSIS_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Optional.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CFLAliasAnalysisUtils.h"
#include "llvm/Analysis/MemoryLocation.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include <forward_list>
#include <memory>
#include <iostream>

namespace llvm {

class TargetLibraryInfo;

namespace cflaa {

struct AliasSummary;

}


class CFLDummyAAResult : public AAResultBase<CFLDummyAAResult> {
  friend AAResultBase<CFLDummyAAResult>;

public:
  explicit CFLDummyAAResult(const TargetLibraryInfo &TLI);
  CFLDummyAAResult(CFLDummyAAResult &&Arg);
  ~CFLDummyAAResult();

  const cflaa::AliasSummary *getAliasSummary(Function &Fn);

  void scan(Function *Fn);

  AliasResult query(const MemoryLocation &LocA, const MemoryLocation &LocB);
  AliasResult alias(const MemoryLocation &LocA, const MemoryLocation &LocB,
                    AAQueryInfo &AAQI) {
      std::cout << "CFLDummyAAResult::alias(Mem, Mem, AAQuery)" << std::endl;
      AliasResult QueryResult = query(LocA, LocB);
      return QueryResult;
  }

private:
  const TargetLibraryInfo &TLI;
};

class CFLDummyAA : public AnalysisInfoMixin<CFLDummyAA> {
  friend AnalysisInfoMixin<CFLDummyAA>;

  static AnalysisKey Key;

public:
  using Result = CFLDummyAAResult;

  CFLDummyAAResult run(Function &F, FunctionAnalysisManager &AM);
};

class CFLDummyAAWrapperPass : public ImmutablePass {
  std::unique_ptr<CFLDummyAAResult> Result;
public:
  static char ID;

  CFLDummyAAWrapperPass();

  CFLDummyAAResult &getResult() { return *Result; }
  const CFLDummyAAResult &getResult() const { return *Result; }

  void initializePass() override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;

};

ImmutablePass *createCFLDummyAAWrapperPass();

void initializeCFLDummyAAWrapperPassPass(PassRegistry &Registry);

}


#endif // CFLDUMMYALIASANALYSIS_H
