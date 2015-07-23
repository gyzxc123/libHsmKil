/*
 * UserSpaceI2c.h
 *
 *  Created on: Dec 2, 2013
 *      Author: fauthd
 */


#pragma once



class CUserSpaceI2c
{
public:
	CUserSpaceI2c();
	~CUserSpaceI2c();
	bool Init();

	bool WriteIIC_PSOC(const unsigned char *buffer, unsigned char cnt);
	bool ReadIIC_PSOC(unsigned char *buffer, unsigned char cnt);
	bool ResetSensor();

protected:
	int m_fd;
};

