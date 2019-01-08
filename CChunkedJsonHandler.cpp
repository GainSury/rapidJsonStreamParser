#include "CChunkedJsonHandler.h"



CChunkedJsonHandler::CChunkedJsonHandler(QString strFilePath):
    state_(kExpectObjectStart),
    nIndex(0),
    m_gGzipOs(strFilePath)
{
    if(!m_gGzipOs)
    {
        //< logerror
        return ;
    }

    genDMLHeader(m_gGzipOs);
}



bool CChunkedJsonHandler::StartObject() {

    switch (state_) {
    case kExpectObjectStart:
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

bool CChunkedJsonHandler::EndObject(SizeType memberCount) {
    Q_UNUSED(memberCount);
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

bool CChunkedJsonHandler::StartArray() {

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

bool CChunkedJsonHandler::EndArray(SizeType elementCount) {
    Q_UNUSED(elementCount);
    switch (state_) {
    case kExpectObjectStart:
        state_ = kExpectResultOrErr;
        return true;
    case kExpectColumnsValueOrArrayEnd:
        state_ = kExpectKeyValues;
        return true;
    case kExpectValuesArrayDetailsValueOrArrayEnd:
        m_gGzipOs << m_strCurMeasurementName.c_str() << ",key_id_tag=";


        m_gGzipOs << m_strVecvalues[1].c_str() << " status=";//输出tag
        m_gGzipOs << m_strVecvalues[2].c_str() << ",value=" << m_strVecvalues[3].c_str() << " ";         //输出field
        m_gGzipOs << m_strVecvalues[0].c_str() <<"\n";//输出time

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

bool CChunkedJsonHandler::Key(const char *str, SizeType length, bool copy) {

    Q_UNUSED(copy);
    Q_UNUSED(length);
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

bool CChunkedJsonHandler::Null() {

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

bool CChunkedJsonHandler::Bool(bool b) {

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

bool CChunkedJsonHandler::String(const char *str, SizeType length, bool copy) {

    Q_UNUSED(copy);
    Q_UNUSED(length);
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

bool CChunkedJsonHandler::RawNumber(const Ch *str, SizeType length, bool copy){
    Q_UNUSED(copy);
    Q_UNUSED(length);
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

void CChunkedJsonHandler::genDMLHeader(CChunkedJsonHandler::CGZipOStream &os)
{

    //        linux header
    os << "# DML\n" << "# CONTEXT-DATABASE:iscs6000\n" << "# CONTEXT-RETENTION-POLICY:autogen\n" << "# writing tsm data\n";
    //    std::cout <<"# DML\n"
    //                "# CONTEXT-DATABASE:iscs60000\n"
    //                "# CONTEXT-RETENTION-POLICY:autogen\n"
    //                "# writing tsm data" << std::endl;

    //    mac header
    //    std::cout <<" # DML\n"
    //             "# CONTEXT-DATABASE:NOAA_water_database\n"
    //             "# CONTEXT-RETENTION-POLICY:autogen\n"
    //             "# writing tsm data" << std::endl;

}

std::string CChunkedJsonHandler::stateToString(CChunkedJsonHandler::State state)
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

CChunkedJsonHandler::CGZipOStream::operator bool() const
{
    return m_bOkState;
}

CChunkedJsonHandler::CGZipOStream::CGZipOStream(QString filePath_):
    m_destFile(filePath_)
  , m_nBufSize(1 << 16) //65536
  , m_nInLen(0)
{
    m_bOkState = m_destFile.open(QIODevice::WriteOnly);

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

        m_bOkState = false;
        return ;
    }

    //< 分配缓冲区
    m_inBuf = (char*)malloc(m_nBufSize * sizeof(char));
    m_outBuf =(char*)malloc(m_nBufSize * sizeof(char));
}

CChunkedJsonHandler::CGZipOStream::~CGZipOStream()
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
