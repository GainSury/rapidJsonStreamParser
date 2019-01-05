
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include <cstdio>
#include <iostream>
#include "rapidjson/error/en.h"
#include <QtCore>

#include "curlreadstream.h"


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



struct CChunkedJsonHandler : public BaseReaderHandler<UTF8<>, MyHandler> {
    CChunkedJsonHandler():
        state_(kExpectObjectStart),
        nIndex(0)
    {
        //< todo to initialize


    }
    bool StartObject() {
//        cout << "before StartObject():"  << stateToString(state_) << endl;

        switch (state_) {
        case kExpectObjectStart:
//            cout << "\n\n\nnew object ||||\n\n\n" << endl;
            state_ = kExpectResultOrErr;
            return true;
        case kExpectResultObjectStart:
            state_ = kExpectKeyStateMentId;
            return true;
        case kExpectSeriesObjectStart:
            state_ = kExpectKeyName;
            return true;
        default:
            return false;
        }



    }
    bool EndObject(SizeType memberCount) {
//        cout << "before EndObject():"  << stateToString(state_) << endl;

        switch (state_) {
        case kExpectObjectEnd:
            state_ = kExpectObjectStart;
            return true;
        case kExpectSeriesObjectEndOrKeyPartial:
            state_ = kExpectSeriesArrayEnd;
            return true;
        case kExpectResultObjectEndOrKeyPartial:
            state_ = kExpectResultArrayEnd;
            return true;
        default:
            return false;
        }
    }
    bool StartArray() {
//        cout << "before StartArray():"  << stateToString(state_) << endl;

        switch (state_) {
        case kExpectObjectStart:
            state_ = kExpectResultOrErr;
            return true;
        case kExpectResultArrayStart:
            state_ = kExpectResultObjectStart;
            return true;
        case kExpectSeriesArrayStart:
            state_ = kExpectSeriesObjectStart;
            return true;
        case kExpectColumnsArrayStart:
            state_ = kExpectColumnsValueOrArrayEnd;
            return true;
        case kExpectValuesArrayStart:
            state_ = kExpectValuesArrayDetailsArrayStart;
            return true;
        case kExpectValuesArrayDetailsArrayStart:
            state_ = kExpectValuesArrayDetailsValueOrArrayEnd;
            return true;
        case kExpectValuesArrayEndOrDetailsArrayStart:
            state_ = kExpectValuesArrayDetailsValueOrArrayEnd;
            return true;


        default:
            return false;
        }
    }
    bool EndArray(SizeType elementCount) {
//        cout << "before EndArray():"  << stateToString(state_) << endl;

        switch (state_) {
        case kExpectObjectStart:
            state_ = kExpectResultOrErr;
            return true;
        case kExpectColumnsValueOrArrayEnd:
            state_ = kExpectKeyValues;
            return true;
        case kExpectValuesArrayDetailsValueOrArrayEnd:
            std::cout << m_strCurMeasurementName << ",key_id_tag=";


            std::cout << m_strVecvalues[1] << " status=";//输出tag
            std::cout << m_strVecvalues[2] << ",value=" << m_strVecvalues[3] << " time=";         //输出field
            std::cout << m_strVecvalues[0] <<"\n";

            ++nIndex;
            m_strVecvalues.clear();
            state_= kExpectValuesArrayEndOrDetailsArrayStart;

           return true;
        case kExpectValuesArrayEndOrDetailsArrayStart:

            state_= kExpectSeriesObjectEndOrKeyPartial;
           return true;
        case kExpectSeriesArrayEnd:
            state_ = kExpectResultObjectEndOrKeyPartial;
            return true;
        case kExpectResultArrayEnd:
            state_ = kExpectObjectEnd;
            return true;
        default:
            return false;
        }
    }


    bool Key(const char* str, SizeType length, bool copy) {
//        cout << "before Key():"  << stateToString(state_) ;
//        cout << "||  Key(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;

        switch (state_) {
        case kExpectResultOrErr:
            if( strcmp(str,"error")  == 0)
            {
                state_ = kExpectErrDesc;
                return true;
            }
            else if (strcmp( str, "results" ) == 0)
            {
                state_ = kExpectResultArrayStart;
                return true;
            }
            return false;

        case kExpectKeyStateMentId:
            state_ = kExpectValueNum;
            return true;
        case kExpectKeySeries:
            state_ = kExpectSeriesArrayStart;
            return true;
        case kExpectKeyName:
            state_ = kExpectNameValue;
            return true;
        case kExpectKeyColumns:
            state_ = kExpectColumnsArrayStart;
            return true;
        case kExpectKeyValues:
            state_ = kExpectValuesArrayStart;
            return true;
        case kExpectSeriesObjectEndOrKeyPartial:
            state_ = kExpectSeriesObjectValueBool;
            return true;
        case kExpectResultObjectEndOrKeyPartial:
            state_ = kExpectResultObjectValueBool;
            return true;
        default:
            return false;
        }
    }

