

#include "commoninclude.h"
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>

#include <types.h>
#include "Gen6ImageTester.h"

#undef DBG_FUNC
#define DBG_FUNC()

using namespace std;

#define SCAN_WIDTH_JADE 832
#define SCAN_HEIGHT_JADE 640

enum TWI_Registers
{
	MISCEL_CTRL = 0x0F,
	MISCEL_CTRL_STDBY_RQST 	= A_BIT(0), //	(1 << 0),
	MISCEL_CTRL_TRIG	= A_BIT(1), //	(1 << 1),
	MISCEL_CTRL_TWI_LOC	= A_BIT(3), //  (1 << 3)
	MISCEL_CTRL_HI_Z	= A_BIT(6), //  (1 << 6)
	FRAME_TINT = 0x80,
};

#define WINDOW_X_FIRST_COL_REGISTER	0x83
#define WINDOW_Y_FIRST_ROW_REGISTER 0x84
#define WINDOW_X_WIDTH_REGISTER     0x85
#define WINDOW_Y_HEIGHT_REGISTER    0x86
#define ROW_Y_OFFSET                26
#define COL_X_OFFSET                6
#define TWAIT_TIME                  0x0024     // frame time delay
#define ZERO_BIT4                   0xffffffef


//You might want to define DEBUG_SLOW_FRAME while we debug so it slows things down a bit.
//#define DEBUG_SLOW_FRAME 1
//#define ENABLE_TEST_PATTERN 1

// Jade Sensor Register Settings
static RegisterEntry_t JadeRegEntriesForHHPExposureControl[] =
{
	// Configure Filter
	{0,0},
	{1,0},
	{2,0},
	{3,0},
	{4,0},
	{5,1},
	{6,0},
	{7,0},
	{8,0},
	{9,0},

	// Image Register Settings
	{WINDOW_X_FIRST_COL_REGISTER, ROW_Y_OFFSET},   // 0x83
	{WINDOW_Y_FIRST_ROW_REGISTER, COL_X_OFFSET},   // 0x84
	{WINDOW_X_WIDTH_REGISTER, SCAN_WIDTH_JADE },   // 0x85
	{WINDOW_Y_HEIGHT_REGISTER, SCAN_HEIGHT_JADE }, // 0x86

	// Remaining Register Settings
	{0x0e,0x0000},                                  // ignore STANDBY and TRIG pin
	{0x0b,0x0018},                                  // frame_config (imager mounted upside down, flip H & V)
	{0x0d,0x0001},                                  // frame_flash_config  //16.7uS
	{0x0a,0x002f},                        // synchro , set vert and horiz sync characteristics here                             //lmc  debug,  was 2f, change to divide clock /2 to 6f
	{0x82,0x0100},                        // REAL gain = 2
	{0x0f,0x0024&ZERO_BIT4}, // miscel_ctrl
	{0x41,0x0068},                                   // fa_carac_analog
	{0x44,0x0008},                                   // fa_low_pwr
	{0x46,0x0003},                                   // fa_carac_sdec_len
	{0xc1,0x1334},                                   // fa_pixtime_tra_com
	{0xc2,0x0b34},                                   // fa_pixtime_sel
	{0xc3,0x4352},                                   // fa_pixtime_res_com
	{0xc4,0x00a0},                                   // fa_pixtime_res_mem
	{0xc5,0x0d31},                                   // fa_pixtime_res
	{0xc6,0x0096},                                   // fa_pixtime_shs
	{0xc7,0x0096},                                   // fa_pixtime_shr1
	{0xc8,0x1a40},                                   // fa_pixtime_shr2
	{0xca,0x0098},                                   // fa_pixtime_oc
	{0xcb,0x0083},                                   // fa_pixtime_on_tra_com
	{0xcc,0x0d1b},                                   // fa_pixtime_res_on_readout
	{0xce,0xb50f},                                   // fa_carac_ana
	{0xdc,0x0400},                                   // fa_carac_ana_force
	{0xdd,0x0400},                                  // fa_carac_ana_val
#ifdef ENABLE_TEST_PATTERN
	{0x13,0x0002},                                   // ena_test_pattern (0x0002 for test pattern, 0x0000 for image, 0x0001 for moving pattern)
#else
	{0x13,0x0000},                                   // ena_test_pattern (0x0002 for test pattern, 0x0000 for image, 0x0001 for moving pattern)
#endif
#ifdef DEBUG_SLOW_FRAME
	{0x81,0xffff},                     // vert wait time
#else
	{0x81,TWAIT_TIME},                      // vert wait time
#endif
};

