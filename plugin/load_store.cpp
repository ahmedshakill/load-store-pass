#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"

using namespace llvm;

namespace {

void visitor(Function &F,FunctionAnalysisManager & FAM){


    MemoryDependenceResults *MD = nullptr;
    
    
    for(auto && itBB = F.begin(); itBB!=F.end(); ++itBB ){
        for(auto && itInst = itBB->begin();itInst!=itBB->end();++itInst){
            if(itInst->getOpcode()==itInst->Store){
                errs()<< "found store inst \n";//<<itInst->getNameOrAsOperand()<<"\n";
                errs()<<"operands : ";
                for(int i = 0;i<itInst->getNumOperands();++i){
                    errs()<<*itInst->getOperand(i)<<" ";
                }
                errs()<<"\n";
                
                llvm::MemDepResult memdep = MD->getDependency(&(*itInst));
                auto dependentInst = memdep.getInst();

                // for(auto tmp : itInst->operands()){
                //     errs()<<"operands : "<<*tmp<<"\n";
                // }
            }
            if(itInst->getOpcode()==itInst->Load){
                // errs()<< "found load inst "<<itInst->getName()<<"\n";
            }

        }
    }
}

struct LoadStore : PassInfoMixin<LoadStore>{
    PreservedAnalyses  run(Function &F, FunctionAnalysisManager& FAM){
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