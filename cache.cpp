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
    std::cout << "head: " << *head << "tail: " << *tail << " member_size: " << *member_size << std::endl;
    for (int i=*head; i<*tail; i++){
        int index = i*sizeof(int64_t)*2;
        if(data[index] > 0){
            data_index[data[index]] = i;
        }
    }
    if (data == MAP_FAILED)
    {
        throw std::runtime_error("Error mapping file");
    }
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
    if(index != data_index.end()){
        // 存在 更新时间
        // data[index] = uid;
        data[index->second*sizeof(int64_t)*2+1] = time(NULL);
        lock.unlock();
        return;
    }
    int i = (*tail)*sizeof(int64_t)*2;
    data_index[uid] = *tail;
    data[i] = uid;
    data[i+1] = time(NULL);
    *tail = (*tail)+1;
    *member_size = (*member_size)+1;
    lock.unlock();
}

int64_t RoomCacheLRU::GetMember(int64_t uid)
{
    lock.lock();
    auto index = data_index.find(uid);
    int64_t ts = 0;
    if(index != data_index.end()){
        ts = data[index->second*sizeof(int64_t)*2+1];
    }
    lock.unlock();
    return ts;
}

void RoomCacheLRU::Debug()
{
    std::cout << std::endl;
    for(int i=0; i<*tail; i++){
        int index = i*sizeof(ino64_t)*2;
        std::cout << "uid: " << data[index] << " ts: " <<  data[index+1] << std::endl;
    }
    std::cout << std::endl;
}