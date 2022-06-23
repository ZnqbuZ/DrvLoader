#include "binded_str.h"

using namespace DrvLoader;

DrvLoader::binded_str::binded_str()
{
    data = nullptr;
};

binded_str::~binded_str()
{
    if (data != nullptr)
    {
        free(data);
        data = nullptr;
    }
}

binded_str::operator String ^ () { return data_str; }
binded_str::operator PWSTR() { return data; }

binded_str^ binded_str::operator=(String^ source)
{
    data_str = source;
    this->~binded_str();
    const pin_ptr<const WCHAR> s = PtrToStringChars(source);
    const size_t len = wcslen(s) + 1;
    data = static_cast<PWSTR>(malloc(len * sizeof(WCHAR)));
    if (data != nullptr)
    {
        wcscpy_s(data, len, s);
    }
    return this;
}

size_t binded_str::Length::get()
{
    if (data != nullptr)
    {
        return wcslen(data);
    }
    return 0;
}
