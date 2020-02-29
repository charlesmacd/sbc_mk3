



#include <stdio.h>

#include "sbc_pio.hpp"

const PIOSignalInfo_t pio_signals[] =
{
/* DBL */
	{kTD0, 		kBIDIR, 	kNONE,		kDBL,	0}, /* 574/245 */
	{kTD1, 		kBIDIR, 	kNONE,		kDBL,	1},
	{kTD2, 		kBIDIR, 	kNONE,		kDBL,	2},
	{kTD3, 		kBIDIR, 	kNONE,		kDBL,	3},
	{kTD4, 		kBIDIR, 	kNONE,		kDBL,	4},
	{kTD5, 		kBIDIR, 	kNONE,		kDBL,	5},
	{kTD6, 		kBIDIR, 	kNONE,		kDBL,	6},
	{kTD7, 		kBIDIR, 	kNONE,		kDBL,	7},
/* DBH */
	{kTD8, 		kBIDIR, 	kNONE,		kDBH,	0}, /* 574/245 */
	{kTD9, 		kBIDIR, 	kNONE,		kDBH,	1},
	{kTD10,		kBIDIR, 	kNONE,		kDBH,	2},
	{kTD11,		kBIDIR, 	kNONE,		kDBH,	3},
	{kTD12,		kBIDIR, 	kNONE,		kDBH,	4},
	{kTD13,		kBIDIR, 	kNONE,		kDBH,	5},
	{kTD14,		kBIDIR, 	kNONE,		kDBH,	6},
	{kTD15,		kBIDIR, 	kNONE,		kDBH,	7},
/* ADL */
	/* D0 : ADL_SPAREIN0 */
	{kTA1, 		kINPUT, 	kNONE, 		kADL,	1}, /* 245 */
	{kTA2, 		kINPUT, 	kNONE, 		kADL,	2},
	{kTA3, 		kINPUT, 	kNONE, 		kADL,	3},
	{kTA4, 		kINPUT, 	kNONE, 		kADL,	4},
	{kTA5, 		kINPUT, 	kNONE, 		kADL,	5},
	{kTA6, 		kINPUT, 	kNONE, 		kADL,	6},
	{kTA7, 		kINPUT, 	kNONE, 		kADL,	7},
/* ADM */
	{kTA8, 		kINPUT, 	kNONE, 		kADM,	0},	/* 245 */
	{kTA9, 		kINPUT, 	kNONE,		kADM,	1},
	{kTA10,		kINPUT, 	kNONE,		kADM,	2},
	{kTA11,		kINPUT, 	kNONE,		kADM,	3},
	{kTA12,		kINPUT, 	kNONE,		kADM,	4},
	{kTA13,		kINPUT, 	kNONE,		kADM,	5},
	{kTA14,		kINPUT, 	kNONE,		kADM,	6},
	{kTA15,		kINPUT, 	kNONE,		kADM,	7},
/* ADH */
	{kTA16,		kINPUT, 	kNONE, 		kADH,	0}, /* 245 */
	{kTA17,		kINPUT, 	kNONE, 		kADH,	1},
	{kTA18,		kINPUT, 	kNONE, 		kADH,	2},
	{kTA19,		kINPUT, 	kNONE, 		kADH,	3},
	{kTA20,		kINPUT, 	kNONE, 		kADH,	4},
	{kTA21,		kINPUT, 	kNONE, 		kADH,	5},
	{kTA22,		kINPUT, 	kNONE, 		kADH,	6},
	{kTA23,		kINPUT, 	kNONE, 		kADH,	7},
/* CTOL */
	{kTDTACK, 	kOUTPUT, 	kWSG_IN,	kCTL_OUT, 	0}, /* 574 */
	{kTBGACK,	kOUTPUT,	kNONE,		kCTL_OUT, 	1},
	{kTBR, 		kOUTPUT,	kNONE,		kCTL_OUT, 	2},
	{kTVPA,		kOUTPUT,	kNONE,		kCTL_OUT, 	3},
	{kTBERR, 	kOUTPUT, 	kNONE,		kCTL_OUT, 	4},
	{kTHALTO,  	kOUTPUT,	kNONE,		kCTL_OUT, 	5},
	{kTRESETO, 	kOUTPUT,	kNONE,		kCTL_OUT, 	6},
	{kTPS_EN, 	kOUTPUT,	kNONE,		kCTL_OUT, 	7},
/* CTIL */
	{kTFC0, 	kINPUT, 	kNONE,		kCTL_IN, 	0}, /* 245 */
	{kTFC1, 	kINPUT, 	kNONE,		kCTL_IN, 	1},
	{kTFC2, 	kINPUT, 	kNONE,		kCTL_IN, 	2},
	{kTE,		kINPUT,		kNONE,		kCTL_IN, 	3},
	{kTVMA,		kINPUT,		kNONE,		kCTL_IN, 	4},
	{kTHALTI,  	kBIDIR,		kNONE,		kCTL_IN, 	5},
	{kTRESETI, 	kBIDIR,		kNONE,		kCTL_IN, 	6},
	/* D7 : CTIL_SPAREIN7 */
/* CTIH */
	{kTAS, 		kINPUT,		kWSG_OUT,	kCTH_IN,	0},
	{kTUDS,		kINPUT,		kNONE,		kCTH_IN,	1},
	{kTLDS,		kINPUT,		kNONE,		kCTH_IN,	2},
	{kTRW, 		kINPUT,		kNONE,		kCTH_IN,	3},
	{kTBG, 		kINPUT,		kNONE,		kCTH_IN,	4},
	/* D5 : CTIH_SPAREIN5 */
	/* D6 : CTIH_SPAREIN6 */
	/* D7 : CTIH_SPAREIN7 */
/* CTOL */
	{kTIPL2, 	kOUTPUT, 	kNONE,		kCTH_OUT,	0},
	{kTIPL1, 	kOUTPUT, 	kNONE,		kCTH_OUT,	1},
	{kTIPL0, 	kOUTPUT, 	kNONE,		kCTH_OUT,	2}, /* 574 */
	{kTCLK,		kINPUT,		kCLOCK,		kCTH_OUT,	3},
	{kTCLKSEL0,	kOUTPUT, 	kNONE,		kCTH_OUT,	4},
	{kTCLKSEL1,	kOUTPUT, 	kNONE,		kCTH_OUT,	5},
	{kOUT3, 	kOUTPUT, 	kNONE,		kCTH_OUT,	6}, 
	{kOUT6, 	kOUTPUT, 	kNONE,		kCTH_OUT,	7}, 
/* MISC */
	{kUM_TXE,	kINPUT, 	kNONE,		kMISC_IN,	0},
	{kUM_RXF,	kINPUT, 	kNONE,		kMISC_IN,	1},
	{kUM_PWE,	kINPUT, 	kNONE,		kMISC_IN,	2},
	{kUSER_LED,	kINPUT, 	kNONE,		kMISC_IN,	4},
	{kTPS_FLT,	kINPUT, 	kNONE,		kMISC_IN,	5},
	{kFF_CKO,	kINPUT, 	kNONE,		kMISC_IN,	6},
	{kWAITREQ,	kINPUT, 	kNONE,		kMISC_IN,	7},
};

constexpr int kNumPioSignalInfo = sizeof(pio_signals) / sizeof(PIOSignalInfo_t);

/* End */