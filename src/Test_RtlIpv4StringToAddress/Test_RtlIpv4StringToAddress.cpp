// Test_RtlIpv4StringToAddress.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <ip2string.h>
#include <stdio.h>

#pragma comment(lib, "ntdll.lib")

void Test(const char* ip, BOOLEAN strict)
{
    struct in_addr addr = {};
    const char* term = NULL;

    NTSTATUS status = RtlIpv4StringToAddressA(ip, strict, &term, &addr);

    printf("Input=\"%s\" Strict=%d ¡÷ status=0x%08X term_offset=%d",
        ip, strict,
        status, (int)(term - ip));

    if (status == 0)
        printf(" addr=%u.%u.%u.%u",
            addr.S_un.S_un_b.s_b1,
            addr.S_un.S_un_b.s_b2,
            addr.S_un.S_un_b.s_b3,
            addr.S_un.S_un_b.s_b4);

    printf("\n");
}

int main()
{
    Test("187.228.33.9", FALSE);
    Test("187.228.33.9", TRUE);
    Test("10.0.", FALSE);
    Test("10.0.", TRUE);
    Test("192.168.1", FALSE);
    Test("192.168.1", TRUE);
    Test("192.168.1.", FALSE);
    Test("192.168.1.", TRUE);
    Test("192.168.1.in-addr.arpa.", FALSE);
    Test("192.168.1.in-addr.arpa.", TRUE);
    Test("1.0.0.139.in-addr.arpa.", FALSE);
    Test("1.0.0.139.in-addr.arpa.", TRUE);
    Test("192.168.1.y", FALSE);
    Test("192.168.1.y", TRUE);
    Test("192.168.1y", FALSE);
    Test("192.168.1y", TRUE);

    return 0;
}
