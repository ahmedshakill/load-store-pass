#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"


#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/SourceMgr.h"

using namespace llvm;

namespace {

void visitor(Function &F,FunctionAnalysisManager & FAM){
    auto &MD = FAM.getResult<MemoryDependenceAnalysis>(F);  //: nullptr;
    // FAM.getResult<Memory
    // &FAM.getResult<AA>
    

    // llvm::PassManager<MemoryDependenceAnalysis> *pm;
    // pm->addPass(MemoryDependenceAnalysis());
    // pm->run(F,FAM);
    // getA
    // MemoryDependenceResults *MD = nullptr;
    // FAM.registerPass(PassBuilder && PB);
    
    for(auto && itBB = F.begin(); itBB!=F.end(); ++itBB ){
        for(auto && itInst = itBB->begin();itInst!=itBB->end();++itInst){
            if(itInst->getOpcode()==itInst->Store){
                // errs()<< "store ";
                // for(auto  &&it = itInst->op_begin(); it!=itInst->op_end();++it){
                //     errs()<<*it->get()<<" ";
                // }
                // errs()<<"\n";
                // llvm::MemDepResult memdep = MD.getDependency(&(*itInst));
                // if(memdep.isLocal()){
                //     auto dependentInst = memdep.getInst();
                //     // if(dyn_cast<CallInst>(dependentInst)){
                //         errs()<<"dependent instruction \n"<<dependentInst->getOpcodeName()<<" ";
                //         // <<" operands : ";
                //         // for(int i = 0;i<dependentInst->getNumOperands();++i){
                //         //     errs()<<*dependentInst->getOperand(i)<<" ";
                //         // }

                //         for(auto  &&it = dependentInst->op_begin(); it!=dependentInst->op_end();++it){
                //                 errs()<<*it->get()<<" ";
                //         }
                //         errs()<<"\n";
                //     // }
                // }
            }
            if(itInst->getOpcode()==itInst->Load){
                errs()<< "load ";

                // for(auto  &&it = itInst->op_begin(); it!=itInst->op_end();++it){
                //     errs()<<*it->get()<<" ";
                // }
                // errs()<<"\n";
                
                // auto dependentInst = MemDep.getDependency(&(*itInst)).getInst() ;
                // llvm::MemDepResult memdep = MD.getInvariantGroupPointerDependency(itInst,itBB);

                // if(MemDep.getDependency(&(*itInst)).isNonLocal()){
                //     llvm::SmallVector< llvm::NonLocalDepResult > memdepResult;
                //     MemDep.getNonLocalPointerDependency(&(*itInst),memdepResult); 

                //     for(auto &tmp : memdepResult){
                //         for(auto i=tmp.getBB()->begin();i!=tmp.getBB()->end();++i){
                //                 errs()<<*i->getOpcodeName()<<" ";
                //         }
                //         errs()<<"\n";
                //     }

                // }
                

            }

        }
    }
}

struct LoadStore : PassInfoMixin<LoadStore>{
   
    PreservedAnalyses  run(Function &F, FunctionAnalysisManager& FAM){
        // FPM.addPass(MemoryDependenceAnalysis());
        // FPM.run(F,FAM);

        if(F.getName().str()=="kernel_cholesky")
            visitor(F,FAM);

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