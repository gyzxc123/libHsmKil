/*
 * Gen5ImagerTester.cpp
 *
 *  Created on: Jun 27, 2013
 *      Author: fauthd
 */

#include "commoninclude.h"
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>

#include <types.h>
#include "Gen5ImageTester.h"

/* mt9v022 selected register addresses */
#define MT9V022_CHIP_VERSION		0x00
#define MT9V022_COLUMN_START		0x01
#define MT9V022_ROW_START		0x02
#define MT9V022_WINDOW_HEIGHT		0x03
#define MT9V022_WINDOW_WIDTH		0x04
#define MT9V022_HORIZONTAL_BLANKING	0x05
#define MT9V022_VERTICAL_BLANKING	0x06
#define MT9V022_CHIP_CONTROL		0x07
#define MT9V022_SHUTTER_WIDTH1		0x08
#define MT9V022_SHUTTER_WIDTH2		0x09
#define MT9V022_SHUTTER_WIDTH_CTRL	0x0a
#define MT9V022_TOTAL_SHUTTER_WIDTH	0x0b
#define MT9V022_RESET			0x0c
#define MT9V022_READ_MODE		0x0d
#define MT9V022_MONITOR_MODE		0x0e
#define MT9V022_PIXEL_OPERATION_MODE	0x0f
#define MT9V022_LED_OUT_CONTROL		0x1b
#define MT9V022_ADC_MODE_CONTROL	0x1c
#define MT9V022_ANALOG_GAIN		0x35
#define MT9V022_BLACK_LEVEL_CALIB_CTRL		0x47
#define MT9V022_PIXCLK_FV_LV					0x74
#define MT9V022_DIGITAL_TEST_PATTERN		0x7f
#define MT9V022_TILED_DIGITAL_GAIN			0x80
#define MT9V022_NUMBER_OF_TILES				(5*5)
#define MT9V022_DESIRED_BIN					0xA5
#define MT9V022_AEC_UPDATE_FREQUENCY		0xA6
#define MT9V022_AGC_UPDATE_FREQUENCY		0xA9
#define MT9V022_AEC_AGC_ENABLE				0xAF
#define MT9V022_AGC_GAIN_OUTPUT				0xBA
#define MT9V022_AEC_EXPOSURE_OUTPUT			0xBB
#define MT9V022_CURRENT_BIN					0xBC
#define MT9V022_MAX_TOTAL_SHUTTER_WIDTH		0xBD

// only used for AEC/AGC
static const unsigned short mt9v022_flat = 0xF4;
static const unsigned short mt9v022_center_weight[MT9V022_NUMBER_OF_TILES] =
{
	0x64,0x64,0x64,0x64,0x64,
	0x64,0xB4,0xB4,0xB4,0x64,
	0x64,0xB4,0xF4,0xB4,0x64,
	0x64,0xB4,0xB4,0xB4,0x64,
	0x64,0x64,0x64,0x64,0x64,
};

void Gen5ImagerTester::SetupCenterWeight(bool on)
{
	AssertPowered();
	if (on)
	{
		for (int i = 0; i < MT9V022_NUMBER_OF_TILES; i++)
		{
			if(GetHWL()->WriteReg(MT9V022_TILED_DIGITAL_GAIN + i, &mt9v022_center_weight[i], 1) < 0)
			{
				printf("Sensor I2C write, Reg=%02X, Value=%04X, (%d) %s\n",
						(UINT)MT9V022_TILED_DIGITAL_GAIN + i, (UINT)mt9v022_center_weight[i], errno, strerror (errno));
			}
		}
	}
	else
	{
		for (int i = 0; i < MT9V022_NUMBER_OF_TILES; i++)
		{
			if(GetHWL()->WriteReg(MT9V022_TILED_DIGITAL_GAIN + i, &mt9v022_flat, 1) < 0)
			{
				printf("Sensor I2C write, Reg=%02X, Value=%04X, (%d) %s\n",
						(UINT)MT9V022_TILED_DIGITAL_GAIN + i, (UINT)mt9v022_flat, errno, strerror (errno));
			}
		}
	}
}

