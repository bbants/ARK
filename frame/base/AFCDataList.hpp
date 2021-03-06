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

#include "interface/AFIData.hpp"
#include "interface/AFIDataList.hpp"
#include "AFMisc.hpp"

namespace ark
{

    class AFDataListAlloc
    {
    public:
        AFDataListAlloc() = default;
        ~AFDataListAlloc() = default;

        void* Alloc(size_t size)
        {
            ARK_NEW_ARRAY_RET(char, size);
        }

        void Free(void* ptr, size_t size)
        {
            ARK_DELETE_ARRAY(char, ptr);
        }

        void Swap(AFDataListAlloc& src)
        {
            //Do nothing
        }
    };

    template<size_t DATA_SIZE, size_t BUFFER_SIZE, typename ALLOC = AFDataListAlloc>
    class AFBaseDataList : public AFIDataList
    {
    private:
        using self_t = AFBaseDataList<DATA_SIZE, BUFFER_SIZE, ALLOC>;

        struct dynamic_data_t
        {
            int nType;
            union
            {
                bool mbValue;
                int mnValue;
                int64_t mn64Value;
                float mfValue;
                double mdValue;
                size_t mnstrValue;
                void* mpVaule;
                size_t mnUserData;
                AFGUID mxGUID;
            };
        };

    public:
        AFBaseDataList()
        {
            assert(DATA_SIZE > 0);
            assert(BUFFER_SIZE > 0);

            mpData = mDataStack;
            mnDataSize = DATA_SIZE;
            mnDataUsed = 0;

            mpBuffer = mBufferStack;
            mnBufferSize = BUFFER_SIZE;
            mnBufferUsed = 0;
        }

        AFBaseDataList(const char* strSour, int nLengh, char strSplit)
        {
            assert(DATA_SIZE > 0);
            assert(BUFFER_SIZE > 0);

            mpData = mDataStack;
            mnDataSize = DATA_SIZE;
            mnDataUsed = 0;

            mpBuffer = mBufferStack;
            mnBufferSize = BUFFER_SIZE;
            mnBufferUsed = 0;

            int nBegin = 0;
            int nEnd = 0;

            for (int i = 0; i < nLengh; i++)
            {
                if (strSour[i] == strSplit)
                {
                    nEnd = i;

                    if (!AddString(&strSour[nBegin], nEnd - nBegin))
                    {
                        ARK_ASSERT_NO_EFFECT(0);
                    }

                    if (i + 1 < nLengh)
                    {
                        nBegin = i + 1;
                    }
                }
            }

            if (nEnd < nLengh)
            {
                nEnd = nLengh;

                if (!AddString(&strSour[nBegin], nEnd - nBegin))
                {
                    ARK_ASSERT_NO_EFFECT(0);
                }
            }
        }

        AFBaseDataList(const self_t& src)
        {
            assert(DATA_SIZE > 0);
            assert(BUFFER_SIZE > 0);

            mpData = mDataStack;
            mnDataSize = DATA_SIZE;
            mnDataUsed = 0;

            mpBuffer = mBufferStack;
            mnBufferSize = BUFFER_SIZE;
            mnBufferUsed = 0;
            InnerAppend(src, 0, src.GetCount());
        }

        AFBaseDataList(const AFIDataList& src)
        {
            assert(DATA_SIZE > 0);
            assert(BUFFER_SIZE > 0);

            mpData = mDataStack;
            mnDataSize = DATA_SIZE;
            mnDataUsed = 0;

            mpBuffer = mBufferStack;
            mnBufferSize = BUFFER_SIZE;
            mnBufferUsed = 0;
            InnerAppend(src, 0, src.GetCount());
        }

        virtual ~AFBaseDataList()
        {
            Release();
        }

        self_t operator=(const self_t& src)
        {
            Release();

            mpData = mDataStack;
            mnDataSize = DATA_SIZE;
            mnDataUsed = 0;

            mpBuffer = mBufferStack;
            mnBufferSize = BUFFER_SIZE;
            mnBufferUsed = 0;
            InnerAppend(src, 0, src.GetCount());

            return *this;
        }

        void Release()
        {
            if (mnDataSize > DATA_SIZE)
            {
                mxAlloc.Free(mpData, mnDataSize * sizeof(dynamic_data_t));
            }

            if (mnBufferSize > BUFFER_SIZE)
            {
                mxAlloc.Free(mpBuffer, mnBufferSize * sizeof(char));
            }
        }

        bool Concat(const AFIDataList& src) override
        {
            InnerAppend(src, 0, src.GetCount());
            return true;
        }

