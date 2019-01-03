// Example of parsing JSON to document by parts.

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/writer.h"
#include "rapidjson/ostreamwrapper.h"
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"

using namespace rapidjson;
using namespace std;

struct MyHandler : public BaseReaderHandler<UTF8<>, MyHandler> {
    bool Null() { cout << "Null()" << endl; return true; }
    bool Bool(bool b) { cout << "Bool(" << boolalpha << b << ")" << endl; return true; }
    bool Int(int i) { cout << "Int(" << i << ")" << endl; return true; }
    bool Uint(unsigned u) { cout << "Uint(" << u << ")" << endl; return true; }
    bool Int64(int64_t i) { cout << "Int64(" << i << ")" << endl; return true; }
    bool Uint64(uint64_t u) { cout << "Uint64(" << u << ")" << endl; return true; }
    bool Double(double d) { cout << "Double(" << d << ")" << endl; return true; }
    bool String(const char* str, SizeType length, bool copy) {
        cout << "String(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;
        return true;
    }
    bool StartObject() { cout << "StartObject()" << endl; return true; }
    bool Key(const char* str, SizeType length, bool copy) {
        cout << "Key(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;
        return true;
    }
    bool EndObject(SizeType memberCount) { cout << "EndObject(" << memberCount << ")" << endl; return true; }
    bool StartArray() { cout << "StartArray()" << endl; return true; }
    bool EndArray(SizeType elementCount) { cout << "EndArray(" << elementCount << ")" << endl; return true; }
};


template<unsigned parseFlags = kParseDefaultFlags>
class AsyncSAXParser {
public:
    AsyncSAXParser(Reader& reader,MyHandler& handler)
        : stream_(*this)
        , reader_(reader)
        , handler_(handler)
        , parseThread_()
        , mutex_()
        , notEmpty_()
        , finish_()
        , completed_()
    {
        // Create and execute thread after all member variables are initialized.
        parseThread_ = std::thread(&AsyncSAXParser::Parse, this);
    }

    ~AsyncSAXParser() {
        if (!parseThread_.joinable())
            return;

        {
            std::unique_lock<std::mutex> lock(mutex_);

            // Wait until the buffer is read up (or parsing is completed)
            while (!stream_.Empty() && !completed_)
                finish_.wait(lock);

            // Automatically append '\0' as the terminator in the stream.
            static const char terminator[] = "";
            stream_.src_ = terminator;
            stream_.end_ = terminator + 1;
            notEmpty_.notify_one(); // unblock the AsyncStringStream
        }

        parseThread_.join();
    }

    void ParsePart(const char* buffer, size_t length) {
        std::unique_lock<std::mutex> lock(mutex_);

        // Wait until the buffer is read up (or parsing is completed)
        while (!stream_.Empty() && !completed_)
            finish_.wait(lock);

        // Stop further parsing if the parsing process is completed.
        if (completed_)
            return;

        // Set the buffer to stream and unblock the AsyncStringStream
        stream_.src_ = buffer;
        stream_.end_ = buffer + length;
        notEmpty_.notify_one();
    }

private:
    void Parse() {
        //d_.ParseStream<parseFlags>(stream_);
        reader_.Parse<parseFlags>(stream_,handler_);

        // The stream may not be fully read, notify finish anyway to unblock ParsePart()
        std::unique_lock<std::mutex> lock(mutex_);
        completed_ = true;      // Parsing process is completed
        finish_.notify_one();   // Unblock ParsePart() or destructor if they are waiting.
    }

    struct AsyncStringStream {
        typedef char Ch;

        AsyncStringStream(AsyncSAXParser& parser) : parser_(parser), src_(), end_(), count_() {}

        char Peek() const {
            std::unique_lock<std::mutex> lock(parser_.mutex_);

            // If nothing in stream, block to wait.
            while (Empty())
                parser_.notEmpty_.wait(lock);

            return *src_;
        }

        char Take() {
            std::unique_lock<std::mutex> lock(parser_.mutex_);

            // If nothing in stream, block to wait.
            while (Empty())
                parser_.notEmpty_.wait(lock);

            count_++;
            char c = *src_++;

            // If all stream is read up, notify that the stream is finish.
            if (Empty())
                parser_.finish_.notify_one();

            return c;
        }

