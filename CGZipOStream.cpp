#include "CGZipOStream.h"
#include "qcompressor.h"

CGZipOStream::CGZipOStream(QString filePath_):
    m_destFile(filePath_)
  , m_nBufSize(1 << 16) //65536
  , m_nInLen(0)
{
    m_openResult = m_destFile.open(QIODevice::WriteOnly);

    //< 初始化z_stream头
    m_objStream.zalloc = Z_NULL;
    m_objStream.zfree = Z_NULL;
    m_objStream.opaque = Z_NULL;

    //< MAX_WBITS+16 : 使生成的目标内存中包含gzip头尾
    int nRet = deflateInit2(&m_objStream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                     MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY);

    if(nRet != Z_OK)
    {
        // todo logerrror

        m_openResult = false;
        return ;
    }

    //< 分配缓冲区
    m_inBuf = (char*)malloc(m_nBufSize * sizeof(char));
    m_outBuf =(char*)malloc(m_nBufSize * sizeof(char));
}

CGZipOStream::~CGZipOStream()
{
    //< 更新输入输出缓冲区的参数
    m_objStream.next_in =  (z_const Bytef *)m_inBuf;
    m_objStream.avail_in = (uInt)m_nInLen;

    int nRet;

    do{
        m_objStream.next_out = (Bytef *)m_outBuf;
        m_objStream.avail_out = (uInt)m_nBufSize; // 不检查avail_out 因为分配了足够的空间
        nRet = deflate( &m_objStream , Z_FINISH );
        //< 写入流数据
        int nRcWrite = m_destFile.write((char *)m_outBuf
                            , (char*)m_objStream.next_out - m_outBuf);
        std::cout << "finish Write: " << nRcWrite << " writesize: "<< (char*)m_objStream.next_out - m_outBuf << std::endl;
        if(nRet == Z_STREAM_END )
            break;
    }while( m_objStream.avail_out == 0 );

    if (deflateEnd(&m_objStream) != Z_OK)
    {
       //< logerror
    }


    free(m_outBuf);
    free(m_inBuf);
    m_destFile.close();


}

CGZipOStream::operator bool() const
{
    return m_openResult;
}






