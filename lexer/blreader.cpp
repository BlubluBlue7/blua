#include "blreader.h"

BLReader::BLReader(size_t bufferSize)
    : buffer_(bufferSize), MAX_BUFFER_SIZE_(bufferSize) { }

bool BLReader::ReadLuaFile(const std::string& fileName) {
    file_.open(fileName, std::ios::binary);
    if (!file_.is_open()) {
        std::cerr << "无法打开文件: " << fileName << std::endl;
        return false;
    }
    // 第一次读取时需要处理BOM
    Read(true);
    return true;
}

void BLReader::Read(bool isFirst) {
    if (!file_.is_open()) {
        return;
    }

    // 读取最多MAX_BUFFER_SIZE_个字符到缓冲区
    file_.read(buffer_.data(), MAX_BUFFER_SIZE_);
    currentBufferSize_ = static_cast<size_t>(file_.gcount());
    leftChar_ = currentBufferSize_;
    readPos_ = 0;

    // 第一次读取时，处理UTF-8 BOM（三个字节：EF BB BF）
    if (isFirst && currentBufferSize_ >= 3) {
        if (static_cast<unsigned char>(buffer_[0]) == 0xEF &&
            static_cast<unsigned char>(buffer_[1]) == 0xBB &&
            static_cast<unsigned char>(buffer_[2]) == 0xBF) {
            readPos_ += 3;
            leftChar_ = (currentBufferSize_ >= 3 ? currentBufferSize_ - 3 : 0);
            }
    }
}

unsigned char BLReader::GetChar() {
    // 当当前缓冲区数据已读完，则尝试重新填充缓冲区
    if (leftChar_ == 0) {
        Read();
        if (leftChar_ == 0) {
            // 如果依然没有数据，则返回结束符
            return '\0';
        }
    }

    unsigned char c = static_cast<unsigned char>(buffer_[readPos_]);
    // std::cout << "current char: " << c << std::endl;
    ++readPos_;
    --leftChar_;
    return c;
}