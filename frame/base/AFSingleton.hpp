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

#include "AFPlatform.hpp"
#include "AFSpinLock.hpp"

namespace ark
{

    template<typename T>
    class AFSingleton
    {
    public:
        static T* get()
        {
            std::call_once(once_, &AFSingleton<T>::Init);
            return instance_;
        }

        static void ShutDown()
        {
            delete instance_;
            instance_ = nullptr;
        }
    private:
        static void Init()
        {
            instance_ = new T();
        }

        static std::once_flag once_;
        static T* instance_;
    };

    template<typename T>
    std::once_flag AFSingleton<T>::once_;

    template<typename T>
    T* AFSingleton<T>::instance_ = nullptr;

}