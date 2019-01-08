#ifndef CCHUNKEDJSONHANDLER_H
#define CCHUNKEDJSONHANDLER_H

#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"
#include <vector>
#include <string>
#include <iostream>

#include "CGZipOStream.h"
struct CChunkedJsonHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, CChunkedJsonHandler> {
    typedef rapidjson::SizeType SizeType;
    CChunkedJsonHandler(QString strFilePath);
    class CGZipOStream
    {
    public:

        CGZipOStream(QString filePath_);
        ~CGZipOStream();

        operator bool() const;
        friend CGZipOStream &operator <<(CGZipOStream& os,const char* data){
            //< 获取数据长度
            int nDataStrlen = strlen(data);

            //< 数据缓存到缓冲区,如果缓冲区长度不够,则压缩输入缓冲区中的数据
            if( os.m_nInLen + nDataStrlen > os.m_nBufSize)
            {
                //< 更新输入输出缓冲区的参数
                os.m_objStream.next_in =  (z_const Bytef *)os.m_inBuf;
                os.m_objStream.next_out = (Bytef *)os.m_outBuf;
                os.m_objStream.avail_in = (uInt)os.m_nInLen; //设置为当前输入缓冲区的数据长度
                os.m_objStream.avail_out = (uInt)os.m_nBufSize;

                int nRet = deflate( &( os.m_objStream ),Z_NO_FLUSH);
                if(nRet != Z_OK)
                {
                    //< todo logerror

                    os.m_bOkState = false;
                    return os;
                }

                //< 将压缩数据写入到文件中
                int nRc = os.m_destFile.write((char *)os.m_outBuf
                                    , (char*)os.m_objStream.next_out - os.m_outBuf);
                std::cout << "nRc: " << nRc << " writesize: "<< (char*)os.m_objStream.next_out - os.m_outBuf << std::endl;
                if(nRc != (char*)os.m_objStream.next_out - os.m_outBuf)
                {
                    //< todo logerror 写入文件流有错
                }

                //< 更新当前输入缓冲区数据长度
                os.m_nInLen = 0;

            }
            //< 追加输入数据到输入缓冲区
            memcpy( os.m_inBuf + os.m_nInLen, data, nDataStrlen);
            os.m_nInLen += nDataStrlen;
            return os;
        }


    private:
        QFile m_destFile;
        bool m_bOkState;
        z_stream m_objStream;
        char* m_inBuf;
        char* m_outBuf;
        const size_t m_nBufSize;
        size_t m_nInLen; //当前缓冲区需要压缩的数据长度
    };
    bool StartObject();
    bool EndObject(SizeType memberCount);

    bool StartArray();
    bool EndArray(SizeType elementCount);

    bool Key(const char* str, SizeType length, bool copy);

    bool Null();
    bool Bool(bool b);
    bool String(const char* str, SizeType length, bool copy);
    bool RawNumber(const Ch* str, SizeType length, bool copy);


    //< 状态机
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

    static void genDMLHeader(CGZipOStream& os);


    std::string m_strCurMeasurementName;
    std::vector<std::string> m_strVecvalues;
    size_t nIndex;
    CGZipOStream m_gGzipOs;

    static std::string stateToString(State state);




};

#endif // CCHUNKEDJSONHANDLER_H
