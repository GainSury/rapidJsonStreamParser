

#include <cstdio>
#include <iostream>

#include <QtCore>

#include "curlreadstream.h"
#include "CChunkedJsonHandler.h"
#include <QDebug>


using namespace rapidjson;
using namespace std;



int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);



    using namespace rapidjson;

    //for linux test
    const char *url =  "http://127.0.0.1:8086/query?db=iscs6000&chunked=true&chunk_size=2&epoch=ms&q=select%20%2A%20from%20ai_sample_result%20limit%201000";

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

    QElapsedTimer t;
    t.start();

    UrlReadStream stream(handle);

    Reader reader;

    {
        CChunkedJsonHandler handler(QString("/home/logan/Desktop/export.txt.gz"));


        while(1)
        {
            if(reader.Parse<kParseNumbersAsStringsFlag>(stream,handler))
                break;
        }
    }
    if (reader.HasParseError()) {
        std::cout << "Error at offset " << reader.GetErrorOffset() << ": " << GetParseError_En(reader.GetParseErrorCode()) << std::endl;
        return EXIT_FAILURE;
    }

    int nStatusCode = 0;

    curl_easy_getinfo(handle->handle.curl,CURLINFO_RESPONSE_CODE,&nStatusCode);


    qDebug()<<"ttt:"<<t.elapsed();

    std::cout << "status code: " << nStatusCode  << "time:" << t.elapsed()/1000.0<< std::endl;
     url_fclose(handle);

}

