
// Test_NsIpv4StringToAddress.cpp : This test is used to validate NsIpv4StringToAddress() in ENG-1059544 / PR-8762
#include <ntstatus.h>
#include <windows.h>

#include <ip2string.h>
#include <stdio.h>

static __inline BOOL IsDecDigitChar(const CHAR c) {
    return (c >= '0' && c <= '9');
}

static BOOL ParseIPv4Field(const CHAR* src_str, ULONG* out_value, ULONG* buf_remain, const CHAR** str_end)
{
    ULONG value = 0;
    ULONG digits = 0;

    const CHAR* str = NULL;

    if (!src_str || !out_value || !buf_remain || !str_end) {
        return FALSE;
    }

    str = src_str;

    //only accept 3 digit in each field => 0~255
    //only accept decimal number
    while ((*buf_remain) > 0 && IsDecDigitChar(str[0])) {
        if (digits > 2) {
            return FALSE;
        }

        value = value * 10 + (str[0] - '0');
        digits++;
        str++;
        (*buf_remain)--;
    }

    if (value > 255 || 0 == digits) {
        return FALSE;
    }

    *out_value = value;
    *str_end = str;
    return TRUE;
}

//This is a custom replacement for `RtlIpv4StringToAddressA` with strict=TRUE behavior.
//IP addresses must use decimal digits only, and all four fields must be present (e.g. "192.168.0.1").
//Partial addresses like "192.168" or "192.168.0" are rejected.
NTSTATUS NsIpv4StringToAddressA(
    _In_ PCSTR src_str,
    _In_ ULONG src_buflen,
    _Out_ PCSTR* terminator,
    _Out_ struct in_addr* addr) {

    NTSTATUS ret = STATUS_SUCCESS;
    PCSTR str = src_str;
    ULONG fields[4] = { 0 };
    int field_count = 0;
    ULONG remain = src_buflen;

    if (NULL == src_str || 0 == src_buflen || NULL == terminator || NULL == addr) {
        return STATUS_INVALID_PARAMETER;
    }

    //there are only 4 fields in IPv4 string
    for (int i = 0; i <= 3; i++) {
        if (0 == remain) {
            ret = STATUS_DATA_ERROR;
            break;
        }

        if (str[0] == '.') {
            if (0 == i) {
                ret = STATUS_DATA_ERROR;
                break;
            }
            else {
                str++;
                remain--;
                if (0 == remain) {
                    ret = STATUS_DATA_ERROR;
                    break;
                }

                //reject ip string has double dot like "10..0.1"
                if (str[0] == '.') {
                    ret = STATUS_INVALID_PARAMETER;
                    break;
                }
            }
        }

        if (str[0] == '\0') {
            break;
        }

        if (!ParseIPv4Field(str, &fields[i], &remain, &str)) {
            //something wrong in fields. e.g. invalid char like 'A'
            ret = STATUS_DATA_ERROR;
            break;
        }

        field_count++;
    }

    //if no fields correctly parsed...
    if (4 != field_count) {
        ret = STATUS_DATA_ERROR;
    }

    if (STATUS_SUCCESS == ret) {
        //if src_str == "192." , addr will be 192.0.0.0
        //if src_str == "192.168." , addr will be 192.168.0.0
        //if src_str == "192.168.7" , addr will be 192.168.7.0
        addr->S_un.S_un_b.s_b1 = fields[0];
        addr->S_un.S_un_b.s_b2 = fields[1];
        addr->S_un.S_un_b.s_b3 = fields[2];
        addr->S_un.S_un_b.s_b4 = fields[3];
        *terminator = str;
    }
    return ret;
}

void Test(PCSTR addr_str) {
    //NTSTATUS NsIpv4StringToAddressA(
    //    _In_ PCSTR src_str,
    //    _In_ ULONG src_buflen,
    //    _Out_ PCSTR * terminator,
    //    _Out_ struct in_addr* addr) {

    NTSTATUS status = STATUS_UNSUCCESSFUL;
    struct in_addr addr = {};
    const char* term = NULL;

    status = NsIpv4StringToAddressA(addr_str, strlen(addr_str) + 1, &term, &addr);
    printf("NsIpv4StringToAddressA(%s) return 0x%08X, ", addr_str, status);
    if (STATUS_SUCCESS == status) {
        printf(" addr=%u.%u.%u.%u",
            addr.S_un.S_un_b.s_b1,
            addr.S_un.S_un_b.s_b2,
            addr.S_un.S_un_b.s_b3,
            addr.S_un.S_un_b.s_b4);
    }
    printf("\n");
}

int main()
{
    Test("187.228.33.9");
    Test("192.168.1.");
    Test("192.168.1");
    Test("10.10.");
    Test("10.10");
    Test("1.0.0.139.in-addr.arpa.");
    Test("192.168.1.y");
    Test("192.168.1y");
}
