#include "cache.h"
#include <thread>
#include "cost.h"







void addRoomLru(int index, RoomCacheLRU& roomCache) {
    for(int i=0; i<1000000; i++){
        roomCache.AddMember((index << 20) + i);
    }
}



void worker(int index, RoomCache& roomCache) {
    for(int i=0; i<10000; i++){
        roomCache.AddMember((index << 10) + i);
    }
}

void addTest(RoomCacheLRU& r){
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int i=0; i<9; i++){
        std::thread t1(addRoomLru, i, std::ref(r));
        t1.join();
    }
    // 记录结束时间
    auto end_time = std::chrono::high_resolution_clock::now();
    // 计算耗时
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "添加数据耗时: " << duration << " 毫秒" << std::endl;
}

int main(){
    getchar();
 
    RoomCacheLRU* r;

    while (true)
    {
        /* code */
        int menu = 0;
        std::cin >> menu;

        switch (menu)
        {
        case 0:
            addTest(*r);
            break;
        case 1:
            r->ShowAllData();
        case 2:
            r->Debug();
        default:
            r = &RoomCacheLRU("./10001.bin");
            break;
        }
        getchar();
    }
    
    return 0;
}