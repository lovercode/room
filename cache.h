#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <atomic>
#include <mutex>
#include <unordered_map>

int max_size = 10000000;

class RoomCache
{
private:
    int fd;
    size_t file_size;
    /*
    length uid1 uid2
    */
    std::atomic<int64_t> *data;
    std::atomic<int64_t> *member_size;

public:
    RoomCache(const std::string &filename);
    ~RoomCache();

    void AddMember(int64_t uid);
    bool IsInRoom(int64_t uid);
    void Debug();
    void Size();
};


class RoomCacheLRU
{
private:
    int fd;
    size_t file_size;
    /*
    前几个字节存元数据
    头偏移
    尾偏移
    成员数量
    */
    int64_t *head; // 环形队列头偏移量，相对data的
    int64_t *tail; // 环形队列尾，相对data的
    int64_t *member_size; // 成员数量

    int64_t *data; // 数据 uid(64) + ts(32) + other(32) 
    std::mutex lock; // 锁
    std::unordered_map<int, int> data_index; // 存 uid 和 arr offset
public:
    RoomCacheLRU(const std::string &filename);
    ~RoomCacheLRU();

    void AddMember(int64_t uid);
    void DelMember(int64_t uid);
    int64_t GetMember(int64_t uid);
    void Debug();
    void RangeTest();
    void ShowAllData();
};