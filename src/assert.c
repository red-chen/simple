#include <assert.h>

#include <stdarg.h>
#include <string.h>
#include <unistd.h>

void va_list_print(char log_message[], char* format, ...)
{
    va_list ap_;
    va_start(ap_, format);
    vsprintf(log_message, format, ap_);
    va_end(ap_);
}
