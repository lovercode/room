#include "cache.h"

RoomCache::RoomCache(const std::string &filename)
{
    fd = open(filename.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        throw std::runtime_error("Error opening file");
    }

    // 设置文件大小
    file_size = sizeof(std::atomic<int64_t>) * max_size;
    if (ftruncate(fd, file_size) == -1)
    {
        throw std::runtime_error("Error setting file size");
    }

    // 内存映射
    auto arr = (std::atomic<int64_t> *)mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    member_size = arr;
    data = arr + 1;
    if (data == MAP_FAILED)
    {
        throw std::runtime_error("Error mapping file");
    }
}

RoomCache::~RoomCache()
{
}

void RoomCache::AddMember(int64_t uid)
{
    int index = 0;
    int64_t expected = 0;
    while (!data[index].compare_exchange_weak(expected, uid))
    {
        index++;
        expected = 0;
    }

    int64_t current_member_size = member_size->load();
    while (index > current_member_size)
    {
        if (member_size->compare_exchange_weak(current_member_size, index))
        {
            break;
        }
    }
}

bool RoomCache::IsInRoom(int64_t uid)
{
    for (int i=0; i<member_size->load(); i++){
        if(data[i].load() == uid){
            return true;
        }
    }
    return false;
}

void RoomCache::Debug()
{
    std::cout << "size: " << member_size->load() << " data: ";
    for (int i = 0; i <= member_size->load(); i++)
    {
        std::cout << data[i].load() << " ";
    }
}

void RoomCache::Size()
{
    std::cout << "size: " << member_size->load() << " data: ";
}

RoomCacheLRU::RoomCacheLRU(const std::string &filename)
{
    auto start_time = std::chrono::high_resolution_clock::now();
    fd = open(filename.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        throw std::runtime_error("Error opening file");
    }
    // 设置文件大小 head + data = sizeof(int64) * 3 + sizeof(int64) * 2 * 100w
    file_size = sizeof(int64_t)*3 + sizeof(int64_t)*2*max_size;
    if (ftruncate(fd, file_size) == -1)
    {
        throw std::runtime_error("Error setting file size");
    }

    // 内存映射
    int64_t* arr = (int64_t*)mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    head = arr;
    tail = arr + 1;
    member_size = arr + 2;
    data = arr + 3;
    for (int i=*head; i<*tail; i++){
        int index = i*2;
        if(data[index] > 0){
            data_index[data[index]] = i;
        }
    }
    if (data == MAP_FAILED)
    {
        throw std::runtime_error("Error mapping file");
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "加载" << *member_size << "条数据耗时: " << duration << " 毫秒" << std::endl;
}

RoomCacheLRU::~RoomCacheLRU()
{
}

void RoomCacheLRU::AddMember(int64_t uid)
{
    lock.lock();
    /*
        存在，更新时间
        不存在，添加
    */
    auto index = data_index.find(uid);
    int64_t dataIndex = 0;
    if(index != data_index.end()){
        dataIndex = index->second*2;
        // 存在 更新时间
        data[dataIndex+1] = time(NULL);
    }else{
        /*
        1. 找空闲的
            (tail+1) != head 说明还有空闲的，直接放在tail
            (tail+1) == head 说明用完了
                检查head是否是空的，是的话尽量往后走一走
                不为空，说明head和tail都不为空，但是中间有空闲的，需要碎片整理（整理是渐进式的，每次整理一批出来）
        0 0 1 1 1 1 1 0 1
              t h
        开始整理
        0 1 1 1 0 1 1 0 1
              t h
        1 1 1 1 0 0 1 0 1
              t h
        1 1 1 1 0 0 0 1 1
              t h
        1 1 1 1 0 0 0 1 1
              t       h
        */
        // 先找到一个可用的
        dataIndex = (*tail)*2;
        data_index[uid] = (*tail);
        data[dataIndex] = uid;
        data[dataIndex+1] = time(NULL);
        *tail = (*tail)+1;
        *member_size = (*member_size)+1;
    }

    msync((void*)&data[dataIndex], sizeof(int64_t)*2, MS_SYNC);
    lock.unlock();
}

void RoomCacheLRU::DelMember(int64_t uid)
{
    lock.lock();
    auto index = data_index.find(uid);
    if(index != data_index.end()){
        int64_t dataIndex = index->second*2;
        data[dataIndex] = 0;
        data[dataIndex+1] = 0;
        data_index.erase(uid);
        *member_size = (*member_size)-1;
        msync((void*)&data[dataIndex], sizeof(int64_t)*2, MS_SYNC);
    }
    lock.unlock();
}

int64_t RoomCacheLRU::GetMember(int64_t uid)
{
    lock.lock();
    auto index = data_index.find(uid);
    int64_t ts = 0;
    if(index != data_index.end()){
        ts = data[index->second*2+1];
    }
    lock.unlock();
    return ts;
}

void RoomCacheLRU::Debug()
{
    std::cout << "head: " << *head << " tail: " << *tail << " member_size: " << *member_size << " index_size:" << data_index.size() << std::endl;
}

void RoomCacheLRU::RangeTest()
{
    int64_t sum = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int i=0; i<*tail; i++){
        int64_t uid = data[i*2];
        if((i&1) == 0){
            sum += uid;
        }else{
            sum -= uid;
        }
    }
    // for(auto it=data_index.begin(); it!=data_index.end(); it++){
    //     int64_t uid = it->first;
    //     if((it->second&1) == 0){
    //         sum += uid;
    //     }else{
    //         sum -= uid;
    //     }
    // }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "遍历计算" << *member_size << "条数据 "<< sum <<"耗时: " << duration << " 毫秒" << std::endl;
}

void RoomCacheLRU::ShowAllData()
{
    std::cout << "index\t" << "dindex\t" << "uid\t" << "ts\t" << std::endl;
    for(int i=0; i<*tail; i++){
        int64_t uid = data[i*2];
        int64_t ts = data[i*2+1];
        std::cout << i << "\t" << data_index[uid] <<"\t" << uid << "\t" << ts << "\t" << std::endl;
    }
    std::cout << std::endl;
}
