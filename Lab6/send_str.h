#ifndef SEND_STR_
#define SEND_STR_

void putty_puts(const char* str)
{
    while (*str != '\0')
    {
        cyBot_sendByte(*str);
        str++;
    }
}

#endif //!SEND_STR_