Gen6ImagerTester::Gen6ImagerTester(IHwl* pHwl)
: CPowerTester(pHwl)
{
	m_pRegEntries = JadeRegEntriesForHHPExposureControl;
	m_cRegEntries = ARRAY_SIZE(JadeRegEntriesForHHPExposureControl);
	m_iRegister_for_i2c_loop = 0xdd; // fa_carac_ana_val
}

Gen6ImagerTester::~Gen6ImagerTester()
{

}

#define ENA_TEST_PATTERN_REGISTER 0x13

bool Gen6ImagerTester::ConfigureForTestImage(int pattern)
{
	printf("configure for test image... pattern = %s\n", (pattern == TEST_PATTERN_NONE)  ? "TEST_PATTERN_NONE" :
                   (pattern == TEST_PATTERN_FIXED)    ? "TEST_PATTERN_FIXED" :
                   (pattern == TEST_PATTERN_MOVING) ? "TEST_PATTERN_MOVING" :
                   "???");
	AssertPowered();
	bool bOk = false;
	unsigned short ena_test_pattern = 0x00;
	unsigned short check = 0x00;

	switch(pattern)
	{
		case 0: // none
		case 1: // moving
		case 2: // fixed
		case 3: // functional
			ena_test_pattern = pattern; // should be ok
			printf("ena_test_pattern = %d\n", ena_test_pattern);
			bOk = true;
			break;
		default:
			printf("invalid setting\n");
			break;
	}

	if(bOk) // got a valid argument
	{
		printf("write sensor with new ena_test_pattern\n");
		GetHWL()->WriteReg(ENA_TEST_PATTERN_REGISTER, &ena_test_pattern,1);

		if(GetHWL()->ReadReg(ENA_TEST_PATTERN_REGISTER, &check, 1))
		{
			printf("read sensor ena_test_pattern check good\n");
			if(check != ena_test_pattern)
			{
				printf("ena_test_pattern write error!!!\n");
				bOk = false;
			}
		}
	}

	return bOk;
}

void Gen6ImagerTester::WriteRegisters(RegisterEntry_t *pRegs, size_t NumRegs)
{
	AssertPowered();
	for(size_t i=0; i<NumRegs; i++)
	{
		// hi-jack the table for test image
		if(pRegs[i].nRegister == ENA_TEST_PATTERN_REGISTER) // 0x13
		{
			printf("hi-jacking the table for test image!!\n");

			pRegs[i].nValue = m_iTestPattern;

			printf("pRegs[i].nValue now = %s\n", (m_iTestPattern == TEST_PATTERN_NONE)  ? "TEST_PATTERN_NONE" :
				(m_iTestPattern == TEST_PATTERN_FIXED)    ? "TEST_PATTERN_FIXED" :
				(m_iTestPattern == TEST_PATTERN_MOVING) ? "TEST_PATTERN_MOVING" :
				"???");
		}

		GetHWL()->WriteReg(pRegs[i].nRegister, &pRegs[i].nValue, 1);
	}
}

