#include <stdio.h>
#include <syslog.h>

int main(){
    openlog(NULL, LOG_CONS, LOG_SYSLOG);
    syslog(LOG_INFO, "hello");
    closelog();
    return 0;
}
