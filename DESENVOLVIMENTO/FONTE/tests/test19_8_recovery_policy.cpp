#include "recovery_policy.hpp"
#include <iostream>
#include <stdexcept>
int main(){
 using alaser::RecoveryPolicy;
 RecoveryPolicy state;
 if(state.dirty()||state.needsSnapshot())throw std::runtime_error("initially clean");
 state.edited();if(!state.dirty()||!state.needsSnapshot())throw std::runtime_error("dirty after edit");
 state.snapshotted();if(!state.dirty()||state.needsSnapshot())throw std::runtime_error("snapshot is not save");
 state.edited();if(!state.needsSnapshot())throw std::runtime_error("re-edit requires snapshot");
 state.saved();if(state.dirty()||state.needsSnapshot())throw std::runtime_error("saved clean");
 state.edited();state.opened();if(state.dirty()||state.needsSnapshot())throw std::runtime_error("opened clean");
 std::cout<<"TESTE 19.8 recovery policy PASS\n";
}