void Gen6ImagerTester::TriggerSensor(bool bTriggerOnce)
{
	DBG_FUNC();
	AssertPowered();
	unsigned short miscel_ctrl = 0x00;
	unsigned short check = 0x00;

	if (GetHWL()->ReadReg(MISCEL_CTRL, &miscel_ctrl,1))
	{
		miscel_ctrl &= ~MISCEL_CTRL_STDBY_RQST;
		miscel_ctrl |= MISCEL_CTRL_TRIG;
		miscel_ctrl &= ~(MISCEL_CTRL_HI_Z);
		GetHWL()->WriteReg(MISCEL_CTRL, &miscel_ctrl,1);

		if(GetHWL()->ReadReg(MISCEL_CTRL, &check, 1))
		{
			if(check != miscel_ctrl)
				puts("miscel_ctrl write error!!!");
		}

		if(bTriggerOnce)
		{
			miscel_ctrl &= ~(MISCEL_CTRL_TRIG);
			GetHWL()->WriteReg(MISCEL_CTRL, &miscel_ctrl,1);
		}
	}
	else
	{
		puts("Sensor read error!!!");
	}
	m_bTriggered = !bTriggerOnce;
}

void Gen6ImagerTester::UnTriggerSensor()
{
	DBG_FUNC();
	AssertPowered();
	unsigned short miscel_ctrl = 0x00;
	unsigned short check = 0x00;

	if(m_bTriggered)
	{
		if (GetHWL()->ReadReg(MISCEL_CTRL, &miscel_ctrl,1))
		{
			miscel_ctrl &= ~(MISCEL_CTRL_TRIG | MISCEL_CTRL_STDBY_RQST);
			miscel_ctrl &= ~(MISCEL_CTRL_HI_Z);
			GetHWL()->WriteReg(MISCEL_CTRL, &miscel_ctrl,1);

			if(GetHWL()->ReadReg(MISCEL_CTRL, &check, 1))
			{
				if(check != miscel_ctrl)
					puts("miscel_ctrl write error!!!");
			}
		}
		else
		{
			puts("Sensor read error!!!");
		}
		m_bTriggered=false;
	}
}

bool Gen6ImagerTester::SensorTest()
{
	printf("+++++ Executing SensorTest +++++\n");
	CAutoPower(this);

	unsigned short read = 0;
	unsigned short write = 0;
	unsigned short check = 0;
	unsigned short reset_write = 0;

	// Read the current value
	printf("Read the current value...\n");
	if(!GetHWL()->ReadReg(ENA_TEST_PATTERN_REGISTER, &read, 1))
	{
		printf("read sensor failed.\n");
		return false;
	}
	printf("ReadReg from 0x%x, value = %d\n", ENA_TEST_PATTERN_REGISTER, read);

	// set reset value (for end of test!)
	reset_write = read;

	// change the value
	printf("Change the value...\n");
	read++;
	if(read > 3) // valid settings are 0 to 3
		read = 0;
	printf("Set write to value of read...\n");
	write = read;
	printf("new write = %d\n", write);

	// Write the new value
	printf("Write the new value...\n");
	printf("WriteIIC_Sensor to 0x%x, value = %d\n", ENA_TEST_PATTERN_REGISTER, write);
	if(!GetHWL()->WriteReg(ENA_TEST_PATTERN_REGISTER,&write,1))
	{
		printf("write sensor failed.\n");
		return false;
	}

	// Read the new value
	printf("Read the new value...\n");
	if(!GetHWL()->ReadReg(ENA_TEST_PATTERN_REGISTER, &check, 1))
	{
		printf("read sensor failed.\n");
		return false;
	}
	printf("ReadReg from 0x%x, value = %d\n", ENA_TEST_PATTERN_REGISTER, check);


	printf("Check value...\n");
	printf("write = %d, check = %d\n", write, check);
	if(check != write)
	{
		printf("check != write\n");
		return false;
	}

	// Write the old value back (reset)
	printf("Write the old value back (reset)...\n");
	//reset_write = read;
	printf("WriteIIC_Sensor to 0x%x, value = %d\n", ENA_TEST_PATTERN_REGISTER, reset_write);
	if(!GetHWL()->WriteReg(ENA_TEST_PATTERN_REGISTER,&reset_write,1))
	{
		printf("write sensor failed.\n");
		return false;
	}

	printf("----- SensorTest Complete -----\n");

	return true;
}

