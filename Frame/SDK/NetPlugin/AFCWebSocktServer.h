﻿/*
* This source file is part of ArkGameFrame
* For the latest info, see https://github.com/ArkGame
*
* Copyright (c) 2013-2018 ArkGame authors.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#pragma once

#include "SDK/Interface/AFINet.h"
#include "SDK/Core/AFQueue.hpp"
#include "SDK/Core/AFRWLock.hpp"
#include <brynet/net/SocketLibFunction.h>
#include <brynet/net/EventLoop.h>
#include <brynet/net/WrapTCPService.h>
#include <brynet/net/ListenThread.h>
#include <brynet/net/Socket.h>
#include <brynet/net/http/HttpService.h>
#include <brynet/net/http/HttpFormat.h>

#pragma pack(push, 1)

class AFCWebSocktServer : public AFINet
{
public:
    AFCWebSocktServer();

    template<typename BaseType>
    AFCWebSocktServer(BaseType* pBaseType, void (BaseType::*handleRecieve)(const AFIMsgHead& xHead, const int, const char*, const size_t, const AFGUID&), void (BaseType::*handleEvent)(const NetEventType, const AFGUID&, const int))
        : mnMaxConnect(0)
        , mnThreadNum(0)
        , mnServerID(0)
        , nNextID(0)
    {
        mRecvCB = std::bind(handleRecieve, pBaseType, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        mEventCB = std::bind(handleEvent, pBaseType, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        bWorking = false;

        m_pTCPService = std::make_shared<brynet::net::WrapTcpService>();
        m_pListenThread = brynet::net::ListenThread::Create();
    }

    ~AFCWebSocktServer() override;

    void Update() override;

    //Just for pure virtual function
    bool Start(const int target_busid, const std::string& ip, const int port, bool ip_v6 = false) override
    {
        return false;
    }

    bool Start(const int busid, const std::string& ip, const int port, const int thread_num, const unsigned int max_client, bool ip_v6 = false) override;
    bool Shutdown() override final;
    bool IsServer() override;

    bool SendMsgWithOutHead(const uint16_t nMsgID, const char* msg, const size_t nLen, const AFGUID& xClientID, const AFGUID& xPlayerID) override;
    bool SendMsgToAllClientWithOutHead(const uint16_t nMsgID, const char* msg, const size_t nLen, const AFGUID& xPlayerID) override;

    bool CloseNetEntity(const AFGUID& xClientID) override;
    bool Log(int severity, const char* msg) override;

protected:
    //From ListenThread
    void OnAcceptConnectionInner(brynet::net::TcpSocket::PTR session);
    void OnHttpMessageCallBack(const brynet::net::HTTPParser& httpParser, const brynet::net::HttpSession::PTR& session);
    void OnWebSockMessageCallBack(const brynet::net::HttpSession::PTR& httpSession, brynet::net::WebSocketFormat::WebSocketFrameType opcode, const std::string& payload);
    void OnHttpConnect(const brynet::net::HttpSession::PTR& httpSession);
    void OnHttpDisConnection(const brynet::net::HttpSession::PTR& httpSession);

    bool SendMsgToAllClient(const char* msg, const size_t nLen);
    bool SendMsg(const char* msg, const size_t nLen, const AFGUID& xClient);
    bool AddNetEntity(const AFGUID& xClientID, AFHttpEntity* pEntity);
    bool RemoveNetEntity(const AFGUID& xClientID);
    AFHttpEntity* GetNetEntity(const AFGUID& xClientID);

    void ProcessMsgLogicThread();
    void ProcessMsgLogicThread(AFHttpEntity* pEntity);
    bool CloseSocketAll();
    bool DismantleNet(AFHttpEntity* pEntity);

    int DeCode(const char* strData, const size_t len, AFCMsgHead& xHead);
    int EnCode(const AFCMsgHead& xHead, const char* strData, const size_t len, std::string& strOutData);

private:
    std::map<AFGUID, AFHttpEntity*> mxNetEntities;
    AFCReaderWriterLock mRWLock;
    int mnMaxConnect{ 0 };
    int mnThreadNum{ 0 };
    int mnServerID{ 0 };

    NET_RECV_FUNCTOR mRecvCB{ nullptr };
    NET_EVENT_FUNCTOR mEventCB{ nullptr };

    brynet::net::WrapTcpService::PTR m_pTCPService{ nullptr };
    brynet::net::ListenThread::PTR m_pListenThread{ nullptr };
    std::atomic<std::uint64_t> nNextID{ 1 };
};

#pragma pack(pop)