        size_t Tell() const { return count_; }

        // Not implemented
        char* PutBegin() { return 0; }
        void Put(char) {}
        void Flush() {}
        size_t PutEnd(char*) { return 0; }

        bool Empty() const { return src_ == end_; }

        AsyncSAXParser& parser_;
        const char* src_;     //!< Current read position.
        const char* end_;     //!< End of buffer
        size_t count_;        //!< Number of characters taken so far.
    };

    AsyncStringStream stream_;
    Reader& reader_;
    MyHandler& handler_;
    std::thread parseThread_;
    std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable finish_;
    bool completed_;
};


template<unsigned parseFlags = kParseDefaultFlags>
class AsyncDocumentParser {
public:
    AsyncDocumentParser(Document& d)
        : stream_(*this)
        , d_(d)
        , parseThread_()
        , mutex_()
        , notEmpty_()
        , finish_()
        , completed_()
    {
        // Create and execute thread after all member variables are initialized.
        parseThread_ = std::thread(&AsyncDocumentParser::Parse, this);
    }

    ~AsyncDocumentParser() {
        if (!parseThread_.joinable())
            return;

        {
            std::unique_lock<std::mutex> lock(mutex_);

            // Wait until the buffer is read up (or parsing is completed)
            while (!stream_.Empty() && !completed_)
                finish_.wait(lock);

            // Automatically append '\0' as the terminator in the stream.
            static const char terminator[] = "";
            stream_.src_ = terminator;
            stream_.end_ = terminator + 1;
            notEmpty_.notify_one(); // unblock the AsyncStringStream
        }

        parseThread_.join();
    }

    void ParsePart(const char* buffer, size_t length) {
        std::unique_lock<std::mutex> lock(mutex_);

        // Wait until the buffer is read up (or parsing is completed)
        while (!stream_.Empty() && !completed_)
            finish_.wait(lock);

        // Stop further parsing if the parsing process is completed.
        if (completed_)
            return;

        // Set the buffer to stream and unblock the AsyncStringStream
        stream_.src_ = buffer;
        stream_.end_ = buffer + length;
        notEmpty_.notify_one();
    }

private:
    void Parse() {
        d_.ParseStream<parseFlags>(stream_);

        // The stream may not be fully read, notify finish anyway to unblock ParsePart()
        std::unique_lock<std::mutex> lock(mutex_);
        completed_ = true;      // Parsing process is completed
        finish_.notify_one();   // Unblock ParsePart() or destructor if they are waiting.
    }

    struct AsyncStringStream {
        typedef char Ch;

        AsyncStringStream(AsyncDocumentParser& parser) : parser_(parser), src_(), end_(), count_() {}

        char Peek() const {
            std::unique_lock<std::mutex> lock(parser_.mutex_);

            // If nothing in stream, block to wait.
            while (Empty())
                parser_.notEmpty_.wait(lock);

            return *src_;
        }

        char Take() {
            std::unique_lock<std::mutex> lock(parser_.mutex_);

            // If nothing in stream, block to wait.
            while (Empty())
                parser_.notEmpty_.wait(lock);

            count_++;
            char c = *src_++;

            // If all stream is read up, notify that the stream is finish.
            if (Empty())
                parser_.finish_.notify_one();

            return c;
        }

        size_t Tell() const { return count_; }

        // Not implemented
        char* PutBegin() { return 0; }
        void Put(char) {}
        void Flush() {}
        size_t PutEnd(char*) { return 0; }

        bool Empty() const { return src_ == end_; }

        AsyncDocumentParser& parser_;
        const char* src_;     //!< Current read position.
        const char* end_;     //!< End of buffer
        size_t count_;        //!< Number of characters taken so far.
    };

    AsyncStringStream stream_;
    Document& d_;
    std::thread parseThread_;
    std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable finish_;
    bool completed_;
};