#define PSOC_BOOST_ENABLE_REG 0x37

bool Gen6ImagerTester::PsocTest()
{
	printf("+++++ Executing PsocTest +++++\n");
	CAutoPower(this);

	BYTE psocBoostEnableRegValue = 0x01;           //one to enable , zero to disable
	BYTE read = 0;
	BYTE write = 0;
	BYTE check = 0;
	BYTE reset_write = 0;

	// Read the current value
	printf("Read the current value...\n");
	if(!GetHWL()->ReadIIC_PSOC(PSOC_BOOST_ENABLE_REG, &read, 1))
	{
		printf("read psoc failed.\n");
		return false;
	}
	printf("GetHWL()->ReadIIC_PSOC from 0x%x, value = %d\n", PSOC_BOOST_ENABLE_REG, read);

	// set reset_write to read (for end of test)
	reset_write = read;

	// Negate the read value
	printf("Negate the read value...\n");
	write = !read;
	printf("write = %d\n", write);

	// Write the new value
	printf("Write the new value...\n");
	printf("GetHWL()->WriteIIC_PSOC to 0x%x, value = %d\n", PSOC_BOOST_ENABLE_REG, write);
	if(!GetHWL()->WriteIIC_PSOC(PSOC_BOOST_ENABLE_REG,&write,1))
	{
		printf("write psoc failed.\n");
		return false;
	}

	// Read the new value
	printf("Read the new value...\n");
	if(!GetHWL()->ReadIIC_PSOC(PSOC_BOOST_ENABLE_REG, &check, 1))
	{
		printf("read psoc failed.\n");
		return false;
	}
	printf("GetHWL()->ReadIIC_PSOC from 0x%x, value = %d\n", PSOC_BOOST_ENABLE_REG, check);


	printf("Check value...\n");
	printf("write = %d, check = %d\n", write, check);
	if(check != write)
	{
		printf("check != write\n");
		return false;
	}

	// Write the old value back (reset)
	printf("Write the old value back (reset)...\n");
	reset_write = read;
	printf("GetHWL()->WriteIIC_PSOC to 0x%x, value = %d\n", PSOC_BOOST_ENABLE_REG, reset_write);
	if(!GetHWL()->WriteIIC_PSOC(PSOC_BOOST_ENABLE_REG,&reset_write,1))
	{
		printf("write psoc failed.\n");
		return false;
	}

	printf("----- PsocTest Complete -----\n");

	return true;
}

