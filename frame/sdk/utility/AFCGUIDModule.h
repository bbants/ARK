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

#pragma once

#include "base/AFUidGenerator.hpp"
#include "interface/AFIGUIDModule.h"

namespace ark
{

    class AFCGUIDModule : public AFIGUIDModule
    {
    public:
        explicit AFCGUIDModule() = default;

        bool Init() override;
        bool PreShut() override;

        AFGUID CreateGUID() override;
        std::string ParseUID(const AFGUID& id) override;

    private:
#ifdef AF_THREAD_SAFE
        AFUidGeneratorThreadSafe* uid_generator_ { nullptr };
#else
        AFUidGenerator* uid_generator_ { nullptr };
#endif // AF_THREAD_SAFE
    };

}