int main() {

    MyHandler handler;
    Reader reader;

    {
        const char json1[] = " { \"hello\" : \"world\", \"t\" : tr";
        //const char json1[] = " { \"hello\" : \"world\", \"t\" : trX"; // For test parsing error
        const char json2[] = "ue, \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.14";
        const char json3[] = "16, \"a\":[1, 2, 3, 4] } ";
        AsyncSAXParser<kParseStopWhenDoneFlag> parser(reader,handler);
        parser.ParsePart(json1,sizeof(json1) - 1);
        parser.ParsePart(json2,sizeof(json2) - 1);
        parser.ParsePart(json3,sizeof(json3) - 1);
        parser.ParsePart(json1,sizeof(json1) - 1);
        parser.ParsePart(json2,sizeof(json2) - 1);
        parser.ParsePart(json3,sizeof(json3) - 1);
    }

    if (reader.HasParseError()) {
        std::cout << "Error at offset " << reader.GetErrorOffset() << ": " << GetParseError_En(reader.GetParseErrorCode()) << std::endl;
        return EXIT_FAILURE;
    }


//    Document d;

//    {
//        AsyncDocumentParser<> parser(d);

//        const char json1[] = " { \"hello\" : \"world\", \"t\" : tr";
//        //const char json1[] = " { \"hello\" : \"world\", \"t\" : trX"; // For test parsing error
//        const char json2[] = "ue, \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.14";
//        const char json3[] = "16, \"a\":[1, 2, 3, 4] } ";

//        parser.ParsePart(json1, sizeof(json1) - 1);
//        parser.ParsePart(json2, sizeof(json2) - 1);
//        parser.ParsePart(json3, sizeof(json3) - 1);
//    }

//    if (d.HasParseError()) {
//        std::cout << "Error at offset " << d.GetErrorOffset() << ": " << GetParseError_En(d.GetParseError()) << std::endl;
//        return EXIT_FAILURE;
//    }



    return EXIT_SUCCESS;
}


#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include <cstdio>
#include <iostream>

#include <QtCore>

#include <thread>
#include <mutex>
#include <condition_variable>
#include "curl/curl.h"

using namespace rapidjson;
using namespace std;


struct MyHandler : public BaseReaderHandler<UTF8<>, MyHandler> {
    MyHandler():count(0){}
    bool Null() { cout << "Null()" << endl; return true; }
    bool Bool(bool b) { cout << "Bool(" << boolalpha << b << ")" << endl; return true; }
    bool Int(int i) { cout << "Int(" << i << ")" << endl; return true; }
    bool Uint(unsigned u) { cout << "Uint(" << u << ")" << endl; return true; }
    bool Int64(int64_t i) { cout << "Int64(" << i << ")" << endl; return true; }
    bool Uint64(uint64_t u) { cout << "Uint64(" << u << ")" << endl; return true; }
    bool Double(double d) { cout << "Double(" << d << ")" << endl; return true; }
    bool String(const char* str, SizeType length, bool copy) {
        cout << "String(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;
        return true;
    }
    bool StartObject() { cout << "StartObject()" << endl; count++; return true; }
    bool Key(const char* str, SizeType length, bool copy) {
        cout << "Key(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;
        return true;
    }
    bool EndObject(SizeType memberCount) { cout << "EndObject(" << memberCount << ")" << endl; return true; }
    bool StartArray() { cout << "StartArray()" << endl; return true; }
    bool EndArray(SizeType elementCount) { cout << "EndArray(" << elementCount << ")" << endl; return true; }


    int count;
};

class streamWrapper{
public:
    streamWrapper(char* buffer, size_t bufferSize)
        : stream_(*this,buffer,bufferSize)
        , mutex_()
        , notEmpty_()
        , Empty_()
        , start_(true)
    {
    }

class FileReadStream {
public:
    typedef char Ch;    //!< Character type (byte).

    //! Constructor.
    /*!
        \param buffer user-supplied buffer.
        \param bufferSize size of buffer in bytes. Must >=4 bytes.
    */
    FileReadStream(streamWrapper& wrapper
                   ,char* buffer
                   , size_t bufferSize) :
        wrapper_(wrapper)
      , buffer_(buffer)
      , bufferSize_(bufferSize)
      , bufferLast_(0), current_(buffer_)
      , readCount_(0)
      , count_(0)
      {

        RAPIDJSON_ASSERT(bufferSize >= 4);
    }
    ~FileReadStream(){
        //< 防止另外一个线程阻塞
        wrapper_.Empty_.notify_one();
    }

