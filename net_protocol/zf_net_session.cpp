#include <unistd.h>
#include "zf_net_session.h"
#include "protocol_parser.h"
#include "protocol_utils.h"
#include "data_source.h"
zf_tcp_session::zf_tcp_session(int sock):tcp_session(sock)
{

}

int  zf_tcp_session::handle_recv_data()
{
    char buf[1024] ={0};
    ssize_t size = 0;
    do
    {
        size = ::read(m_sockfd, buf, 1024);
        if(size > 0)
        {
            m_data.append(buf, size);
        }

    }while(size > 0 && size == 1024);



    Message msg;
    extract_msg(m_data, msg);

    switch(msg.command)
    {
    case CT_GetUserInfoRequest:
    {
        GetUserInfoRequest request;
        get_request(request, msg.data);
        GetUserInfoResponse response;
        process_request(request, response);

        //reuse msg object
        msg.clear();
        msg.command = CT_GetUserInfoResponse;
        //response object to bytes
        make_response(response, msg.data);
        //msg object to package(bytes)
        string data;
        msg_to_package(msg,data);
        //send bytes to client
        ::write(m_sockfd, data.data(), data.length());

    }
        break;
    default:
        break;
    }


    return 0;
}
