#ifndef CGZIPOSTREAM_H
#define CGZIPOSTREAM_H

#include <QtCore>
#include <iostream>
#include "zlib.h"


class CGZipOStream
{
public:

    CGZipOStream(QString &filePath_);
    ~CGZipOStream();

    operator bool() const;

//    friend CGZipOStream& operator <<(CGZipOStream& os,QByteArray& ba)
//    {
//        QByteArray compBuf;
//        if(QCompressor::gzipCompress(ba, compBuf)){
//            os.m_destFile.write(compBuf);
//        }

//        return os;
//    }

    friend CGZipOStream& operator <<(CGZipOStream& os,const char* data){
        if( os.m_nCurIdx + sizeof(data) > os.m_nBufSize -1)
        {
            //< 处理数据
            os.m_objStream.avail_in = (uInt)os.m_nCurIdx;
            os.m_objStream.avail_out = (uInt)os.m_nBufSize; // 不检查avail_out 因为分配了足够的空间

            deflate( &( os.m_objStream ),Z_NO_FLUSH);
            //< 写入流数据
            int nRc = os.m_destFile.write((char *)os.m_objStream.next_out
                                , os.m_nBufSize - os.m_objStream.avail_out);
            std::cout << "nRc: " << nRc << " writesize: "<< os.m_nBufSize - os.m_objStream.avail_out << std::endl;


            os.m_nCurIdx = 0;

        }
        memcpy(os.m_inBuf+os.m_nCurIdx,data,sizeof(data));

        os.m_nCurIdx += sizeof(data);
        return os;
    }
private:
    QFile m_destFile;
    bool m_openResult;
    z_stream m_objStream;
    char* m_inBuf;
    char* m_outBuf;
    const size_t m_nBufSize;
    size_t m_nCurIdx; //当前缓冲区需要压缩的数据长度
    static bool compressToGzipTest(const void *pDataSrc, size_t nSizeSrc,
                                      void **ppDataDst, size_t *pnSizeDst)
    {
        if (NULL == pDataSrc || 0 == nSizeSrc ||
            NULL == ppDataDst || NULL == pnSizeDst)
        {
    //        LOGERROR("compressToGzip(): NULL == pDataSrc || 0 == nSizeSrc || "
    //                 "NULL == ppDataDst || NULL == pnSizeSrc , return false !");
            return false;
        }

        if (nSizeSrc >= 2147483647)
        {
            //< zlib支持的大小不止这么多，保险起见
    //        LOGERROR("compressToGzip(): nSizeSrc >= 2147483647 , too big to compress !");
            return false;
        }

        z_stream objStream;

        objStream.zalloc = Z_NULL;
        objStream.zfree = Z_NULL;
        objStream.opaque = Z_NULL;

        //< MAX_WBITS+16 : 使生成的目标内存中包含gzip头尾，见zlib说明
        if (deflateInit2(&objStream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                         MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
        {
    //        LOGERROR("compressToGzip(): deflateInit2 failed !");
            return false;
        }

        //< 计算压缩后的最大长度
        const size_t nSizeDstMax = deflateBound(&objStream, (uLong)nSizeSrc);

        if (nSizeDstMax >= 2147483647)
        {
            //< zlib支持的大小不止这么多，保险起见
    //        LOGERROR("compressToGzip(): nSizeDstMax >= 2147483647 , too big to compress !");
            return false;
        }

        if (NULL != *ppDataDst)
        {
            //< 不用realloc，避免可能发生的拷贝
            free(*ppDataDst);
        }

        //< 注意，函数调用者需在外部释放内存
        *ppDataDst = malloc(nSizeDstMax);
        if (NULL == *ppDataDst)
        {
    //        LOGERROR("compressToGzip(): malloc() failed !");
            return false;
        }

        objStream.next_in  = (z_const Bytef *)pDataSrc;
        objStream.avail_in  = (uInt)nSizeSrc;
        objStream.next_out = (Bytef *)(*ppDataDst);
        objStream.avail_out  = (uInt)nSizeDstMax;

        const int nRc = deflate(&objStream, Z_FINISH);

        if (deflateEnd(&objStream) != Z_OK)
        {
    //        LOGERROR("compressToGzip(): deflateEnd(&objStream) != Z_OK");
        }

        if (Z_STREAM_END == nRc)
        {
            //LOGDEBUG("compressToGzip(): OK ! nSizeSrc = %lu , nSizeDst = %lu",
            //         (unsigned long long)nSizeSrc,
            //         (unsigned long long)(objStream.total_out));

            //< 外部释放 *ppDataDst
            *pnSizeDst = objStream.total_out;
            return true;
        }
        else

    //    LOGERROR("compressToGzip(): deflate() != Z_STREAM_END , return false !");

        free(*ppDataDst);
        *ppDataDst = NULL;
        *pnSizeDst = 0;

        return false;
    }

};

#endif // CGZIPOSTREAM_H
