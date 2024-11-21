#include "HttpContext.h"

using namespace muduo;
using namespace muduo::net;

bool HttpContext::parseRequest(Buffer* buf, Timestamp receiveTime)
{
    bool ok = true; // 解析每行请求格式是否正确
    bool hasMore = true;
    while (hasMore)
    {
        if (state_ == kExpectRequestLine)
        {
            const char* crlf = buf->findCRLF(); // 注意这个返回值边界可能有错
            if (crlf)
            {   
                ok = processRequestLine(buf->peek(), crlf);
                if (ok)
                {
                    request_.setReceiveTime(receiveTime);
                    buf->retrieveUntil(crlf + 2);
                    state_ = kExpectHeaders;
                }
                else
                {
                    hasMore = false;
                }
            }
            else 
            {
                hasMore = false;
            } 
        }
        else if (state_ == kExpectHeaders)
        {
            const char* crlf = buf->findCRLF();
            if (crlf)
            {
                const char* colon = std::find(buf->peek(), crlf, ':');
                if (colon < crlf)
                {
                    request_.addHeader(buf->peek(), colon, crlf);
                }
                else if (buf->peek() == crlf)
                {   // 空行，结束Header
                    if (!request_.getHeader("Content-Length").empty())
                    {
                        request_.setContentLength(std::stoi(request_.getHeader("Content-Length")));    
                    }
                    
                    if (request_.contentLength() == 0) 
                    {
                        state_ = kGotAll;
                        hasMore = false;
                    }
                    else // 有请求体
                    {
                        state_ = kExpectBody;
                    }
                }
                else 
                {
                    ok = false; // Header行格式错误
                    hasMore = false;
                }
                buf->retrieveUntil(crlf + 2); // 开始读指针指向下一行数据
            }
            else
            {
                hasMore = false;
            }
        }
        else if (state_ == kExpectBody)
        {
            request_.setBody(buf->peek(), buf->peek() + request_.contentLength());
            state_ = kGotAll;
            hasMore = false;
            buf->retrieve(request_.contentLength());
            
        }
    }
    return ok;
}

// 这个就是把报文解析出来将关键信息封装到HttpRequest对象里面去
bool HttpContext::processRequestLine(const char* begin, const char* end)
{
    bool succeed = false;
    const char* start = begin;
    const char* space = std::find(start, end, ' ');
    if (space != end && request_.setMethod(start, space))
    {
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end)
        {
            const char* argumentStart = std::find(start, space, '?');
            if (argumentStart != space) // 请求带参数
            {
                request_.setPath(start, argumentStart); // 注意这些返回值边界
                request_.setArgument(argumentStart, space); 
            }
            else // 请求不带参数
            {
                request_.setPath(start, space);
            }

            start = space + 1;
            succeed = ((end - start == 8) && std::equal(start, end - 1, "HTTP/1."));
            if (succeed)
            {
                if (*(end - 1) == '1')
                {
                    request_.setVersion("HTTP/1.1");
                }
                else if (*(end - 1) == '0')
                {
                    request_.setVersion("HTTP/1.0");
                }
                else 
                {
                    succeed = false;
                }
            }
        }
    }
    return succeed;
}