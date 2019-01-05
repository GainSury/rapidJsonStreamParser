#ifndef CURLREADSTREAM_H
#define CURLREADSTREAM_H

#include "curl_fopen.h"
class UrlReadStream {
public:
    typedef char Ch;    //!< Character type (byte).

    //! Constructor.
    /*!
        \param fp File pointer opened for read.
        \param buffer user-supplied buffer.
        \param bufferSize size of buffer in bytes. Must >=4 bytes.
    */
    UrlReadStream(URL_FILE * handle) : handle_(handle), bufferLast_(0), readCount_(0), count_(0), eof_(false) {
        buffer_ = new char[1024];
        bufferSize_ = 1024;
        current_ = buffer_;
        Read();
    }
    ~UrlReadStream()
    {
        delete buffer_;
    }
    Ch Peek() const { return *current_; }
    Ch Take() { Ch c = *current_; Read(); return c; }
    size_t Tell() const { return count_ + static_cast<size_t>(current_ - buffer_); }

    // Not implemented
    void Put(Ch) { RAPIDJSON_ASSERT(false); }
    void Flush() { RAPIDJSON_ASSERT(false); }
    Ch* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
    size_t PutEnd(Ch*) { RAPIDJSON_ASSERT(false); return 0; }

    // For encoding detection only.
    const Ch* Peek4() const {
        return (current_ + 4 <= bufferLast_) ? current_ : 0;
    }

private:
    void Read() {
        if (current_ < bufferLast_)
            ++current_;
        else if (!eof_) {
            count_ += readCount_;

            //< read data from handle
            readCount_ = url_fread(buffer_, 1, sizeof(bufferSize_), handle_);

            bufferLast_ = buffer_ + readCount_ - 1;
            current_ = buffer_;

            if (url_feof(handle_)) {
                buffer_[readCount_] = '\0';
                ++bufferLast_;
                eof_ = true;
            }
        }
    }

    URL_FILE *handle_;
    Ch *buffer_;
    size_t bufferSize_;
    Ch *bufferLast_;
    Ch *current_;
    size_t readCount_;
    size_t count_;  //!< Number of characters read
    bool eof_;
};


#endif // CURLREADSTREAM_H
