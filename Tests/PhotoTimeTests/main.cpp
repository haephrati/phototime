#include <cstdio>
#include <cstdlib>
#include "EXIF.H"

static void Fail(const char* msg)
{
    std::printf("FAIL %s\n", msg);
    std::exit(1);
}

int main()
{
    if (M_SOI != 0xD8)
    {
        Fail("M_SOI");
    }
    if (M_EOI != 0xD9)
    {
        Fail("M_EOI");
    }
    if (M_EXIF != 0xE1)
    {
        Fail("M_EXIF");
    }
    if (MAX_SECTIONS != 20)
    {
        Fail("MAX_SECTIONS");
    }
    if (EXIF_READ_ALL != 0x03)
    {
        Fail("EXIF_READ_ALL");
    }
    std::printf("OK PhotoTimeTests\n");
    return 0;
}
