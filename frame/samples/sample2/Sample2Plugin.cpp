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

#include "Sample2Plugin.h"
#include "Sample2Module.h"

namespace ark
{

    ARK_DLL_PLUGIN_ENTRY(Sample2Plugin)
    ARK_DLL_PLUGIN_EXIT(Sample2Plugin)

    //////////////////////////////////////////////////////////////////////////

    int Sample2Plugin::GetPluginVersion()
    {
        return 0;
    }

    const std::string Sample2Plugin::GetPluginName()
    {
        return GET_CLASS_NAME(Sample2Plugin)
    }

    void Sample2Plugin::Install()
    {
        RegisterModule<Sample2Module, Sample2Module>();
    }

    void Sample2Plugin::Uninstall()
    {
        DeregisterModule<Sample2Module, Sample2Module>();
    }

}