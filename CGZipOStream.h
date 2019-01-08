#ifndef CGZIPOSTREAM_H
#define CGZIPOSTREAM_H

#include <QtCore>
#include "zlib.h"
#include <iostream>

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

                os.m_openResult = false;
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
    bool m_openResult;
    z_stream m_objStream;
    char* m_inBuf;
    char* m_outBuf;
    const size_t m_nBufSize;
    size_t m_nInLen; //当前缓冲区需要压缩的数据长度
};

#endif // CGZIPOSTREAM_H
