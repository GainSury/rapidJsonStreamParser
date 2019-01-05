#ifndef CCHUNKEDJSONHANDLER_H
#define CCHUNKEDJSONHANDLER_H


struct CChunkedJsonHandler : public BaseReaderHandler<UTF8<>, MyHandler> {
    CChunkedJsonHandler();
    bool StartObject();
    bool EndObject(SizeType memberCount);
    bool StartArray();
    bool EndArray(SizeType elementCount);


    bool Key(const char* str, SizeType length, bool copy);

    bool Null();
    bool Bool(bool b);
    bool String(const char* str, SizeType length, bool copy);
    bool RawNumber(const Ch* str, SizeType length, bool copy);


    std::string m_strCurMeasurementName;
    std::vector<std::string> m_strVecvalues;
    size_t nIndex;

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

   static std::string stateToString(State state);
};

#endif // CCHUNKEDJSONHANDLER_H
