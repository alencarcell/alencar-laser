#pragma once
// Small Qt-free state machine. Never saves directly over the user project.
#include <cstdint>
namespace alaser {
class RecoveryPolicy {
 std::uint64_t edit_=0, saved_=0, snapshot_=0;
public:
 void edited(){++edit_;}
 void opened(){saved_=edit_;snapshot_=edit_;}
 void saved(){saved_=edit_;snapshot_=edit_;}
 bool dirty() const{return edit_!=saved_;}
 bool needsSnapshot() const{return dirty()&&edit_!=snapshot_;}
 void snapshotted(){snapshot_=edit_;}
 std::uint64_t editVersion()const{return edit_;}
};
}