#if 0 // from old kernel driver
	case V4L2_CID_AUTOGAIN:
		if (ctrl->value) {
			data = reg_set(client, MT9V022_AEC_AGC_ENABLE, 0x2);
			data -= reg_write(client, MT9V022_AGC_UPDATE_FREQUENCY, 0);
		}
		else
			data = reg_clear(client, MT9V022_AEC_AGC_ENABLE, 0x2);
		if (data < 0)
			return -EIO;
		break;
	case V4L2_CID_EXPOSURE_AUTO:
		if (ctrl->value) {
			data = reg_set(client, MT9V022_AEC_AGC_ENABLE, 0x1);
			data -= reg_write(client, MT9V022_AEC_UPDATE_FREQUENCY, 0);
		}
		else
			data = reg_clear(client, MT9V022_AEC_AGC_ENABLE, 0x1);
		if (data < 0)
			return -EIO;
		break;
	case V4L2_CID_EXPOSURE_CENTER_WEIGHT:
		if (ctrl->value) {
			int i;
			for(i = 0;i < MT9V022_NUMBER_OF_TILES; i++)
				if (reg_write(client, MT9V022_TILED_DIGITAL_GAIN+i, mt9v022_center_weight[i]) < 0)
					return -EIO;
		}
		else {
			int i;
			for(i = 0;i < MT9V022_NUMBER_OF_TILES; i++)
				if (reg_write(client, MT9V022_TILED_DIGITAL_GAIN+i, mt9v022_flat) < 0)
					return -EIO;
		}
		break;
#endif

static RegisterEntry_t MT9V022RegSettingsBase[] =
{
	{ 0x10, 0x0028 }, // ramp delay 0x1f, 0x2d (reset), now 0x28

	{ 0x15, 0x7f32 }, // reserved register upper nybble from 0x0e recommended to 0x7f to limit FPN, pixel reset time
	{ 0x1c, 0x0002 }, // linear 2, companding 3
	{ 0x20, 0x01d1 }, // bits 6:8 recommended be set for FPN (from RESET 0x0011)
	{ 0x21, 0x0018 }, // if anti-eclipse set, recomended change to 0x18 (from RESET 0x20)
	{ 0x2b, 0x0002 }, // Removes Column Noise
	{ 0x2c, 0x0000 }, // ADC reference 1.0V
	{ 0x31, 0x001f }, // next 3 are V1, V1, V3 for Hidy knees
	{ 0x32, 0x001a },
	{ 0x33, 0x0012 },

	{ 0x47, 0x8081 }, // Disable BLC algorithm
	{ 0x48, 0x0014 }, // Analog offset is manually set to 20

	{ 0x0d, 0x0300 }, // include dark rows (off)
	{ 0x74, 0x0000 }, // explicitly set Line Valid to not occur during Vertical Blanking **Pull when complete **

	{ 0xc2, 0x00c0 }, // anti-eclipse change for FPN -- fix image lag and shutter reset row corruption
	//{ MT9V022_TOTAL_SHUTTER_WIDTH, 480},
	//{ MT9V022_MAX_TOTAL_SHUTTER_WIDTH, 480},
};

static RegisterEntry_t MT9V022RegSettingsForOnChipExposureControl[] =
{
	{ MT9V022_AEC_AGC_ENABLE, 0x3 },
//	{ MT9V022_AEC_UPDATE_FREQUENCY, 0 },
	{ MT9V022_ANALOG_GAIN, 16 },
};

static RegisterEntry_t MT9V022RegSettingsExperiements[] =
{
//	{ 0x0D, 0x0300|0x0010|0x0020 }, // upside down
//	{ 0x74, 0x0002|0x0008 },  // Line Valid during Vertical Blanking
//	{ 0x74, 0x0010 },
//	{ 0x74, 0x0002 }, // inv VSYNC
//	{ 0x0d, 0x0340 }, // include dark rows
	{ 0x01,  1 }, // Column Start
	{ 0x02,  4 }, // Row Start
	{ 0x03, 480 }, // Window Height
	{ 0x04, 752 }, // Window Width
	{ 0x05, 94 }, // Horizontal Blanking
	{ 0x06, 45 }, // Vertical Blanking

};

#define SCAN_WIDTH_MT9V022	752	// fixme !!
#define SCAN_HEIGHT_MT9V022	480

static RegisterEntry_t MT9V022RegSettingsFromJeff[] =
{
	{0x03, SCAN_HEIGHT_MT9V022},
	{0x04, SCAN_WIDTH_MT9V022},
	{0x05, 94},     // Horizontal Blanking
	{0x06, 45},     // Vertical Blanking

	{0x10, 0x0028}, // ramp delay 0x1f, 0x2d (reset), now 0x28

	{0x15, 0x7f32}, // reserved register upper nybble from 0x0e recommended to 0x7f to limit FPN, pixel reset time
	{0x1c, 0x0002}, // linear 2, companding 3
	{0x20, 0x01d1}, // bits 6:8 recommended be set for FPN (from RESET 0x0011)
	{0x21, 0x0018}, // if anti-eclipse set, recomended change to 0x18 (from RESET 0x20)
	{0x2b, 0x0002}, // Removes Column Noise
	{0x2c, 0x0000}, // ADC reference 1.0V
	{0x31, 0x001f}, // next 3 are V1, V1, V3 for Hidy knees
	{0x32, 0x001a},
	{0x33, 0x0012},

	{0x47, 0x8081}, // Disable BLC algorithm
	{0x48, 0x0014}, // Analog offset is manually set to 20

	{0x0d, 0x0300},	// include dark rows
	{0x74, 0x0008},	// explitly set Line Valid to not occur during Vertical Blanking **Pull when complete **

	{0xaf, 0x0000}, // disable chip AGC/AEC
	{0xc2, 0x00c0}, // anti-eclipse change for FPN -- fix image lag and shutter reset row corruption

	/// @todo The entries below were determined by monitoring the old
	/// scan driver and the commands it sends to the HW Layer.
	{ 0x10, 0x40 },
	{ 0x13, 0x2d32 },
	{ 0x15, 0xFF32 },
	{ 0x18, 0x3e3a },
	{ 0x2C, 0x0 },
	{ 0x0B, 0x5E9 },
	{ 0x35, 0x20 },
	{ 0xC2, 0xC0 },
};


