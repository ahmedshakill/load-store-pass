#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/DependenceAnalysis.h"

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
            /*
                errs()<<I.getOpcodeName()<<" ";
                for(auto  &&it = I.op_begin(); it!=I.op_end();++it){
                    errs()<<*it->get()<<" ";
                }
                errs()<<"\n";

                llvm::MemDepResult memdep = MDA->getDependency(&I);
                auto dependentInst = memdep.getInst();
                if(dependentInst!=nullptr){
                    // if(dyn_cast<CallInst>(dependentInst)){
                        errs()<<"dependent instruction \n"<<dependentInst->getOpcodeName()<<" ";
                        for(auto  &&it = dependentInst->op_begin(); it!=dependentInst->op_end();++it){
                                errs()<<*it->get()<<" ";
                        }
                        errs()<<"\n";
                    // }
                }

            */    
                using LoadDepVect = SmallVector<NonLocalDepResult, 64>;
                LoadDepVect Deps;
                MDA->getNonLocalPointerDependency(&I, Deps);
                unsigned NumDeps = Deps.size();
                if(NumDeps > 1){
                    errs()<<I<<" "<<"\t\t has "<<NumDeps<<" deps \n";;
                    for(auto NLDR : Deps){
                        Instruction *DepInst =  NLDR.getResult().getInst();
                        auto depInfo = DA->depends(DepInst,&I,true);
                        if((DepInst!=nullptr  && depInfo!=nullptr ) /*&& !strcmp(DepInst->getOpcodeName(),"store")*/ ){
                            errs()<<"\t"<<*DepInst<<" \n";
                            depInfo->dump(errs()<<"\t ");
                            errs().flush();
                        }
                    }
                errs()<<"\n";
                }
            }
            
            if( I.getOpcode()==I.Load){
                instVect.insert(&I);
               /* 
                errs()<<I.getOpcodeName()<<" ";
                for(auto  &&it = I.op_begin(); it!=I.op_end();++it){
                    errs()<<*it->get()<<" ";
                }
                errs()<<"\n";

                llvm::MemDepResult memdep = MDA->getDependency(&I);
                if(memdep.isNonLocal()){
                    auto dependentInst = memdep.getInst();
                    if(dependentInst!=nullptr){
                        // if(dyn_cast<CallInst>(dependentInst)){
                            errs()<<"dependent instruction \n"<<dependentInst->getOpcodeName()<<" ";
                            for(auto  &&it = dependentInst->op_begin(); it!=dependentInst->op_end();++it){
                                    errs()<<*it->get()<<" ";
                            }
                            errs()<<"\n";
                        // }
                    }
                    
                }*/
                
                using LoadDepVect = SmallVector<NonLocalDepResult, 64>;
                LoadDepVect Deps;
                MDA->getNonLocalPointerDependency(&I, Deps);
                unsigned NumDeps = Deps.size();
                if(NumDeps > 1){
                    errs()<<I<<" "<<"\t\t has "<<NumDeps<<" deps \n";;
                    for(auto NLDR : Deps){
                        Instruction *DepInst =  NLDR.getResult().getInst();
                        auto depInfo = DA->depends(DepInst,&I,true);
                        if((DepInst!=nullptr && depInfo!=nullptr) /*&& !strcmp(DepInst->getOpcodeName(),"store")*/ ){
                            errs()<<"\t"<<*DepInst<<" ";
                            depInfo->dump(errs()<<"\t ");
                            errs().flush();
                        }
                    }
                errs()<<"\n";
                }
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