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
 
 
    RoomCacheLRU r("./10001.bin");

    // 记录开始时间
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 100000; i++) {
        r.AddMember((1 << 5) + i);
    }

    // 记录结束时间
    auto end_time = std::chrono::high_resolution_clock::now();

    std::cout << r.GetMember(999) << std::endl;

    // 计算耗时
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "添加数据耗时: " << duration << " 毫秒" << std::endl;

    getchar();
    return 0;
}