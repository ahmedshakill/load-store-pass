#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AliasAnalysisEvaluator.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"


using namespace llvm;

namespace {

static void PrintResults(AliasResult AR, 
                         std::pair<const Value *, Type *> Loc1,
                         std::pair<const Value *, Type *> Loc2,
                         const Module *M) {
    
    Type *Ty1 = Loc1.second, *Ty2 = Loc2.second;
    unsigned AS1 = Loc1.first->getType()->getPointerAddressSpace();
    unsigned AS2 = Loc2.first->getType()->getPointerAddressSpace();
    std::string o1, o2;
    {
      raw_string_ostream os1(o1), os2(o2);
      Loc1.first->printAsOperand(os1, false, M);
      Loc2.first->printAsOperand(os2, false, M);
    }
 
    if (o2 < o1) {
      std::swap(o1, o2);
      std::swap(Ty1, Ty2);
      std::swap(AS1, AS2);
      // Change offset sign for the local AR, for printing only.
      AR.swap();
    }
    errs() << "  " << AR << ":\t";
    Ty1->print(errs(), false, /* NoDetails */ true);
    if (AS1 != 0)
      errs() << " addrspace(" << AS1 << ")";
    errs() << "* " << o1 << ", ";
    Ty2->print(errs(), false, /* NoDetails */ true);
    if (AS2 != 0)
      errs() << " addrspace(" << AS2 << ")";
    errs() << "* " << o2 << "\n";
  
}
 
static inline void PrintModRefResults(
    const char *Msg,  Instruction *I,
    std::pair<const Value *, Type *> Loc, Module *M) {
  
    errs() << "  " << Msg << ":  Ptr: ";
    Loc.second->print(errs(), false, /* NoDetails */ true);
    errs() << "* ";
    Loc.first->printAsOperand(errs(), false, M);
    errs() << "\t<->" << *I << '\n';
  
}
 
static inline void PrintModRefResults(const char *Msg, CallBase *CallA,
                                      CallBase *CallB, Module *M) {
  
    errs() << "  " << Msg << ": " << *CallA << " <-> " << *CallB << '\n';
  
}
 
static inline void PrintLoadStoreResults(AliasResult AR,
                                         const Value *V1, const Value *V2,
                                         const Module *M) {
  
    errs() << "  " << AR << ": " << *V1 << " <-> " << *V2 << '\n';
  
}

void getAliasAnalysis(Function &F, AAResults &AA){

    static   int64_t FunctionCount = 0;
    static   int64_t NoAliasCount = 0, MayAliasCount = 0, PartialAliasCount = 0;
    static   int64_t MustAliasCount = 0;
    static   int64_t NoModRefCount = 0, ModCount = 0, RefCount = 0, ModRefCount = 0;

  const DataLayout &DL = F.getParent()->getDataLayout(); 
  SetVector<std::pair<const Value *, Type *>> Pointers;
  SmallSetVector<CallBase *, 16> Calls;
  SetVector<Value *> Loads;
  SetVector<Value *> Stores;
 
  for (Instruction &Inst : instructions(F)) {
    if (auto *LI = dyn_cast<LoadInst>(&Inst)) {
      Pointers.insert({LI->getPointerOperand(), LI->getType()});
      Loads.insert(LI);
    } else if (auto *SI = dyn_cast<StoreInst>(&Inst)) {
      Pointers.insert({SI->getPointerOperand(),
                       SI->getValueOperand()->getType()});
      Stores.insert(SI);
    } else if (auto *CB = dyn_cast<CallBase>(&Inst))
      Calls.insert(CB);
  }
 
    errs() << "Function: " << F.getName() << ": " << Pointers.size()
           << " pointers, " << Calls.size() << " call sites\n";
 
    errs()<<"\n\n Pointer Alias Analysis\n\n";

  // iterate over the worklist, and run the full (n^2)/2 disambiguations
  for (auto I1 = Pointers.begin(), E = Pointers.end(); I1 != E; ++I1) {
    LocationSize Size1 = LocationSize::precise(DL.getTypeStoreSize(I1->second));
    for (auto I2 = Pointers.begin(); I2 != I1; ++I2) {
      LocationSize Size2 =
          LocationSize::precise(DL.getTypeStoreSize(I2->second));
      AliasResult AR = AA.alias(I1->first, Size1, I2->first, Size2);
      switch (AR) {
      case AliasResult::NoAlias:
        //PrintResults(AR,  *I1, *I2, F.getParent());
        ++NoAliasCount;
        break;
      case AliasResult::MayAlias:
        PrintResults(AR,  *I1, *I2, F.getParent());
        ++MayAliasCount;
        break;
      case AliasResult::PartialAlias:
        PrintResults(AR,  *I1, *I2, F.getParent());
        ++PartialAliasCount;
        break;
      case AliasResult::MustAlias:
        PrintResults(AR,  *I1, *I2, F.getParent());
        ++MustAliasCount;
        break;
      }
    }
  }

 errs()<<"\n Load Store Alias Analysis\n\n";

    // if (EvalAAMD) {
    // iterate over all pairs of load, store
    for (Value *Load : Loads) {
      for (Value *Store : Stores) {
        AliasResult AR = AA.alias(MemoryLocation::get(cast<LoadInst>(Load)),
                                  MemoryLocation::get(cast<StoreInst>(Store)));
        switch (AR) {
        case AliasResult::NoAlias:
        //   PrintLoadStoreResults(AR,  Load, Store, F.getParent());
          ++NoAliasCount;
          break;
        case AliasResult::MayAlias:
          PrintLoadStoreResults(AR,  Load, Store, F.getParent());
          ++MayAliasCount;
          break;
        case AliasResult::PartialAlias:
          PrintLoadStoreResults(AR,  Load, Store, F.getParent());
          ++PartialAliasCount;
          break;
        case AliasResult::MustAlias:
          PrintLoadStoreResults(AR,  Load, Store, F.getParent());
          ++MustAliasCount;
          break;
        }
      }
    }
 
    // iterate over all pairs of store, store
    for (SetVector<Value *>::iterator I1 = Stores.begin(), E = Stores.end();
         I1 != E; ++I1) {
      for (SetVector<Value *>::iterator I2 = Stores.begin(); I2 != I1; ++I2) {
        AliasResult AR = AA.alias(MemoryLocation::get(cast<StoreInst>(*I1)),
                                  MemoryLocation::get(cast<StoreInst>(*I2)));
        switch (AR) {
        case AliasResult::NoAlias:
        //   PrintLoadStoreResults(AR,  *I1, *I2, F.getParent());
          ++NoAliasCount;
          break;
        case AliasResult::MayAlias:
          PrintLoadStoreResults(AR,  *I1, *I2, F.getParent());
          ++MayAliasCount;
          break;
        case AliasResult::PartialAlias:
          PrintLoadStoreResults(AR,  *I1, *I2, F.getParent());
          ++PartialAliasCount;
          break;
        case AliasResult::MustAlias:
          PrintLoadStoreResults(AR,  *I1, *I2, F.getParent());
          ++MustAliasCount;
          break;
        }
      }
    }
//   }
 
 errs()<<"\n\n ModRef Analysis\n\n";

  // Mod/ref alias analysis: compare all pairs of calls and values
  for (CallBase *Call : Calls) {
    for (const auto &Pointer : Pointers) {
      LocationSize Size =
          LocationSize::precise(DL.getTypeStoreSize(Pointer.second));
      switch (AA.getModRefInfo(Call, Pointer.first, Size)) {
      case ModRefInfo::NoModRef:
        // PrintModRefResults("NoModRef",  Call, Pointer,
                        //    F.getParent());
        ++NoModRefCount;
        break;
      case ModRefInfo::Mod:
        PrintModRefResults("Just Mod", Call, Pointer, F.getParent());
        ++ModCount;
        break;
      case ModRefInfo::Ref:
        PrintModRefResults("Just Ref", Call, Pointer, F.getParent());
        ++RefCount;
        break;
      case ModRefInfo::ModRef:
        PrintModRefResults("Both ModRef",  Call, Pointer,
                           F.getParent());
        ++ModRefCount;
        break;
      }
    }
  }
 
  // Mod/ref alias analysis: compare all pairs of calls
  for (CallBase *CallA : Calls) {
    for (CallBase *CallB : Calls) {
      if (CallA == CallB)
        continue;
      switch (AA.getModRefInfo(CallA, CallB)) {
      case ModRefInfo::NoModRef:
        // PrintModRefResults("NoModRef",  CallA, CallB,
                        //    F.getParent());
        ++NoModRefCount;
        break;
      case ModRefInfo::Mod:
        PrintModRefResults("Just Mod", CallA, CallB, F.getParent());
        ++ModCount;
        break;
      case ModRefInfo::Ref:
        PrintModRefResults("Just Ref",  CallA, CallB, F.getParent());
        ++RefCount;
        break;
      case ModRefInfo::ModRef:
        PrintModRefResults("Both ModRef", CallA, CallB,
                           F.getParent());
        ++ModRefCount;
        break;
      }
    }
  }
}

void visitor(Function &F,FunctionAnalysisManager & FAM){
    getAliasAnalysis(F, FAM.getResult<AAManager>(F));
    errs()<<"\n Dependency Analysis\n\n";
    llvm::MemoryDependenceAnalysis::Result *MDA = &FAM.getResult<MemoryDependenceAnalysis>(F);  
    llvm::DependenceAnalysis::Result *DA  = &FAM.getResult<DependenceAnalysis>(F);
    llvm::SetVector<Instruction*> instVect;
    for(BasicBlock &BB : F){
        for(Instruction &I : BB){
            if(I.getOpcode() ==I.Store ){
                instVect.insert(&I);
                using LoadDepVect = SmallVector<NonLocalDepResult, 64>;
                LoadDepVect Deps;
                MDA->getNonLocalPointerDependency(&I, Deps);
                unsigned NumDeps = Deps.size();
                errs()<<I<<" "<<";";
                if(NumDeps > 1){
                    errs()<<"\t\tNonLocal dep \n";
                    for(auto NLDR : Deps){
                        Instruction *DepInst =  NLDR.getResult().getInst();
                        auto depInfo = DA->depends(DepInst,&I,true);
                        if((DepInst!=nullptr  && depInfo!=nullptr ) /*&& !strcmp(DepInst->getOpcodeName(),"store")*/ ){
                            errs()<<"\t\t"<<*DepInst<<" ";
                            depInfo->dump(errs()<<"\t ; deptype ");
                            errs().flush();
                        }
                    }
                errs()<<"\n";
                }

                
                auto localDepResult = MDA->getDependency(&I);
                auto dependentInst = localDepResult.getInst();
                if(dependentInst!=nullptr ){
                    auto depInfo = DA->depends(dependentInst,&I,true);
                    errs()<<"\t\t\tLocal dep \n\t\t"<<*dependentInst<<" ";
                    if( depInfo!=nullptr && !dyn_cast<CallInst>(dependentInst))
                        depInfo->dump(errs()<<"\t ; deptype ");
                    errs().flush();
                    errs()<<"\n";
                }else{
                    errs()<<"\n";
                }
            }
            
        }
    }


}

struct LoadStore : PassInfoMixin<LoadStore>{
   
    PreservedAnalyses  run(Function &F, FunctionAnalysisManager& FAM){

        if(F.getName().str()=="kernel_cholesky"){
            AAEvaluator aaeval;
            aaeval.run(F,FAM);
            
            visitor(F,FAM);
        }

        return PreservedAnalyses::all();
    }
    static bool isRequired(){return true;}
};
}

llvm::PassPluginLibraryInfo getLoadStorePluginInfo(){

    return {
        LLVM_PLUGIN_API_VERSION,"LoadStore", LLVM_VERSION_STRING,
        [](llvm::PassBuilder& PB){
            PB.registerPipelineParsingCallback(
                [](StringRef Name,
                FunctionPassManager& FPM,
                ArrayRef<PassBuilder::PipelineElement>){
                    if(Name == "load-store"){
                        FPM.addPass(LoadStore());
                        return true;
                    }
                    return false;
                }
            );
        }
    };

}


extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo 
llvmGetPassPluginInfo(){
    return getLoadStorePluginInfo();
}

/*

  opt -load-pass-plugin ./build/plugin/libLoadStore.so -passes=load-store -disable-output cholesky.ll

*/