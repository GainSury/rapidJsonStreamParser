
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include <cstdio>
#include <iostream>
#include "rapidjson/error/en.h"
#include <QtCore>

#include <thread>
#include <mutex>
#include <condition_variable>
#include "curl/curl.h"

#include "curl_fopen.h"
#include <vector>

using namespace rapidjson;
using namespace std;


struct MyHandler : public BaseReaderHandler<UTF8<>, MyHandler> {
    MyHandler():count(0){}
    bool Null() { cout << "Null()" << endl; return true; }
    bool Bool(bool b) { cout << "Bool(" << boolalpha << b << ")" << endl; return true; }
    bool String(const char* str, SizeType length, bool copy) {
        cout << "String(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;
        return true;
    }
    bool RawNumber(const Ch* str, SizeType length, bool copy){
        cout << "RawNumber(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;
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



struct JsonHandler : public BaseReaderHandler<UTF8<>, MyHandler> {
    JsonHandler():
        state_(kExpectObjectStart){
        //< todo to initialize
    }
    bool StartObject() {
        cout << "before StartObject():"  << stateToString(state_) << endl;

        switch (state_) {
        case kExpectObjectStart:
            state_ = kExpectResultOrErr;
            return true;
        default:
            return false;
        }



    }
    bool EndObject(SizeType memberCount) {
        cout << "before EndObject():"  << stateToString(state_) << endl;

        switch (state_) {
        case kExpectObjectEnd:
            state_ = kObjectEnd;
            return true;
        default:
            return false;
        }
    }
    bool StartArray() {
        cout << "before StartArray():"  << stateToString(state_) << endl;

        switch (state_) {
        case kExpectObjectStart:
            state_ = kExpectResultOrErr;
            return true;
        default:
            return false;
        }
    }
    bool EndArray(SizeType elementCount) {
        cout << "before EndArray():"  << stateToString(state_) << endl;

        switch (state_) {
        case kExpectObjectStart:
            state_ = kExpectResultOrErr;
            return true;
        default:
            return false;
        }
    }


    bool Key(const char* str, SizeType length, bool copy) {
        cout << "before Key():"  << stateToString(state_) ;
        cout << "||  Key(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;

        if( strcmp( str, "error") == 0 && state_ == kExpectResultOrErr)
        {
            state_ = kExpectErrDesc;
            return true;
        }
        else if(strcmp( str, "result" ) == 0 && state_ == kExpectResultOrErr )
        {
            state_ = kExpectResultArrayStart;
            return true;
        }
        else
            return false;
//        switch (state_) {
//        case kExpectObjectStart:
//            state_ = kExpectResultOrErr;
//            return true;
//        default:
//            return false;
//        }
    }

    bool Null() {
        cout << "before Null():"  << stateToString(state_) ;
        cout << "|| Null()" << endl;
        switch (state_) {
        case kExpectObjectStart:
            state_ = kExpectResultOrErr;
            return true;
        default:
            return false;
        }
    }
    bool Bool(bool b) {
        cout << "before Bool():"  << stateToString(state_) ;
        cout << "|| Bool(" << boolalpha << b << ")" << endl;
        switch (state_) {
        case kExpectObjectStart:
            state_ = kExpectResultOrErr;
            return true;
        default:
            return false;
        }
    }
    bool String(const char* str, SizeType length, bool copy) {
        cout << "before String():"  << stateToString(state_) ;
        cout << "|| String(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;
        switch (state_) {
        case kExpectErrDesc:
           state_ = kExpectObjectEnd;
           return true;
        default:
           return false;
        }
    }
    bool RawNumber(const Ch* str, SizeType length, bool copy){
        cout << "before RawNumber():"  << stateToString(state_) ;
        cout << "|| RawNumber(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;
        switch (state_) {
        case kExpectObjectStart:
           state_ = kExpectResultOrErr;
           return true;
        default:
           return false;
        }
    }





    std::string currentName;
    std::vector<std::string> currentColumnName;
    std::vector<std::string> currentColumnValue;

    //设备状态
    enum State {
           kExpectObjectStart,
           kExpectResultOrErr,
           kExpectErrDesc,
           kExpectObjectEnd,
           kObjectEnd,
           kExpectNameOrObjectEnd,
           kExpectResultArrayStart,
           kExpectResultObjectStart,
           kExpectKeyStateMentId,
           kExpectValueNum,
           kExpectKeySeries,
           kExpectSeriesArrayStart,
           kExpectSeriesObjectStart,
           kExpectKeyName,
           kExpectNameValue,
           kExpectKeyColumns,
           kExpectKeyColumnsArrayStart,
           kExpectColumnsValueOrArrayEnd,
           kExpectKeyValues,
           kExpectValuesArrayStart,
           kExpectValuesArrayDetailsArrayStart,
           kExpectValuesArrayDetailsValueOrArrayEnd,
           kExpectValuesArrayEndOrDetailsArrayStart,
           kExpectSeriesObjectEnd,
           kExpectSeriesArrayEnd,
           kExpectResultObjectEnd,
           kExpectResultArrayEnd
   }state_;

   static std::string stateToString(State state)
   {
       switch (state) {
       case kExpectObjectStart:
           return "kExpectObjectStart";
       case kExpectResultOrErr:
           return "kExpectResultOrErr";
       case kExpectErrDesc:
           return "kExpectErrDesc";
       case kExpectObjectEnd:
           return "kExpectObjectEnd";
       case kObjectEnd:
           return "kObjectEnd";
       case kExpectNameOrObjectEnd:
           return "kExpectNameOrObjectEnd";
       case kExpectResultArrayStart:
           return "kExpectResultArrayStart";
       case kExpectResultObjectStart:
           return "kExpectResultObjectStart";
       case kExpectKeyStateMentId:
           return "kExpectKeyStateMentId";
       case kExpectValueNum:
           return "kExpectValueNum";
       case kExpectKeySeries:
           return "kExpectKeySeries";
       case kExpectSeriesArrayStart:
           return "kExpectSeriesArrayStart";
       case kExpectSeriesObjectStart:
           return "kExpectSeriesObjectStart";
       case kExpectKeyName:
           return "kExpectKeyName";
       case kExpectNameValue:
           return "kExpectNameValue";
       case kExpectKeyColumns:
           return "kExpectKeyColumns";
       case kExpectKeyColumnsArrayStart:
           return "kExpectKeyColumnsArrayStart";
       case kExpectColumnsValueOrArrayEnd:
           return "kExpectColumnsValueOrArrayEnd";
       case kExpectKeyValues:
           return "kExpectKeyValues";
       case kExpectValuesArrayStart:
           return "kExpectValuesArrayStart";
       case kExpectValuesArrayDetailsArrayStart:
           return "kExpectValuesArrayDetailsArrayStart";
       case kExpectValuesArrayDetailsValueOrArrayEnd:
           return "kExpectValuesArrayDetailsValueOrArrayEnd";
       case kExpectValuesArrayEndOrDetailsArrayStart:
           return "kExpectValuesArrayEndOrDetailsArrayStart";
       case kExpectSeriesObjectEnd:
           return "kExpectSeriesObjectEnd";
       case kExpectSeriesArrayEnd:
           return "kExpectSeriesArrayEnd";
       case kExpectResultObjectEnd:
           return "kExpectResultObjectEnd";
       case kExpectResultArrayEnd:
           return "kExpectResultArrayEnd";


       default:
           return "unknown state";
       }
   }
};










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






int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    using namespace rapidjson;

//    const char *url =  "http://127.0.0.1:8086/query?db=iscs6000&chunked=true&chunk_size=20&epoch=ms&q=select%20%2A%20from%20ai_sample_result%20limit%2020";
//    const char *url = "http://127.0.0.1:8086/query?db=NOAA_water_database&chunked=true&chunk_size=20&q=select%20%2A%20from%20h2o_pH";

    // for mac err test
    const char *url = "http://127.0.0.1:8086/query?db=NOAA_water_database&chunked=true&chunk_size=20&q1=select%20%2A%20from%20h2o_pH";
    URL_FILE *handle;
    handle = url_fopen(url, "r");
//    handle = url_fopen(url_err_test, "r");
    if(!handle) {
      printf("couldn't url_fopen() %s\n", url);
      return 2;
    }

    UrlReadStream stream(handle);

    Reader reader;
    JsonHandler handler;


    while(1)
    {
        if(reader.Parse<kParseNumbersAsStringsFlag>(stream,handler))
            break;
    }

    if (reader.HasParseError()) {
        std::cout << "Error at offset " << reader.GetErrorOffset() << ": " << GetParseError_En(reader.GetParseErrorCode()) << std::endl;
        return EXIT_FAILURE;
    }
     url_fclose(handle);

}

