#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class BLReader
{
private:
    // 从文件中读取数据填充缓冲区
    void Read(bool isFirst = false);

    std::ifstream file_;
    std::vector<char> buffer_;
    size_t currentBufferSize_ = 0; // 当前缓冲区中有效数据的字节数
    size_t leftChar_ = 0;          // 缓冲区中未读取的字节数
    size_t readPos_ = 0;           // 当前读取位置

    const size_t MAX_BUFFER_SIZE_; // 缓冲区大小
public:
    explicit BLReader(size_t bufferSize = 1024);  // 可设置缓冲区大小，默认1024字节
    ~BLReader() = default;

    // 打开Lua文件，并准备读取
    bool ReadLuaFile(const std::string& fileName);

    // 获取下一个字符；文件结束时返回 '\0'
    unsigned char GetChar();
};