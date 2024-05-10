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
