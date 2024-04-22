
/**
 * This file contains stubs so that we can enable the long file names option in FF without 
 * using the unicode
 */
#include "ff.h"
#include "diskio.h"

WCHAR ff_uni2oem (	/* Returns OEM code character, zero on error */
	DWORD	uni,	/* UTF-16 encoded character to be converted */
	WORD	cp		/* Code page for the conversion */
)
{
	WCHAR c = 0;
	if (uni < 0x80) {	/* ASCII? */
		c = (WCHAR)uni;
	}

	return c;
}

WCHAR ff_oem2uni (	/* Returns Unicode character in UTF-16, zero on error */
	WCHAR	oem,	/* OEM code to be converted */
	WORD	cp		/* Code page for the conversion */
)
{
	WCHAR c = 0;
	if (oem < 0x80) {	/* ASCII? */
		c = oem;
	} 
	return c;
}

WCHAR ff_convert (WCHAR wch, UINT dir) 
{ 
          if (wch < 0x80) { 
                    /* ASCII Char */ 
                    return wch; 
          }  

          /* I don't support unicode it is too big! */ 
          return 0; 
}  

DWORD ff_wtoupper (DWORD uni)
{ 
          if (uni < 0x80) {      
                    /* ASCII Char */ 
                    if (uni >= 'a' && uni <= 'z') { 
                              uni &= ~0x20; 
                     } 
                      return uni; 
          }  

          /* I don't support unicode it is too big! */ 
          return 0; 
} 

