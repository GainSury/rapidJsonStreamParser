

#include <cstdio>
#include <iostream>

#include <QtCore>

#include "curlreadstream.h"
#include "CChunkedJsonHandler.h"

#include <qcompressor.h>



using namespace rapidjson;
using namespace std;





void genDMLHeader(CGZipOStream& os)
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



int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    QTime t;
    t.start();

    using namespace rapidjson;

    //for linux test
    const char *url =  "http://127.0.0.1:8086/query?db=iscs6000&chunked=true&chunk_size=2&epoch=ms&q=select%20%2A%20from%20ai_sample_result%20limit%201000000";

    //for linux test generate line protocol
//    const char *url =    "http://127.0.0.1:8086/query?db=iscs6000&chunked=true&pretty=true&chunk_size=2&epoch=ms&q=SELECT%20key_id_tag%2Cstatus%2Cvalue%2Ctime%20FROM%20ai_sample_result%20limit%2010";


    //for mac test
//    const char *url = "http://127.0.0.1:8086/query?db=NOAA_water_database&chunked=true&chunk_size=20&epoch=ms&q=select%20%2A%20from%20h2o_pH";

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

    CGZipOStream os(QString("/home/logan/Desktop/export.txt.gz"));

    if(!os)
    {
        return 1;
    }
    CChunkedJsonHandler handler(os);
    genDMLHeader(os);

    while(1)
    {
        if(reader.Parse<kParseNumbersAsStringsFlag>(stream,handler))
            break;
    }

    if (reader.HasParseError()) {
        std::cout << "Error at offset " << reader.GetErrorOffset() << ": " << GetParseError_En(reader.GetParseErrorCode()) << std::endl;
        return EXIT_FAILURE;
    }

    int nStatusCode = 0;
    curl_easy_getinfo(handle->handle.curl,CURLINFO_RESPONSE_CODE,&nStatusCode);

    std::cout << "status code: " << nStatusCode  << std::endl;
     url_fclose(handle);
    std::cout << "count: " << handler.nIndex  << "耗时: " << t.elapsed()/1000.0 << "s"<< std::endl;
}

