
/*****************************************************************************/
/* import functions for keyboard handler (INPUT.ASM)                         */
/*****************************************************************************/

extern DWORD          flip2_KeyErrorcode;     /* dword - keyboard error code */
extern BOOL           FLIP2API flip2_KeyInit (void);
extern void           FLIP2API flip2_KeyDeinit (void);
extern BYTE           FLIP2API flip2_GetKey (void);
extern BYTE           FLIP2API flip2_GetLastKey (void);
extern BYTE           FLIP2API flip2_GetKeyAscii (void);
extern BYTE           FLIP2API flip2_ScanToAscii (BYTE scan_key);
extern BYTE *         flip2_KeyTable;
