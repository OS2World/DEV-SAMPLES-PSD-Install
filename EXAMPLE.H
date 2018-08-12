
/*
 *                      Example Install Include File
 */


/*
 *                              Includes
 */


#define   PMREL1_1                                  /* Defined Rel 1.1 only   */
#define   INCL_WIN                                  /* Window definitions     */
#define   INCL_GPI                                  /* GPI definitions        */
#define   INCL_ERRORS                               /* Error code definitions */
#define   INCL_DOSMISC                              /* 1.2 DosError prototype */
#define   INCL_DOSQUEUES                            /* 1.1 DosError prototype */
#define   INCL_DOSPROCESS                           /* Dos thread definitions */

#include  <os2.h>                                   /* OS/2 definitions       */
#include  <string.h>                                /* C string definitions   */


/*
 *                          Local Prototypes
 */


VOID    ControlThread(VOID);
BOOL    CopyFile(PSZ, PSZ) ;
SHORT   AddProgram(PSZ, PSZ, PCH, PSZ, HPROGRAM) ;
MRESULT EXPENTRY StatusWndProc( HWND, USHORT, MPARAM, MPARAM) ;
MRESULT APIENTRY CrtDirDlgProc(HWND, USHORT, MPARAM, MPARAM);
USHORT  DisplayMsg(  USHORT, SHORT);
VOID    RemoveSysItems(HWND);


/*
 *                             Defines
 */

                /* String table keyword defines  */

#define   MSG001         1
#define   MSG002         2
#define   MSG003         3
#define   MSG004         4
#define   MSG005         5
#define   MSG006         6
#define   MSG007         7
#define   MSG008         8
#define   MSG009         9


#define   WM_INSTALL     WM_USER                    /* Start install message  */
#define   MAXSTRSIZE     150                        /* Maximum string size    */
#define   MAXMSGSIZE     100                        /* Maximum message size   */
#define   BUFFSIZE       6                          /* Copy data buffer size  */
#define   INFOSIZE       5                          /* Group data buffer size */
#define   STACK_SIZE     2000                       /* Thread stack size      */
#define   IDC_OK         1                          /* Dialog box OK response */
#define   IDC_CANCEL     2                          /* Dialog box cancel      */
#define   IDC_HELP       3                          /* Dialog box HELP        */
#define   IDDLG_CRTDIR   10                         /* Dialog box ID          */
#define   IDC_DIRPATH    11                         /* Dialog box input field */


/*
 *                            Variables
 */


HAB  vhab ;                                         /* Anchor bar handle      */
HWND vhwndClient ;                                  /* Client window  handle  */
HWND vhwndFrame ;                                   /* Window frame handle    */
CHAR vszCopyFrom[MAXSTRSIZE];                       /* Copy from status string*/
CHAR vszCopyTo[MAXSTRSIZE];                         /* Copy to status string  */
CHAR vszClass[] = "SampleInstall" ;                 /* Class name             */
CHAR vszTitle[] = "Application Installation" ;      /* Window title           */
UCHAR vStack[STACK_SIZE];                           /* Control thread stack   */

           /* Group program information data buffer.  */
CHAR vszProgInfo[INFOSIZE][MAXSTRSIZE] = {
                                         "Example Group"    ,
                                         "Sample Program"   ,
                                         "\\SAMPLE.EXE"     ,
                                         "/r /b "           ,
                                         "C:\\SAMPLE"
                                         };

           /* Copy information data buffer.  */
CHAR vszCmdData[BUFFSIZE][MAXSTRSIZE] = {
                                        "A:\\DATAFILE.FIL"  ,
                                        "\\DATAFILE.FIL"    ,
                                        "A:\\HELPFILE.HLP"  ,
                                        "\\HELPFILE.HLP"    ,
                                        "A:\\SAMPLE.EXE"    ,
                                        "\\SAMPLE.EXE"
                                        };
