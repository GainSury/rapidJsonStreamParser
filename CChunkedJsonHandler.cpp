#include "CChunkedJsonHandler.h"



CChunkedJsonHandler::CChunkedJsonHandler(CGZipOStream &gGzipOs):
    state_(kExpectObjectStart),
    nIndex(0),
    m_gGzipOs(gGzipOs)
{
    //< todo to initialize


}

bool CChunkedJsonHandler::StartObject() {
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

bool CChunkedJsonHandler::EndObject(SizeType memberCount) {
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

bool CChunkedJsonHandler::StartArray() {
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

bool CChunkedJsonHandler::EndArray(SizeType elementCount) {
    //        cout << "before EndArray():"  << stateToString(state_) << endl;

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

bool CChunkedJsonHandler::Null() {
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

bool CChunkedJsonHandler::Bool(bool b) {
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

bool CChunkedJsonHandler::String(const char *str, SizeType length, bool copy) {
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

bool CChunkedJsonHandler::RawNumber(const Ch *str, SizeType length, bool copy){
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