        bool Split(const std::string& src, const std::string& split) override
        {
            Clear();

            std::string tmp(src);
            if (tmp.empty())
            {
                return true;
            }

            std::string tmp_split(split);
            std::string::size_type pos;
            tmp += tmp_split;
            std::string::size_type size = tmp.length();

            for (std::string::size_type i = 0; i < size; ++i)
            {
                pos = int(tmp.find(tmp_split, i));
                if (pos < size)
                {
                    std::string sub = tmp.substr(i, pos - i);
                    AddString(sub.c_str());

                    i = pos + tmp_split.size() - 1;
                }
            }

            return true;
        }

        bool Append(const AFIData& data) override
        {
            InnerAppend(data);
            return true;
        }

        bool Append(const AFIDataList& src, size_t start, size_t count) override
        {
            if (start >= src.GetCount())
            {
                return false;
            }

            size_t end = start + count;

            if (end > src.GetCount())
            {
                return false;
            }

            InnerAppend(src, start, end);
            return true;
        }

        void Clear() override
        {
            mnDataUsed = 0;
            mnBufferUsed = 0;
        }

        bool Empty() const override
        {
            return (0 == mnDataUsed);
        }

        size_t GetCount() const override
        {
            return mnDataUsed;
        }

        int GetType(size_t index) const override
        {
            if (index >= mnDataUsed)
            {
                return DT_UNKNOWN;
            }

            return mpData[index].nType;
        }

        //add data
        bool AddBool(bool value) override
        {
            dynamic_data_t* p = AddDynamicData();
            p->nType = DT_BOOLEAN;
            p->mbValue = value;
            return true;
        }

        bool AddInt(int value) override
        {
            dynamic_data_t* p = AddDynamicData();
            p->nType = DT_INT;
            p->mnValue = value;
            return true;
        }

        bool AddInt64(int64_t value) override
        {
            dynamic_data_t* p = AddDynamicData();
            p->nType = DT_INT64;
            p->mn64Value = value;
            return true;
        }

        bool AddFloat(float value) override
        {
            dynamic_data_t* p = AddDynamicData();
            p->nType = DT_FLOAT;
            p->mfValue = value;
            return true;
        }

        bool AddDouble(double value) override
        {
            dynamic_data_t* p = AddDynamicData();
            p->nType = DT_DOUBLE;
            p->mdValue = value;
            return true;
        }

        bool AddString(const char* value) override
        {
            assert(value != nullptr);
            dynamic_data_t* p = AddDynamicData();
            p->nType = DT_STRING;
            p->mnstrValue = mnBufferUsed;

            const size_t value_size = strlen(value) + 1;
            char* data = AddBuffer(value_size);
            memcpy(data, value, value_size);

            return true;
        }

        virtual bool AddString(const char* value, const int nLength)
        {
            if (nLength <= 0)
            {
                return false;
            }

            assert(value != nullptr);
            dynamic_data_t* p = AddDynamicData();

            if (nullptr == p)
            {
                return false;
            }

            p->nType = DT_STRING;
            p->mnstrValue = mnBufferUsed;

            size_t value_size = strlen(value);

            if (value_size > (size_t)nLength)
            {
                value_size = (size_t)nLength;
            }

            value_size += 1;
            char* data = AddBuffer(value_size);
            memcpy(data, value, value_size);

            return true;
        }

        bool AddPointer(void* value) override
        {
            dynamic_data_t* p = AddDynamicData();
            p->nType = DT_POINTER;
            p->mpVaule = value;
            return true;
        }

        bool AddUserData(const void* pData, size_t size) override
        {
            assert(pData != nullptr);

            dynamic_data_t* p = AddDynamicData();
            p->nType = DT_USERDATA;
            p->mnUserData = mnBufferUsed;

            const size_t value_size = AFIData::GetRawUserDataSize(size);
            char* value = AddBuffer(value_size);
            AFIData::InitRawUserData(value, pData, size);

            return true;
        }

        bool AddRawUserData(void* value) override
        {
            return AddUserData(AFIData::GetUserData(value), AFIData::GetUserDataSize(value));
        }

        //get data
        bool Bool(size_t index) const override
        {
            if (index > mnDataUsed)
            {
                return NULL_BOOLEAN;
            }

            if (mpData[index].nType == DT_BOOLEAN)
            {
                return mpData[index].mbValue;
            }
            else
            {
                return NULL_BOOLEAN;
            }
        }

        int Int(size_t index) const override
        {
            if (index > mnDataUsed)
            {
                return NULL_INT;
            }

            if (mpData[index].nType == DT_INT)
            {
                return mpData[index].mnValue;
            }
            else
            {
                return NULL_INT;
            }
        }

        int64_t Int64(size_t index) const override
        {
            if (index > mnDataUsed)
            {
                return NULL_INT64;
            }

            if (mpData[index].nType == DT_INT64)
            {
                return mpData[index].mn64Value;
            }
            else
            {
                return NULL_INT64;
            }
        }