    bool Null() {
//        cout << "before Null():"  << stateToString(state_) ;
//        cout << "|| Null()" << endl;
        switch (state_) {
        case kExpectObjectStart:
            state_ = kExpectResultOrErr;
            return true;
        case kExpectValuesArrayDetailsValueOrArrayEnd:
            m_strVecvalues.push_back("null");
           return true;
        default:
            return false;
        }
    }
    bool Bool(bool b) {
//        cout << "before Bool():"  << stateToString(state_) ;
//        cout << "|| Bool(" << boolalpha << b << ")" << endl;
        switch (state_) {
        case kExpectObjectStart:
            state_ = kExpectResultOrErr;
            return true;
        case kExpectValuesArrayDetailsValueOrArrayEnd:
           if(b == true){
               m_strVecvalues.push_back("true");
           }
           else{
               m_strVecvalues.push_back("false");
           }

           return true;
        case kExpectSeriesObjectValueBool:
            state_ = kExpectSeriesObjectEndOrKeyPartial;
            return true;
        case kExpectResultObjectValueBool:
            state_ = kExpectResultObjectEndOrKeyPartial;
            return true;
        default:
            return false;
        }
    }
    bool String(const char* str, SizeType length, bool copy) {
//        cout << "before String():"  << stateToString(state_) ;
//        cout << "|| String(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;
        switch (state_) {
        case kExpectErrDesc:
           state_ = kExpectObjectEnd;
           return true;
        case kExpectNameValue:
           state_ = kExpectKeyColumns;

           // 记录当前的measurements值
           m_strCurMeasurementName = str;
           return true;
        case kExpectColumnsValueOrArrayEnd:
           return true;
        case kExpectValuesArrayDetailsValueOrArrayEnd:
            m_strVecvalues.push_back(str);
           return true;
        default:
           return false;
        }
    }
    bool RawNumber(const Ch* str, SizeType length, bool copy){
//        cout << "before RawNumber():"  << stateToString(state_) ;
//        cout << "|| RawNumber(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;
        switch (state_) {
        case kExpectValueNum:
           state_ = kExpectKeySeries;
           return true;
        case kExpectValuesArrayDetailsValueOrArrayEnd:
            m_strVecvalues.push_back(str);
           return true;
        default:
           return false;
        }
    }


    std::string m_strCurMeasurementName;

    std::vector<std::string> m_strVecvalues;
    size_t nIndex;

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
           kExpectColumnsArrayStart,
           kExpectColumnsValueOrArrayEnd,
           kExpectKeyValues,
           kExpectValuesArrayStart,
           kExpectValuesArrayDetailsArrayStart,
           kExpectValuesArrayDetailsValueOrArrayEnd,
           kExpectValuesArrayEndOrDetailsArrayStart,
           kExpectSeriesObjectEndOrKeyPartial,
           kExpectSeriesArrayEnd,
           kExpectResultObjectEndOrKeyPartial,
           kExpectResultArrayEnd,
           kExpectSeriesObjectValueBool,
           kExpectResultObjectValueBool,
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
       case kExpectColumnsArrayStart:
           return "kExpectColumnsArrayStart";
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
       case kExpectSeriesObjectEndOrKeyPartial:
           return "kExpectSeriesObjectEndOrKeyPartial";
       case kExpectSeriesArrayEnd:
           return "kExpectSeriesArrayEnd";
       case kExpectResultObjectEndOrKeyPartial:
           return "kExpectResultObjectEndOrKeyPartial";
       case kExpectResultArrayEnd:
           return "kExpectResultArrayEnd";
       case kExpectSeriesObjectValueBool:
                  return "kExpectSeriesObjectValueBool";
       case kExpectResultObjectValueBool:
                  return "kExpectResultObjectValueBool";

       default:
           return "unknown state";
       }
   }
};


int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    using namespace rapidjson;

    //for linux test
    const char *url =  "http://127.0.0.1:8086/query?db=iscs6000&chunked=true&chunk_size=2&epoch=ms&q=select%20%2A%20from%20ai_sample_result%20limit%201000000";

    //for linux test generate line protocol
//    const char *url =    "http://127.0.0.1:8086/query?db=iscs6000&chunked=true&pretty=true&chunk_size=2&epoch=ms&q=SELECT%20key_id_tag%2Cstatus%2Cvalue%2Ctime%20FROM%20ai_sample_result%20limit%2010";


    //for mac test
//    const char *url = "http://127.0.0.1:8086/query?db=NOAA_water_database&chunked=true&chunk_size=20&q=select%20%2A%20from%20h2o_pH";

    //for linux err test3
//    const char *url =  "http://127.0.0.1:8086/query?db=iscs6000&chunked=true&chunk_size=20&epoch=ms&q1=select%20%2A%20from%20ai_sample_result%20limit%2020";

    // for mac err test
//    const char *url = "http://127.0.0.1:8086/query?db=NOAA_water_database&chunked=true&chunk_size=20&q1=select%20%2A%20from%20h2o_pH";
    URL_FILE *handle;
    handle = url_fopen(url, "r");
//    handle = url_fopen(url_err_test, "r");
    if(!handle) {
      printf("couldn't url_fopen() %s\n", url);
      return 2;
    }


    UrlReadStream stream(handle);

    Reader reader;
//    MyHandler handler;
    CChunkedJsonHandler handler;


    while(1)
    {
        if(reader.Parse<kParseNumbersAsStringsFlag>(stream,handler))
            break;
    }

    if (reader.HasParseError()) {
        std::cout << "Error at offset " << reader.GetErrorOffset() << ": " << GetParseError_En(reader.GetParseErrorCode()) << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "count: " << handler.nIndex << std::endl;
    int nStatusCode = 0;
    int nRc = curl_easy_getinfo(handle->handle.curl,CURLINFO_RESPONSE_CODE,&nStatusCode);

    std::cout << "status code: " << nStatusCode  << std::endl;

     url_fclose(handle);

}

