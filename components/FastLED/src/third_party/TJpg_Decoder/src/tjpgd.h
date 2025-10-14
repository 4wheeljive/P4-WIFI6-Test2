/*----------------------------------------------------------------------------/
/ TJpgDec - Tiny JPEG Decompressor R0.03 include file         (C)ChaN, 2021
/----------------------------------------------------------------------------*/
#ifndef DEF_TJPGDEC
#define DEF_TJPGDEC

// Include standard library headers outside namespace to avoid conflicts
#include <string.h>
#if !defined(_WIN32)	/* Embedded platform */
#include <stdint.h>
#endif

namespace fl {
namespace third_party {

extern "C" {

#include "tjpgdcnf.h"

#if defined(_WIN32)	/* VC++ or some compiler without stdint.h */
typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef short			int16_t;
typedef unsigned long	uint32_t;
typedef long			int32_t;
#endif

#if JD_FASTDECODE >= 1
typedef int16_t jd_yuv_t;
#else
typedef uint8_t jd_yuv_t;
#endif


/* Error code */
typedef enum {
	JDR_OK = 0,	/* 0: Succeeded */
	JDR_INTR,	/* 1: Interrupted by output function */	
	JDR_INP,	/* 2: Device error or wrong termination of input stream */
	JDR_MEM1,	/* 3: Insufficient memory pool for the image */
	JDR_MEM2,	/* 4: Insufficient stream input buffer */
	JDR_PAR,	/* 5: Parameter error */
	JDR_FMT1,	/* 6: Data format error (may be broken data) */
	JDR_FMT2,	/* 7: Right format but not supported */
	JDR_FMT3	/* 8: Not supported JPEG standard */
} JRESULT;



/* Rectangular region in the output image */
typedef struct {
	uint16_t left;		/* Left end */
	uint16_t right;		/* Right end */
	uint16_t top;		/* Top end */
	uint16_t bottom;	/* Bottom end */
} JRECT;



/* Decompressor object structure */
typedef struct JDEC JDEC;
struct JDEC {
	size_t dctr;				/* Number of bytes available in the input buffer */
	uint8_t* dptr;				/* Current data read ptr */
	uint8_t* inbuf;				/* Bit stream input buffer */
	uint8_t dbit;				/* Number of bits availavble in wreg or reading bit mask */
	uint8_t scale;				/* Output scaling ratio */
	uint8_t msx, msy;			/* MCU size in unit of block (width, height) */
	uint8_t qtid[3];			/* Quantization table ID of each component, Y, Cb, Cr */
	uint8_t ncomp;				/* Number of color components 1:grayscale, 3:color */
	int16_t dcv[3];				/* Previous DC element of each component */
	uint16_t nrst;				/* Restart inverval */
	uint16_t width, height;		/* Size of the input image (pixel) */
	uint8_t* huffbits[2][2];	/* Huffman bit distribution tables [id][dcac] */
	uint16_t* huffcode[2][2];	/* Huffman code word tables [id][dcac] */
	uint8_t* huffdata[2][2];	/* Huffman decoded data tables [id][dcac] */
	int32_t* qttbl[4];			/* Dequantizer tables [id] */
#if JD_FASTDECODE >= 1
	uint32_t wreg;				/* Working shift register */
	uint8_t marker;				/* Detected marker (0:None) */
#if JD_FASTDECODE == 2
	uint8_t longofs[2][2];		/* Table offset of long code [id][dcac] */
	uint16_t* hufflut_ac[2];	/* Fast huffman decode tables for AC short code [id] */
	uint8_t* hufflut_dc[2];		/* Fast huffman decode tables for DC short code [id] */
#endif
#endif
	void* workbuf;				/* Working buffer for IDCT and RGB output */
	jd_yuv_t* mcubuf;			/* Working buffer for the MCU */
	void* pool;					/* Pointer to available memory pool */
	size_t sz_pool;				/* Size of momory pool (bytes available) */
	size_t (*infunc)(JDEC*, uint8_t*, size_t);	/* Pointer to jpeg stream input function */
	void* device;				/* Pointer to I/O device identifiler for the session */
	uint8_t swap;       /* Added by Bodmer to control byte swapping */
};

/* Extended decoder state for progressive processing */
struct JDEC_Progressive {
	JDEC base;					/* Original decoder state */

	/* Progressive state */
	uint16_t current_mcu_x;		/* Current MCU position X */
	uint16_t current_mcu_y;		/* Current MCU position Y */
	uint16_t mcus_processed;	/* MCUs completed so far */
	uint16_t total_mcus;		/* Total MCUs in image */

	/* Suspension/Resume support */
	uint8_t is_suspended;		/* Can be suspended between MCUs */
	uint8_t suspend_reason;		/* Why suspended (data/time/callback) */

	/* Buffer state preservation */
	size_t stream_position;		/* Current stream read position */
	uint8_t bit_buffer_state;	/* Partial bit decoding state */
	uint32_t partial_bits;		/* Bits waiting to be processed */

	/* Memory management */
	void* persistent_workspace;	/* Workspace that survives suspend/resume */
	uint8_t workspace_initialized; /* Track initialization state */
};



/* TJpgDec API functions */
JRESULT jd_prepare (JDEC* jd, size_t (*infunc)(JDEC*,uint8_t*,size_t), void* pool, size_t sz_pool, void* dev);
JRESULT jd_decomp (JDEC* jd, int (*outfunc)(JDEC*,void*,JRECT*), uint8_t scale);

/* Progressive decompression API */
enum { JDR_SUSPEND = 9 };  /* Suspended for progressive processing */

JRESULT jd_decomp_progressive(
    JDEC_Progressive* jpd,                     /* Progressive decoder state */
    int (*outfunc)(JDEC*, void*, JRECT*),      /* Output callback */
    uint8_t scale,                             /* Scaling factor */
    uint16_t max_mcus_per_call,                /* MCU processing limit per call */
    uint8_t* more_data_needed,                 /* Output: needs more input data */
    uint8_t* processing_complete               /* Output: decode finished */
);


} // extern "C"

} // namespace third_party
} // namespace fl

#endif /* _TJPGDEC */