Gen5ImagerTester::Gen5ImagerTester(IHwl* pHwl)
: CPowerTester(pHwl)
{
	m_pRegEntries = MT9V022RegSettingsFromJeff;
	m_cRegEntries = ARRAY_SIZE(MT9V022RegSettingsFromJeff);
	m_iRegister_for_i2c_loop = MT9V022_ROW_START;
}

Gen5ImagerTester::~Gen5ImagerTester()
{

}

void Gen5ImagerTester::SetupSensor()
{
	CPowerTester::SetupSensor();

//	WriteRegisters(MT9V022RegSettingsBase, sizeof(MT9V022RegSettingsBase)/sizeof(RegisterEntry_t));
//	WriteRegisters(MT9V022RegSettingsForOnChipExposureControl, sizeof(MT9V022RegSettingsForOnChipExposureControl)/sizeof(RegisterEntry_t));
//	WriteRegisters(MT9V022RegSettingsExperiements, sizeof(MT9V022RegSettingsExperiements)/sizeof(RegisterEntry_t));
	SetupCenterWeight(true);
}

bool Gen5ImagerTester::SensorTest()
{
	unsigned short	value = 0;

	AssertPowered();
	if( !GetHWL()->ReadReg(MT9V022_CHIP_VERSION, &value, 1) )
	{
		printf("Sensor I2C read, Reg=%02X, Err=%d %s\n", MT9V022_CHIP_VERSION, errno, strerror (errno));
		return false;
	}

	if( (value != 0x1311) && (value != 0x1313) )
	{
		printf("No IT5000 found, ID register 0x%x\n", value);
	}
	else
	{
		printf("Sensor Revision 0x%x\n", value);
	}

	return CPowerTester::SensorTest();
}

bool Gen5ImagerTester::ConfigureForTestImage(int pattern)
{
	const int PatternPosition = 11;
	const int PatternMask = 0x1800;
	const int PatternEnaMask = 0x2000;
	const char *pattern_names[] = { "TEST_PATTERN_NONE", "Vertical Shades", "Horizontal Shades", "Diagonal Shade" };

	AssertPowered();

	// FIXME: pattern is actually one of TEST_PATTERN_XXX. Translate it to native test patterns? 
	// Here we just use value of pattern, not semantics
	pattern %= 3;
	printf("configure for test image... pattern = %s\n", pattern_names[pattern]);
	bool bOk = false;
	unsigned short ena_test_pattern = 0x00;
	unsigned short check = 0x00;

	switch(pattern)
	{
		case 0: // none
			ena_test_pattern=0;
			bOk = true;
			break;
		case 1: // Vertical shades
		case 2: // Horizontal shades
		case 3: // Diagonal shade
			ena_test_pattern = (pattern<<PatternPosition)&PatternMask;
			printf("ena_test_pattern = %04X\n", ena_test_pattern);
			ena_test_pattern |= PatternEnaMask;
			printf("ena_test_pattern = %04X\n", ena_test_pattern);
			bOk = true;
			break;
		default:
			printf("invalid setting\n");
			break;
	}

//	GetHWL()->ReadIIC_Sensor(0x01, &check, 1);
//	printf("Reg 01 = %X\n", check);

	if(bOk) // got a valid argument
	{
		printf("write sensor with new ena_test_pattern\n");
		GetHWL()->WriteReg(MT9V022_DIGITAL_TEST_PATTERN, &ena_test_pattern,1);

		if(GetHWL()->ReadReg(MT9V022_DIGITAL_TEST_PATTERN, &check, 1))
		{
			printf("read sensor ena_test_pattern check good\n");
			if(check != ena_test_pattern)
			{
				printf("ena_test_pattern write error!!!\n");
				bOk = false;
			}
		}
	}

	if( bOk )
	{
		ena_test_pattern = 0x0024; // row noise correction off
		GetHWL()->WriteReg(0x70, &ena_test_pattern,1);
	}
	return bOk;
}
