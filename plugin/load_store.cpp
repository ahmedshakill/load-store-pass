#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AliasAnalysisEvaluator.h"

#include "llvm/Analysis/MemorySSA.h"

using namespace llvm;

namespace {

void visitor(Function &F,FunctionAnalysisManager & FAM){
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