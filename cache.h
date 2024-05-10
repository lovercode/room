#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <atomic>

#define max_size 1000000

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