#include "cache.h"
#include <thread>
#include "cost.h"





void worker(int index, RoomCache& roomCache) {
    for(int i=0; i<10000; i++){
        roomCache.AddMember((index << 10) + i);
    }
}



int main(){
    // RoomCache r = RoomCache("./10001.bin");
    // std::thread t1(worker, 1, std::ref(r));
    // std::thread t2(worker, 2, std::ref(r));
    // t1.join();
    // t2.join();
    // for(int i=0; i<900000; i++){
    //     r.AddMember(i);
    // }
    // r.Size();
    // std::cout << r.IsInRoom(65950);
    // r.Debug();

    RoomCacheLRU r = RoomCacheLRU("./10001.bin");
    // getchar();
}