        float Float(size_t index) const override
        {
            if (index > mnDataUsed)
            {
                return NULL_FLOAT;
            }

            if (mpData[index].nType == DT_FLOAT)
            {
                return mpData[index].mfValue;
            }
            else
            {
                return NULL_FLOAT;
            }
        }

        double Double(size_t index) const override
        {
            if (index > mnDataUsed)
            {
                return NULL_DOUBLE;
            }

            if (mpData[index].nType == DT_DOUBLE)
            {
                return mpData[index].mdValue;
            }
            else
            {
                return NULL_DOUBLE;
            }
        }

        const char* String(size_t index) const override
        {
            if (index > mnDataUsed)
            {
                return NULL_STR.c_str();
            }

            if (mpData[index].nType == DT_STRING)
            {
                return mpBuffer + mpData[index].mnstrValue;
            }
            else
            {
                return NULL_STR.c_str();
            }
        }

        void* Pointer(size_t index) const override
        {
            if (index > mnDataUsed)
            {
                return nullptr;
            }

            if (mpData[index].nType == DT_POINTER)
            {
                return mpData[index].mpVaule;
            }
            else
            {
                return nullptr;
            }
        }

        const void* UserData(size_t index, size_t& size) const override
        {
            if (index > mnDataUsed)
            {
                size = 0;
                return nullptr;
            }

            if (mpData[index].nType == DT_USERDATA)
            {
                char* p = mpBuffer + mpData[index].mnUserData;
                size = AFIData::GetUserDataSize(p);
                return AFIData::GetUserData(p);
            }
            else
            {
                size = 0;
                return nullptr;
            }
        }

        void* RawUserData(size_t index) const override
        {
            if (index > mnDataUsed)
            {
                return nullptr;
            }

            if (mpData[index].nType == DT_USERDATA)
            {
                return mpBuffer + mpData[index].mnUserData;
            }
            else
            {
                return nullptr;
            }
        }

        virtual bool SetBool(size_t index, bool value)
        {
            if (index >= mnDataUsed)
            {
                return false;
            }

            if (mpData[index].nType != DT_BOOLEAN)
            {
                return false;
            }
            else
            {
                mpData[index].mbValue = value;
                return true;
            }
        }

        virtual bool SetInt(size_t index, int value)
        {
            if (index >= mnDataUsed)
            {
                return false;
            }

            if (mpData[index].nType != DT_INT)
            {
                return false;
            }
            else
            {
                mpData[index].mnValue = value;
                return true;
            }
        }

        virtual bool SetInt64(size_t index, int64_t value)
        {
            if (index >= mnDataUsed)
            {
                return false;
            }

            if (mpData[index].nType != DT_INT64)
            {
                return false;
            }
            else
            {
                mpData[index].mn64Value = value;
                return true;
            }
        }

        virtual bool SetFloat(size_t index, float value)
        {
            if (index >= mnDataUsed)
            {
                return false;
            }

            if (mpData[index].nType != DT_FLOAT)
            {
                return false;
            }
            else
            {
                mpData[index].mfValue = value;
                return true;
            }
        }

        virtual bool SetDouble(size_t index, double value)
        {
            if (index >= mnDataUsed)
            {
                return false;
            }

            if (mpData[index].nType != DT_DOUBLE)
            {
                return false;
            }
            else
            {
                mpData[index].mdValue = value;
                return true;
            }
        }

        virtual bool SetString(size_t index, const char* value)
        {
            assert(value != nullptr);

            if (index >= mnDataUsed)
            {
                return false;
            }

            if (mpData[index].nType != DT_STRING)
            {
                return false;
            }

            char* p = mpBuffer + mpData[index].mnstrValue;
            const size_t size1 = strlen(value) + 1;

            if (size1 <= (strlen(p) + 1))
            {
                ARK_STRNCPY(p, value, size1);
                return true;
            }

            mpData[index].mnstrValue = mnDataUsed;
            const size_t value_size = strlen(value) + 1;
            char* v = AddBuffer(value_size);
            memcpy(v, value, value_size);

            return true;
        }

        virtual bool SetPointer(size_t index, void* value)
        {
            if (index >= mnDataUsed)
            {
                return false;
            }

            if (mpData[index].nType != DT_POINTER)
            {
                return false;
            }
            else
            {
                mpData[index].mpVaule = value;
                return true;
            }
        }

        const std::string ToString(size_t index) override
        {
            if (index > mnDataUsed)
            {
                return NULL_STR;
            }

            std::string data;
            switch (mpData[index].nType)
            {
            case DT_BOOLEAN:
                data = ARK_TO_STRING(mpData[index].mbValue);
                break;
            case DT_INT:
                data = ARK_TO_STRING(mpData[index].mnValue);
                break;
            case DT_INT64:
                data = ARK_TO_STRING(mpData[index].mn64Value);
                break;
            case DT_FLOAT:
                data = ARK_TO_STRING(mpData[index].mfValue);
                break;
            case DT_DOUBLE:
                data = ARK_TO_STRING(mpData[index].mdValue);
                break;
            case DT_STRING:
                data = String(index);
                break;
            case DT_POINTER:
                break;
            case DT_USERDATA:
                break;
            default:
                assert(0);
                break;
            }

            return data;
        }

