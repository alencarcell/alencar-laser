#include "../src/dense_render_policy_19_9.hpp"
#include <cassert>
#include <initializer_list>
#include <iostream>
using namespace alaser::renderpolicy199;
static_assert(!useFastDisplay(80000));
static_assert(useFastDisplay(80001));
static_assert(!batchSelectionStyles(31));
static_assert(batchSelectionStyles(32));
int main(){
 for(std::size_t n : {0u,13409u,79999u,80000u}) assert(!useFastDisplay(n));
 for(std::size_t n : {80001u,120081u,122390u}) assert(useFastDisplay(n));
 for(std::size_t n : {0u,1u,31u}) assert(!batchSelectionStyles(n));
 for(std::size_t n : {32u,417u,500u}) assert(batchSelectionStyles(n));
 std::cout<<"PASS: dense render policy 19.9\n";
}
