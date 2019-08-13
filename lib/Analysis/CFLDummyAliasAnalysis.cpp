#include "llvm/Analysis/CFLDummyAliasAnalysis.h"
#include "CFLGraph.h"
#include "llvm/Support/raw_ostream.h"


using namespace llvm;
using namespace cflaa;

CFLDummyAAResult::CFLDummyAAResult(const TargetLibraryInfo &TLI)
  : AAResultBase(), TLI(TLI) {
}
CFLDummyAAResult::CFLDummyAAResult(CFLDummyAAResult &&Arg)
  : AAResultBase(std::move(Arg)), TLI(Arg.TLI) {}

CFLDummyAAResult::~CFLDummyAAResult() = default;

const AliasSummary *CFLDummyAAResult::getAliasSummary(Function &Fn) {
    return nullptr;
}

void CFLDummyAAResult::scan(Function *Fn) {
    CFLGraphBuilder<CFLDummyAAResult> GraphBuilder(*this, TLI, *Fn);
    auto &Graph = GraphBuilder.getCFLGraph();
    for (const auto &Mapping : Graph.value_mappings()) {
        auto Val = Mapping.first;
        Val->print(errs());
        std::cout << std::endl;
        auto &ValueInfo = Mapping.second;
        for (unsigned i = 0; i < ValueInfo.getNumLevels(); ++i) {
            auto &NodeInfo = ValueInfo.getNodeInfoAtLevel(i);
            for (auto &edge : NodeInfo.Edges) {
                edge.Other.Val->print(errs());
                std::cout << ";  deref=" << edge.Other.DerefLevel << std::endl;
            }
        }
        std::cout << "---------" << std::endl;
    }
}

AliasResult CFLDummyAAResult::query(const MemoryLocation &LocA, const MemoryLocation &LocB) {
    std::cout << "Ima here: " << "CFLDummyAAResult::query" << std::endl;
    auto *ValA = const_cast<Value *>(LocA.Ptr);
    auto *ValB = const_cast<Value *>(LocB.Ptr);

    if (!ValA->getType()->isPointerTy() || !ValB->getType()->isPointerTy())
      return NoAlias;

    Function *Fn = nullptr;
    Function *MaybeFnA = const_cast<Function *>(parentFunctionOfValue(ValA));
    Function *MaybeFnB = const_cast<Function *>(parentFunctionOfValue(ValB));
    if (!MaybeFnA && !MaybeFnB) {
      // The only times this is known to happen are when globals + InlineAsm are
      // involved
        std::cout << "CFLSteensAA: could not extract parent function information." << std::endl;
        return MayAlias;
    }

    if (MaybeFnA) {
      Fn = MaybeFnA;
      assert((!MaybeFnB || MaybeFnB == MaybeFnA) &&
             "Interprocedural queries not supported");
    } else {
      Fn = MaybeFnB;
    }

    assert(Fn != nullptr);
    scan(Fn);
    return MayAlias;
}

AnalysisKey CFLDummyAA::Key;

CFLDummyAAResult CFLDummyAA::run(Function &F, FunctionAnalysisManager &AM) {
  return CFLDummyAAResult(AM.getResult<TargetLibraryAnalysis>(F));
}

namespace llvm {


char CFLDummyAAWrapperPass::ID = 0;
INITIALIZE_PASS(CFLDummyAAWrapperPass, "cfl-dummy-aa",
                "Graph-based simple CFL Alias Analysis", false, true)

}

ImmutablePass *llvm::createCFLDummyAAWrapperPass() {
  return new CFLDummyAAWrapperPass();

}

CFLDummyAAWrapperPass::CFLDummyAAWrapperPass()
  : ImmutablePass(ID) {
  initializeCFLDummyAAWrapperPassPass(*PassRegistry::getPassRegistry());
}

void CFLDummyAAWrapperPass::initializePass() {
    std::cout << "Ima here: " << "initializePass" << std::endl;
    auto &TLIWP = getAnalysis<TargetLibraryInfoWrapperPass>();
    Result.reset(new CFLDummyAAResult(TLIWP.getTLI()));
}

void CFLDummyAAWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
    std::cout << "Ima here: " << "getAnalysisUsage" << std::endl;
    AU.setPreservesAll();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
}
