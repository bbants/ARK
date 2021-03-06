﻿/*
* This source file is part of ARK
* For the latest info, see https://github.com/QuadHex
*
* Copyright (c) 2013-2018 QuadHex authors.
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

#include "AFCDirNetModule.h"
#include "interface/AFIPluginManager.h"

namespace ark
{

    bool AFCDirNetModule::Init()
    {
        m_pLogModule = pPluginManager->FindModule<AFILogModule>();
        m_pBusModule = pPluginManager->FindModule<AFIBusModule>();
        m_pNetServiceManagerModule = pPluginManager->FindModule<AFINetServiceManagerModule>();

        return true;
    }

    bool AFCDirNetModule::PostInit()
    {
        int ret = StartServer();
        if (ret != 0)
        {
            exit(0);
            return false;
        }

        return true;
    }

    int AFCDirNetModule::StartServer()
    {
        int ret = m_pNetServiceManagerModule->CreateServer();
        if (ret != 0)
        {
            ARK_LOG_ERROR("Cannot start server net, busid = {}, error = {}", m_pBusModule->GetSelfBusName(), ret);
            ARK_ASSERT_NO_EFFECT(0);
            return ret;
        }

        m_pNetServer = m_pNetServiceManagerModule->GetSelfNetServer();
        if (m_pNetServer == nullptr)
        {
            ret = -3;
            ARK_LOG_ERROR("Cannot find server net, busid = {}, error = {}", m_pBusModule->GetSelfBusName(), ret);
            return ret;
        }

        return 0;
    }

    bool AFCDirNetModule::PreUpdate()
    {
        int ret = StartClient();
        return (ret == 0);
    }

    int AFCDirNetModule::StartClient()
    {
        //创建所有与对端链接的client
        int ret = m_pNetServiceManagerModule->CreateClusterClients();
        if (ret != 0)
        {
            ARK_LOG_ERROR("Cannot start server net, busid = {}, error = {}", m_pBusModule->GetSelfBusName(), ret);
            ARK_ASSERT_NO_EFFECT(0);
            return ret;
        }

        AFINetClientService* pNetClientMaster = m_pNetServiceManagerModule->GetNetClientService(ARK_APP_TYPE::ARK_APP_MASTER);
        if (pNetClientMaster == nullptr)
        {
            return -1;
        }

        return 0;
    }

    AFINetServerService* AFCDirNetModule::GetNetServer()
    {
        return m_pNetServer;
    }

}