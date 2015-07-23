//! \file


#include "CommonInclude.h"
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>

// a manual test:
// echo "test dmesg" >/dev/kmsg

static int dmesg=-1;

DBGAPI void DmesgOpen()
{
	if (dmesg==-1)
	{
		dmesg = open("/dev/kmsg", O_WRONLY, 0);
	}
}

DBGAPI void DmesgClose()
{
	close(dmesg);
	dmesg=-1;
}

void DmesgPuts(const char *szLog)
{
	write(dmesg, szLog, strlen(szLog));
}

static pthread_mutex_t LogProtect = PTHREAD_MUTEX_INITIALIZER;

DBGAPI void DmesgOut(const char *szLog)
{
	pthread_mutex_lock(&LogProtect);
	DmesgPuts(szLog);
	pthread_mutex_unlock(&LogProtect);
}

DBGAPI void DmesgOut1(const char *psz, ...)
{
	pthread_mutex_lock(&LogProtect);
	const int BufSize=512;
	char Buf[BufSize];
	va_list	pArgs;
	va_start( pArgs, psz );
	vsnprintf( Buf, BufSize-1, psz, pArgs );
	DmesgPuts(Buf);
	pthread_mutex_unlock(&LogProtect);
}

