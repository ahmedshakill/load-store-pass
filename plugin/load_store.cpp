#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/DependenceAnalysis.h"

// Use MemorySSA
// It creates an internal data structure of Use, Def and Phi for a provided function
// returns for an instuction its user list.
#include "llvm/Analysis/MemorySSA.h"

using namespace llvm;

namespace {

void visitor(Function &F,FunctionAnalysisManager & FAM){
    llvm::MemoryDependenceAnalysis::Result *MDA = &FAM.getResult<MemoryDependenceAnalysis>(F);  
    llvm::DependenceAnalysis::Result *DA  = &FAM.getResult<DependenceAnalysis>(F);
    auto memssa=  &FAM.getResult<MemorySSAAnalysis>(F).getMSSA();

    errs()<<"\n\n\t\t\t\tMemorySSA representation\n\n\n";
    memssa->print(errs());
    errs()<<" \n\n";
    llvm::SetVector<Instruction*> instVect;
    
    for(BasicBlock &BB : F){
            // for(auto &it=blockdeflist->begin();it!=blockdeflist.end();++it){

            // }

        for(Instruction &I : BB){

            if(I.getOpcode() ==I.Store ){
                
                
            
            }
            
            if( I.getOpcode()==I.Load){
            
                errs()<<I<<"\n defs";
                auto result = memssa->getSkipSelfWalker()->getClobberingMemoryAccess(&I); 
                errs()<<"\t\t\t\t\t\t\t"<<*result<<" \n\n";

            }
        }
    }

    

/*
    for(auto && itBB = F.begin(); itBB!=F.end(); ++itBB ){
        for(auto && itInst = itBB->begin();itInst!=itBB->end();++itInst){
            if(itInst->getOpcode()==itInst->Store or itInst->getOpcode()==itInst->Load){
                // errs()<< "store ";
                // for(auto  &&it = itInst->op_begin(); it!=itInst->op_end();++it){
                //     errs()<<*it->get()<<" ";
                // }
                // errs()<<"\n";
                llvm::MemDepResult memdep = MD.getDependency(&(*itInst));
                errs()<<itInst->getOpcodeName()<<"\n";
                if(memdep.isLocal()){
                    auto dependentInst = memdep.getInst();
                    // if(dyn_cast<CallInst>(dependentInst)){
                        errs()<<"dependent instruction \n"<<dependentInst->getOpcodeName()<<" ";
                        // <<" operands : ";
                        // for(int i = 0;i<dependentInst->getNumOperands();++i){
                        //     errs()<<*dependentInst->getOperand(i)<<" ";
                        // }

                        for(auto  &&it = dependentInst->op_begin(); it!=dependentInst->op_end();++it){
                                errs()<<*it->get()<<" ";
                        }
                        errs()<<"\n";
                    // }
                }
            }
            if(itInst->getOpcode()==itInst->Load){
                // errs()<< "load ";

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
    */
}

struct LoadStore : PassInfoMixin<LoadStore>{
   
    PreservedAnalyses  run(Function &F, FunctionAnalysisManager& FAM){

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