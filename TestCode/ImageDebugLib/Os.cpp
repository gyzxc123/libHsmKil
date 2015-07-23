

#include "CommonInclude.h"
#if LINUX
#include <iostream>
#endif
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "Os.h"

using namespace std;

DBGAPI int kbhit(void)
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

DBGAPI bool AbortOnKbHit()
{
	bool abort = kbhit();
	if(abort)
	{
		getchar();
	}
	return abort;
}

DBGAPI CTerminal::CTerminal()
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

DBGAPI CTerminal::~CTerminal()
{
	if(!m_error)
	{
		tcsetattr(STDIN_FILENO, TCSANOW, &m_oldt);
	}
}
