#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "Os.h"

using namespace std;

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}

unsigned long GetTickCount()
{
 	struct timespec ts;
 	clock_gettime(CLOCK_MONOTONIC, &ts); // or CLOCK_PROCESS_CPUTIME_ID
 	unsigned int ret(ts.tv_sec);
 	return ret * 1000 + ts.tv_nsec / 1000000;
}

CTerminal::CTerminal()
{
	struct termios newt;
	m_error = tcgetattr(STDIN_FILENO, &m_oldt);
	if(!m_error)
	{
		newt = m_oldt;
		newt.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	}
}

CTerminal::~CTerminal()
{
	if(!m_error)
	{
		tcsetattr(STDIN_FILENO, TCSANOW, &m_oldt);
	}
}
