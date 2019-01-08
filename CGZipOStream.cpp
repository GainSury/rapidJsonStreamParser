#include "CGZipOStream.h"
#include "qcompressor.h"

CGZipOStream::CGZipOStream(QString &filePath_):
    m_destFile(filePath_)
  , m_nBufSize(1 << 16) //65536
  , m_nCurIdx(0)
{
    m_openResult = m_destFile.open(QIODevice::WriteOnly);

    //< 初始化z_stream头
    m_objStream.zalloc = Z_NULL;
    m_objStream.zfree = Z_NULL;
    m_objStream.opaque = Z_NULL;

    //< MAX_WBITS+16 : 使生成的目标内存中包含gzip头尾
    if (deflateInit2(&m_objStream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                     MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
    {
       //< 处理错误
    }

    m_inBuf = new char[m_nBufSize];
    m_outBuf = new char[m_nBufSize];
    m_objStream.next_in =  (z_const Bytef *)m_inBuf;
    m_objStream.next_out = (Bytef *)m_outBuf;

}

CGZipOStream::~CGZipOStream()
{
    const int nRc = deflate( &m_objStream , Z_FINISH );


    if (deflateEnd(&m_objStream) != Z_OK)
    {
       //< 记录错误
    }

    if (Z_STREAM_END == nRc)
    {

        delete m_outBuf;
        delete m_inBuf;

    }
    else{
            //< 记录错误
    }
}

CGZipOStream::operator bool() const
{
    return m_openResult;
}





//CGZipOStream &CGZipOStream::operator <<(CGZipOStream &os, QByteArray &ba){
//    QByteArray compBuf;
//    if(QCompressor::gzipCompress(ba, compBuf)){
//        os.m_destFile.write(compBuf);
//    }
//    return os;
//}

//CGZipOStream &CGZipOStream::operator <<(CGZipOStream &os, const char *data){
//    return os << QByteArray(data);
//}