    Ch Peek() const {
        //< 如果缓冲区没有更新,阻塞
        std::unique_lock<std::mutex> lock(wrapper_.mutex_);
        while(Empty())
        {
           wrapper_.Empty_.notify_one();
           wrapper_.notEmpty_.wait(lock);
        }
        wrapper_.start_ = false;
        return *current_;
    }
    Ch Take() {
        //< 锁-检测buffer是否更新
        {
            //< 如果缓冲区没有更新,阻塞
            std::unique_lock<std::mutex> lock(wrapper_.mutex_);
            while(Empty())
            {
                wrapper_.Empty_.notify_one();
                wrapper_.notEmpty_.wait(lock);
            }
            wrapper_.start_ = false;
        }


        Ch c = *current_; Read(); return c;
    }
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

    bool Empty() const
    {
        if(wrapper_.start_)
            return true;
        else
            return current_ == bufferLast_;
    }


    void Read() {
        if (current_ < bufferLast_)
            ++current_;
        else {
            count_ += readCount_;

            //< 锁-检测buffer是否更新
            {
                //< 如果缓冲区没有更新,阻塞
                std::unique_lock<std::mutex> lock(wrapper_.mutex_);
                while(Empty())
                {
                    wrapper_.Empty_.notify_one();
                    wrapper_.notEmpty_.wait(lock);
                }
                wrapper_.start_ = false;
            }
        }
    }
    streamWrapper& wrapper_;
    Ch *buffer_;
    size_t bufferSize_;
    Ch *bufferLast_;
    Ch *current_;
    size_t readCount_;
    size_t count_;  //!< Number of characters read

};

// 类成员
    FileReadStream stream_;
    std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable Empty_;
    bool start_;
};

size_t write_data_call_back(char *buf, size_t size,
                                          size_t nmemb, streamWrapper *pIs)
{
    const size_t nRealSize = size * nmemb;
    int realSize = static_cast<int>(nRealSize);

    //< 等待为空,不为空,阻塞.
    {
        std::unique_lock<std::mutex> lock(pIs->mutex_);
        while(!pIs->stream_.Empty())
            pIs->Empty_.wait(lock);

        //拷贝数据
        memcpy(pIs->stream_.buffer_,buf,realSize);
        pIs->stream_.readCount_ = realSize;
        pIs->stream_.bufferLast_ = pIs->stream_.buffer_ + realSize - 1;
        pIs->stream_.current_ = pIs->stream_.buffer_;
        pIs->notEmpty_.notify_one();
        pIs->start_=false;
    }

    return nRealSize;
}

void foo(streamWrapper* pIs)
{
    auto pCurl = curl_easy_init();
//    char* pStrquery = "http://127.0.0.1:8086/query?db=iscs6000&chunked=true&chunk_size=20&q=select%20%2A%20from%20ai_sample_result";
    char* pStrquery = "http://127.0.0.1:8086/query?db=NOAA_water_database&chunked=true&chunk_size=20&q=select%20%2A%20from%20h2o_pH";

    curl_easy_reset(pCurl);
    curl_easy_setopt(pCurl, CURLOPT_URL, pStrquery);
    curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION,
                      write_data_call_back);
    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, pIs);
    curl_easy_setopt(pCurl, CURLOPT_TIMEOUT_MS, 0);

    CURLcode nRc = curl_easy_perform(pCurl);
    cout << "fuck" << endl;
}

//void parsethread(FileReadStream* pIs)
//{
//    char buffer[65535];
//    pIs = new FileReadStream(buffer,sizeof(buffer));
//}



int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    using namespace rapidjson;
    char buffer[65535];
    streamWrapper wrapper(buffer,sizeof(buffer));

    Reader reader;
    MyHandler handler;
    thread t2(foo,&wrapper);


    while(1)
        reader.Parse<kParseStopWhenDoneFlag>(wrapper.stream_,handler);
    cout <<"count :" << handler.count <<endl;

}

