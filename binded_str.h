#pragma once
#include "windows.h"
#include "vcclr.h"

namespace DrvLoader
{
    using namespace System;

    ref class binded_str
    {
        PWSTR data;
        String^ data_str;

    public:
        binded_str();
        ~binded_str();

        operator String ^ ();
        operator PWSTR();

        binded_str^ operator=(String^ source);

        property size_t Length
        {
            size_t get();
        };
    };
} // namespace DrvLoader
