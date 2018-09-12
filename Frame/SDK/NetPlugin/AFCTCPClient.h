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
#include <brynet/net/WrapTCPService.h>
#include <brynet/net/Connector.h>

#pragma pack(push, 1)

class AFCTCPClient : public AFINet
{
public:
    AFCTCPClient(const brynet::net::WrapTcpService::PTR& server = nullptr, const brynet::net::AsyncConnector::PTR& connector = nullptr);

    template<typename BaseType>
    AFCTCPClient(BaseType* pBaseType, void (BaseType::*handleRecieve)(const AFIMsgHead& xHead, const int, const char*, const size_t, const AFGUID&), void (BaseType::*handleEvent)(const NetEventType, const AFGUID&, const int))
    {
        mRecvCB = std::bind(handleRecieve, pBaseType, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        mEventCB = std::bind(handleEvent, pBaseType, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        mnTargetBusID = 0;
        nRecvSize = 0;
        nSendSize = 0;
        m_pTCPService = std::make_shared<brynet::net::WrapTcpService>();
        m_pConector = brynet::net::AsyncConnector::Create();
    }

    ~AFCTCPClient() override;

    void Update() override;

    bool Start(const int target_busid, const std::string& ip, const int port, bool ip_v6 = false) override;
    bool Start(const int busid, const std::string& ip, const int port, const int thread_num, const unsigned int max_client, bool ip_v6 = false) override
    {
        return false;
    }

    bool Shutdown() override final;
    bool SendMsgWithOutHead(const uint16_t nMsgID, const char* msg, const size_t nLen, const AFGUID& xClientID = 0, const AFGUID& xPlayerID = 0) override;

    bool CloseNetEntity(const AFGUID& xClient) override;
    bool IsServer() override;
    bool Log(int severity, const char* msg) override;

protected:
    void OnClientConnectionInner(const brynet::net::TCPSession::PTR& session);
    void OnClientDisConnectionInner(const brynet::net::TCPSession::PTR& session);
    size_t OnMessageInner(const brynet::net::TCPSession::PTR& session, const char* buffer, size_t len);

    bool SendMsg(const char* msg, const size_t nLen, const AFGUID& xClient = 0);

    bool DismantleNet(AFTCPEntity* pEntity);
    void ProcessMsgLogicThread();
    void ProcessMsgLogicThread(AFTCPEntity* pEntity);
    bool CloseSocketAll();

    int DeCode(const char* strData, const size_t len, AFCMsgHead& xHead);
    int EnCode(const AFCMsgHead& xHead, const char* strData, const size_t len, std::string& strOutData);

private:
    std::unique_ptr<AFTCPEntity> m_pClientEntity{ nullptr };
    std::string mstrIPPort{};
    int mnTargetBusID{ 0 };
    std::atomic<uint64_t> mnNextID{ 0 };

    NET_RECV_FUNCTOR mRecvCB;
    NET_EVENT_FUNCTOR mEventCB;
    AFCReaderWriterLock mRWLock;

    brynet::net::WrapTcpService::PTR m_pTCPService{ nullptr };
    brynet::net::AsyncConnector::PTR m_pConector{ nullptr };
};

#pragma pack(pop)