        size_t GetMemUsage() const override
        {
            size_t size = sizeof(self_t);

            if (mnDataSize > DATA_SIZE)
            {
                size += sizeof(dynamic_data_t) * mnDataSize;
            }

            if (mnBufferSize > BUFFER_SIZE)
            {
                size += sizeof(char) * mnBufferSize;
            }

            return size;
        }

    protected:
        dynamic_data_t* AddDynamicData()
        {
            if (mnDataUsed >= mnDataSize)
            {
                size_t new_size = mnDataSize * 2;
                dynamic_data_t* p = (dynamic_data_t*)mxAlloc.Alloc(new_size * sizeof(dynamic_data_t));
                memcpy(p, mpData, mnDataUsed * sizeof(dynamic_data_t));

                if (mnDataSize > DATA_SIZE)
                {
                    mxAlloc.Free(mpData, mnDataSize * sizeof(dynamic_data_t));
                }

                mpData = p;
                mnDataSize = new_size;
            }

            return mpData + mnDataUsed++;
        }

        char* AddBuffer(size_t need_size)
        {
            size_t new_used = mnBufferUsed + need_size;

            if (new_used > mnBufferSize)
            {
                size_t new_size = mnBufferSize * 2;

                if (new_used > new_size)
                {
                    new_size = new_used * 2;
                }

                char* p = (char*)mxAlloc.Alloc(new_size);
                memcpy(p, mpBuffer, mnBufferUsed);

                if (mnBufferSize > BUFFER_SIZE)
                {
                    mxAlloc.Free(mpBuffer, mnBufferSize);
                }

                mpBuffer = p;
                mnBufferSize = new_size;
            }

            char* ret = mpBuffer + mnBufferUsed;
            mnBufferUsed = new_used;
            return ret;
        }

        void InnerAppend(const AFIData& data)
        {
            bool bRet(false);

            switch (data.GetType())
            {
            case DT_BOOLEAN:
                bRet = AddBool(data.GetBool());
                break;
            case DT_INT:
                bRet = AddInt(data.GetInt());
                break;
            case DT_INT64:
                bRet = AddInt64(data.GetInt64());
                break;
            case DT_FLOAT:
                bRet = AddFloat(data.GetFloat());
                break;
            case DT_DOUBLE:
                bRet = AddDouble(data.GetDouble());
                break;
            case DT_STRING:
                bRet = AddString(data.GetString());
                break;
            case DT_POINTER:
                bRet = AddPointer(data.GetPointer());
                break;
            case DT_USERDATA:
                {
                    size_t size;
                    const void* pData = data.GetUserData(size);
                    bRet = AddUserData(pData, size);
                }
                break;
            default:
                ARK_ASSERT_NO_EFFECT(0);
                break;
            }

            ARK_ASSERT_NO_EFFECT(bRet);
        }

        bool InnerAppend(const AFIDataList& src, size_t start, size_t end)
        {
            bool bRet(false);

            for (size_t i = start; i < end; ++i)
            {
                switch (src.GetType(i))
                {
                case DT_BOOLEAN:
                    bRet = AddBool(src.Bool(i));
                    break;
                case DT_INT:
                    bRet = AddInt(src.Int(i));
                    break;
                case DT_INT64:
                    bRet = AddInt64(src.Int64(i));
                    break;
                case DT_FLOAT:
                    bRet = AddFloat(src.Float(i));
                    break;
                case DT_DOUBLE:
                    bRet = AddDouble(src.Double(i));
                    break;
                case DT_STRING:
                    bRet = AddString(src.String(i));
                    break;
                case DT_POINTER:
                    bRet = AddPointer(src.Pointer(i));
                    break;
                case DT_USERDATA:
                    {
                        size_t size;
                        const void* pData = src.UserData(i, size);
                        bRet = AddUserData(pData, size);
                    }
                    break;
                default:
                    ARK_ASSERT_NO_EFFECT(0);
                    break;
                }
            }

            return bRet;
        }

    private:
        ALLOC mxAlloc;
        dynamic_data_t mDataStack[DATA_SIZE];
        dynamic_data_t* mpData;
        size_t mnDataSize;
        size_t mnDataUsed;
        char mBufferStack[BUFFER_SIZE];
        char* mpBuffer;
        size_t mnBufferSize;
        size_t mnBufferUsed;
    };

    using AFCDataList = AFBaseDataList<8, 128>;

}