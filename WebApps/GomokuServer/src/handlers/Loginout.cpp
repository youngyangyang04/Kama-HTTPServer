#include "../include/handlers/LogoutHandler.h"

void LogoutHandler::handle(const HttpRequest &req, HttpResponse *resp)
{
    auto contentType = req.getHeader("Content-Type");
    if (contentType.empty() || contentType != "application/json" || req.getBody().empty())
    {
        std::cout << "资源释放位置不对，请求体：" << req.getBody() << std::endl; // fixme：这里可能有问题。
        resp->setStatusLine(req.getVersion(), HttpResponse::k400BadRequest, "Bad Request");
        resp->setCloseConnection(true);
        resp->setContentType("application/json");
        resp->setContentLength(0);
        resp->setBody("");
        return;
    }

    // JSON 解析使用 try catch 捕获异常
    try
    {
        // 获取会话
        auto session = server_->getSessionManager()->getSession(req, resp);
        // 获取用户id
        int userId = std::stoi(session->getValue("userId"));
        // 清除会话数据
        session->clear();
        // 销毁会话
        server_->getSessionManager()->destroySession(session->getId());
        
        json parsed = json::parse(req.getBody());
        int gameType = parsed["gameType"]; // fixme: 以后也换成从会话中获取
        // 释放资源
        server_->isLogining_.erase(userId);

        if (gameType == GomokuServer::MAN_VS_AI)
        {
            server_->aiGames_.erase(userId);
        }
        else if (gameType == GomokuServer::MAN_VS_MAN)
        {
            // 释放相应创造资源，并且通知另一个用户对方已经主动退出游戏
        }

        // 返回响应报文
        json response;
        response["message"] = "logout successful";
        std::string responseBody = response.dump(4);
        resp->setStatusLine(req.getVersion(), HttpResponse::k200Ok, "OK");
        resp->setCloseConnection(true);
        resp->setContentType("application/json");
        resp->setContentLength(responseBody.size());
        resp->setBody(responseBody);
    }
    catch (const std::exception &e)
    {
        // 捕获异常，返回错误信息
        json failureResp;
        failureResp["status"] = "error";
        failureResp["message"] = e.what();
        std::string failureBody = failureResp.dump(4);
        resp->setStatusLine(req.getVersion(), HttpResponse::k400BadRequest, "Bad Request");
        resp->setCloseConnection(true);
        resp->setContentType("application/json");
        resp->setContentLength(failureBody.size());
        resp->setBody(failureBody);
    }
}