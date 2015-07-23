
#pragma once

#include <termios.h>

int kbhit(void);
bool AbortOnKbHit();


///////////////////////////////////////////////////////////////////////////////
//! Class to switch terminal into non-canonical mode.
/*! In canonical mode the input is made available line by line.  An input line is
	available when one of the line delimiters is typed (NL, EOL, EOL2)

	In noncanonical mode input is available immediately (without the user having
	to type a line-delimiter character), and line editing is disabled.
	This is what we ant in the simple applications.

	Usage:
	Place a class object somewere in your main function. The dtor will ensure to
	switch back to canonical.
	/code
	CTerminal NonCanonical;
	/endcode
*/
class CTerminal
{
public:
	CTerminal();
	~CTerminal();
protected:
	int m_error;							 	//!< 0 if no error happended in ctor
	struct termios m_oldt;					 	//!< Save old termional settings
};