bool Gen6ImagerTester::ClockTest()
{
#if 0	// new IHwl has no interface fpor clock chip :-(
	printf("+++++ Executing ClockTest +++++\n");

	unsigned char read[3];
	unsigned char write[3];
	unsigned char check[3];
	unsigned char reset_write[3];

	unsigned char clock_conf_new = 0;
	unsigned char clock_conf_old = 0;


	// Read the current value
	printf("Read the current value...\n");
	if(!ReadIIC_Clock_w_ADD(0x0, read, sizeof(read)))
	{
		printf("read clock chip failed.\n");
		return false;
	}

	for(int i = 0; i < 3; i++)
		printf("ReadIIC_Clock_w_ADD value[%d] = 0x%x\n", i, read[i]);

	clock_conf_old = read[2]; // only going to mod clock config

	printf("clock_conf_old = 0x%x\n", clock_conf_old);

	// set reset value (for end of test!)
	/*for(int i = 0; i < 3; i++)
	{
		reset_write[i] = read[i];
		write[i] = read[i];
	}*/

	//
	// change the clock config value
	//

	if(clock_conf_old == 0x82)
		clock_conf_new = 0x80;
	else
		clock_conf_new = 0x82;

	printf("clock_conf_new = 0x%x\n", clock_conf_new);

	write[0] = 0x01;
	write[1] = clock_conf_new & 0xff;

	// change reset_write
	reset_write[0] = 0x01;
	reset_write[1] = clock_conf_old & 0xff;


	for(int i = 0; i < 2; i++)
		printf("new write[%d] = 0x%x\n", i, write[i]);

	// Write the new value
	printf("Write the new values...\n");
	for(int i = 0; i < 2; i++)
		printf("WriteIIC_Clock_w_ADD write[%d] = 0x%x\n", i, write[i]);

	// For write, must include count of bytes as 1st arg.
	// On the i2c wire: <i2c addr|[R|W]> <reg> <cnt> <val>
	// Example of transmission:  D2 01 01 82
	// with ClockConfig value of 0x82.
	if (!WriteIIC_Clock_w_ADD(0x01, &write[0], 2))
	{
		printf("write clock chip failed.\n");
		return false;
	}

	// Read the new value
	printf("Read the new value...\n");
	if(!ReadIIC_Clock_w_ADD(0x00, check, sizeof(check)))
	{
		printf("read clock chip failed.\n");
		return false;
	}

	for(int i = 0; i < 3; i++)
		printf("ReadIIC_Clock_w_ADD value[%d] = 0x%x\n", i, check[i]);


	printf("Check value...\n");
	printf("check[2] = 0x%x, clock_conf_new = 0x%x\n", check[2], clock_conf_new);
	if(check[2] != clock_conf_new)
	{
		printf("check != clock_conf_new\n");
		return false;
	}

	// Write the old value back (reset)
	printf("Write the old value back (reset)...\n");

	for(int i = 0; i < 2; i++)
		printf("WriteIIC_Sensor reset_write[%d] = 0x%x\n", i, reset_write[i]);

	if (!WriteIIC_Clock_w_ADD(0x01, &reset_write[0], 2))
	{
		printf("write clock chip failed.\n");
		return false;
	}

	printf("Read the values again...\n");
	if(!ReadIIC_Clock_w_ADD(0x00, read, sizeof(read)))
	{
		printf("read clock chip failed.\n");
		return false;
	}

	for(int i = 0; i < 3; i++)
		printf("ReadIIC_Clock_w_ADD value[%d] = 0x%x\n", i, read[i]);

	printf("----- ClockTest Complete -----\n");

#endif
	return true;
}

bool Gen6ImagerTester::WriteBoost()
{
	DBG_FUNC();
	AssertPowered();

//	unsigned char nOffset = 0x01;
//	unsigned char nRegValue[2] = {0x01,0xC2};
	const BYTE psocBoostEnableRegValue = 0x01;           //one to enable , zero to disable
	BYTE readPsocBoostEnableReg = 0;
	int boostRetryCount = 3;
	bool bChange=false;
	
	GetHWL()->ReadIIC_PSOC(PSOC_BOOST_ENABLE_REG, &readPsocBoostEnableReg, 1);
	printf("readPsocBoostEnableReg=%i\n", (int)readPsocBoostEnableReg);
	while( (boostRetryCount > 0) && (psocBoostEnableRegValue != readPsocBoostEnableReg) )
	{
		bChange=true;
		if(!GetHWL()->WriteIIC_PSOC(PSOC_BOOST_ENABLE_REG,&psocBoostEnableRegValue,1))
		{
			puts("i2c issue");
		}
		GetHWL()->ReadIIC_PSOC(PSOC_BOOST_ENABLE_REG, &readPsocBoostEnableReg, 1);
		boostRetryCount--;

//		DbgOut1("Boost Retries(%d)\n", boostRetryCount);
	};

	if(boostRetryCount == 0)
	{
		printf("Took too many tries to write boost");
		return false;
	}

	if(bChange)
		WaitMilliseconds(25);

	return true;
}

void Gen6ImagerTester::ResetSensor()
{
	// FIXME: untested - and likely not working, the gen6 PSOC does not seem to support this command.
	CPowerTester::ResetSensor();
}
