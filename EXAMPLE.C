
#include  "example.h"                               /* Local definitions      */




/*
 *			  Start of Main function
 */


int cdecl main(VOID){

    HMQ   hmq;					    /* Message queue handle   */
    QMSG  qmsg; 				    /* Message data structure */
    ULONG lCreate;				    /* Frame creation flags   */
    SHORT cxScreen, cyScreen;			    /* Screen size variables  */

    DosError(0);

    vhab  =  WinInitialize(NULL);		    /* Create anchor handle   */

    hmq  =  WinCreateMsgQueue(vhab, 0); 	    /* Create message queue   */

    /* Register window class and procedure   */
    WinRegisterClass(vhab,
		     vszClass,
		     StatusWndProc,
		     CS_SYNCPAINT,
		     0);

    lCreate = FCF_TITLEBAR |  FCF_BORDER;	    /* Frame creation flags   */

    /* Create a standard main window with a title bar and border.  */
    vhwndFrame = WinCreateStdWindow( HWND_DESKTOP,
				    0L,
				    &lCreate,	    /* Pointer to flags       */
				    vszClass,	    /* Class name	      */
				    vszTitle,	    /* Window's Title         */
				    0L,
				    NULL,
				    0L,
				    &vhwndClient    /* Client handle pointer  */
				    );

    /* Query the vertical and horizontal screen size */
    cxScreen = (SHORT)WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
    cyScreen = (SHORT)WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);

    /* Size and position the status information window	 */
    WinSetWindowPos(
		    (HWND)vhwndFrame,
		    (HWND)HWND_TOP,
		    cxScreen / 4,
		    cyScreen / 2,
		    cxScreen / 2,
		    cyScreen / 4,
		    SWP_ACTIVATE | SWP_SIZE |
		    SWP_MOVE | SWP_HIDE
		    );

    /* Start of main message loop */
    while (WinGetMsg(vhab, &qmsg, NULL, 0, 0))
			  WinDispatchMsg(vhab, &qmsg);

    /* Termination and clean up */
    WinDestroyWindow(vhwndFrame);
    WinDestroyMsgQueue(hmq);
    WinTerminate(vhab);
    return 0;
}


/*
 *    Start of status information window procedure.
 */


MRESULT EXPENTRY StatusWndProc(HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2){

    static SHORT  cyDspPos,			  /* Status display variable  */
		  cxCharWidth;			  /* Character width variable */
    FONTMETRICS   fm;				  /* Font metrics structure   */
    HPS 	  hps;				  /* Presentation space handle*/
    POINTL	  ptl;				  /* Point structure	      */

    switch(msg){

       case WM_SIZE:
	    cyDspPos = SHORT2FROMMP(mp2) / 4;	  /* Get client window size.  */
	    return 0;


       case WM_CREATE:
	    hps = WinGetPS(hwnd);

	    /* Query current font information.	*/
	    GpiQueryFontMetrics(hps, (LONG)sizeof(fm), &fm);

	    /* Determine the average charactor width for this font. */
	    cxCharWidth = (SHORT)fm.lAveCharWidth;
	    WinReleasePS(hps);

	    /* Start the installation */
	    WinPostMsg(hwnd, WM_INSTALL, NULL, NULL);
	    return 0;



       case WM_PAINT:
	    hps = WinBeginPaint(hwnd, NULL, NULL);

	    GpiErase(hps);			  /* Clear the client window. */
	    ptl.y = cyDspPos * 3;		  /* Point at display position*/
	    ptl.x = cxCharWidth * 10;		  /* Set line starting point. */

	    /* Display type of operation  line. */
	    GpiCharStringAt(hps, &ptl,28L , "Installing Program and Files");

	    ptl.y = cyDspPos * 2;		  /* Point at display position*/
	    ptl.x = cxCharWidth * 3;		  /* Set line starting point. */

	    /* Display the first status line. */
	    GpiCharStringAt(hps, &ptl, 5L , "From:");

	    ptl.x = cxCharWidth * 12;		  /* Set new line position.   */
	    GpiCharStringAt(hps, &ptl, (LONG)strlen(vszCopyFrom),
								vszCopyFrom);

	    ptl.y = cyDspPos ;			 /* Point at display position.*/
	    ptl.x = cxCharWidth * 3;		 /* Set line starting point.  */

	    /* Display the second status line. */
	    GpiCharStringAt(hps, &ptl, 3L, "To:");

	    ptl.x = cxCharWidth * 12;		 /* Set new line position.    */
	    GpiCharStringAt(hps, &ptl, (LONG)strlen(vszCopyTo), vszCopyTo);
	    WinEndPaint(hps);
	    return 0;

       case WM_INSTALL:

	    /* Start installation by creating a Directory    */
	    WinDlgBox( HWND_DESKTOP  ,
		       vhwndFrame    ,
		       CrtDirDlgProc ,
		       NULL	     ,
		       IDDLG_CRTDIR  ,
		       NULL);
	    return 0;

       default:
	    return WinDefWindowProc(hwnd, msg, mp1, mp2);
    }
}



/*
 *   ControlThread function runs as an independant thread. By running on it's
 *   own thread of execution, long copy operations may then be started without
 *   violation of the 1/10 second rule.
 */


VOID ControlThread(VOID){

    HPROGRAM hGroup;				    /* Group handle	      */
    SHORT    i = 0;				    /* Array index variable   */
    SHORT    rc;				    /* Return code variable   */
    HAB      hab;				    /* Anchor bar handle      */
    HMQ      hmq;				    /* Message queue handle   */


    hab  =  WinInitialize(NULL);		    /* Create anchor handle   */
    hmq = WinCreateMsgQueue(hab, 0);		    /* Create message queue   */




    /* Start of the copy loop. */

       do{

	  /* Update the copy status line information for display   */
	  /* in the status window's client area.                   */

	     strcpy(vszCopyFrom,vszCmdData[i]);     /* Copy from information  */
	     strcpy(vszCopyTo,vszProgInfo[4]);
	     strcat(vszCopyTo,vszCmdData[i+1]);     /* Copy to information    */

	  /* Generate a WM_PAINT message to update the copy status.*/
	     WinInvalidateRect(vhwndClient, NULL,FALSE);

#ifdef PMREL1_1 				    /* Using OS/2 1.1 toolkit */

	  /* Call a local function to handle the copying of the    */
	  /* program and files. 				   */

	     rc = CopyFile(vszCopyFrom,vszCopyTo);
#else
	  /* When using the OS/2 1.2 toolkit call the DosCopy	   */
	  /* OS/2 function.  The DCPY_EXISTING parameter means	   */
	  /* if target exists replace it			   */

	     rc = DosCopy(vszCopyFrom,vszCopyTo, DCPY_EXISTING, 0L);

#endif /* PMREL1_1 */

	  /* In this example, we are only checking for diskette    */
	  /* not ready. On a not ready condition we will display   */
	  /* a message with retry. All other copy errors will	   */
	  /* display a generic message with cancel only option.    */

	     if (rc)
		  if(rc == ERROR_NOT_READY)
			rc = DisplayMsg( MSG005, MB_RETRYCANCEL | MB_ICONHAND);
		  else
			rc = DisplayMsg( MSG002, MB_CANCEL | MB_ICONHAND);
	      else
		  i+=2;

       }while((i < BUFFSIZE) && (rc == 0 || rc == MBID_RETRY));

	   /* Continue the installation if program and files were  */
	   /* copied without error.				   */

       if(!rc){

	   /* Create the "Example" group.   WinCreateGroup will    */
	   /* return an existing group handle or create a new one  */
	   /* if it doesn't exist.                                 */

	     hGroup = WinCreateGroup(vhab,  vszProgInfo[0],
						      SHE_VISIBLE, 0L, 0L);

	     if (hGroup){

	   /* Calling  a local function "AddProgram"  to add       */
	   /* programinformation to our new group.		   */

		 rc = AddProgram( vszProgInfo[1], vszCopyTo,
				      vszProgInfo[3] , vszProgInfo[4], hGroup);

	   /* In this example, we are only checking for duplicate  */
	   /* program titles in the group. Any other error return  */
	   /* code will display a generic message.		   */

		 if(rc)
		     if(rc == PMERR_DUPLICATE_TITLE)
			  DisplayMsg(  MSG006, MB_CANCEL | MB_ICONHAND);
		     else
			  DisplayMsg(  MSG004, MB_CANCEL | MB_ICONHAND);

		 else{

		     /* Hide the status window */
		     WinShowWindow(vhwndFrame, FALSE);

		     /* If we are here, the installation completed  */
		     /* without error. Display a message to tell    */
		     /* the user.				    */

		     DisplayMsg(  MSG007, MB_OK | MB_ICONASTERISK);
		 }

	     }else
		 /* Group creation error message. */
		 DisplayMsg(  MSG003, MB_CANCEL | MB_ICONHAND);

       }

    /* Post a WM_QUIT message to the message queue to end the  */
    /* installation program.				       */
    WinPostMsg( vhwndClient, WM_QUIT, NULL, NULL );


    /* Termination and clean up */
    WinDestroyMsgQueue( hmq );			    /* Destroy message queue  */
    WinTerminate(hab);				    /* Destroy handle	      */
    DosExit( EXIT_THREAD, 0 );			    /* End thread execution.  */
}


/*
 *   CopyFile function is only needed if using OS/2 release 1.1 toolkit. Since
 *   OS/2 has provided the DosCopy function for release 1.2, this function will
 *   no longer be needed.
 */


BOOL CopyFile(PSZ pszSrc, PSZ pszTarget){

    #define WORKBUFFER	4096			  /* Copy data buffer	      */

    HFILE   hSrcfile ;				  /* Source file handle       */
    HFILE   hTargetfile ;			  /* Target file handle       */
    USHORT  uAction ;				  /* Open action information  */
    USHORT  rc ;				  /* Return code	      */
    USHORT  cbytesRead ;			  /* Count of bytes read      */
    USHORT  cbWritten ; 			  /* Count of bytes written   */
    FILESTATUS fStatus ;			  /* File Status buffer       */
    SEL selAddress ;				  /* Alloc. segment selector  */

    /* Opening the source file, read only, none share.		*/
    if(!(rc = DosOpen(pszSrc, &hSrcfile, &uAction, 0L, 0,
						    0x0001, 0x20A0, 0L))){

       /* Get source file information creation date, time, etc...  */
       DosQFileInfo(hSrcfile, 1, (PBYTE)&fStatus, sizeof(FILESTATUS));

       /* Opening target file replace or create mode none share.   */
       if (!(rc = DosOpen(pszTarget, &hTargetfile, &uAction,
		     fStatus.cbFile, fStatus.attrFile, 0x0012, 0x2091, 0L))){

	  /* Allocate copy buffer space.			   */
	  if (!(rc = DosAllocSeg(WORKBUFFER, &selAddress, 0))){

	     /* Start copy file to file loop.			   */
	     do{

		/* Reading a buffer of data from source file.	    */
		rc = DosRead(hSrcfile, MAKEP(selAddress,0),
						       WORKBUFFER, &cbytesRead);

		/* Write the buffer to the target file. 	    */
		if(!rc)
		   rc = DosWrite(hTargetfile, MAKEP(selAddress,0),
						       cbytesRead, &cbWritten);


	     }while (cbytesRead && rc == 0);

	     /* Check if any error occured during the copy operation.*/

	     if (rc)

	     /* The copy operation failed. Remove the incomplete    */
	     /* target file.					    */

		DosDelete(pszTarget, 0L);
	     else

	     /* The copy operation was successful. Change the target */
	     /* file's status information, creation date, time, to   */
	     /* match the source file's.                             */

		DosSetFileInfo(hTargetfile, 1, (PBYTE)&fStatus,
						       sizeof(FILESTATUS)) ;

	     DosClose(hTargetfile);		  /* Close the target file    */
	     DosFreeSeg(selAddress);		  /* Free copy buffer space   */
	  }
       }
       DosClose(hSrcfile);			  /* Close the source file    */
    }

    /* Return a completion code to the calling function.	   */
    return(rc);
}


/*
 *   AddProgram function adds program information to a group. The
 *   function adds a title, program name and path, any parameters,
 *   and working directory information.
 */


SHORT AddProgram( PSZ pszTitle, PSZ pszNamePath, PCH pszParameters,
				   PSZ pszWorkDir, HPROGRAM hGroup){

    PIBSTRUCT  pib;				  /* Program information block*/
    HPROGRAM   hprog;				  /* Program handle	      */

    pib.pchProgramParameter = pszParameters;	  /* Program's parameters     */
    pib.cchProgramParameter = strlen(pszParameters)+1;

    pib.pchEnvironmentVars = NULL;		  /* No environment string    */
    pib.cchEnvironmentVars = 0;
    pib.progt.progc = PROG_PM;			  /* Installing a PM program. */
    pib.progt.fbVisible = SHE_VISIBLE;		  /* Make entry visible.      */
    pib.szIconFileName[0] = NULL;		  /* Use default program icon.*/
    pib.xywinInitial.x = 0;			  /* Default window position  */
    pib.xywinInitial.y = 0;			  /* and size.		      */
    pib.xywinInitial.cx = 0;
    pib.xywinInitial.cy = 0;
    pib.xywinInitial.fsWindow = XYF_NORMAL;	  /* Normal visible window.   */
    pib.res1 = 0;				  /* Reserved must be zero.   */
    pib.res2 = 0;				  /* Reserved must be zero.   */

    strcpy(pib.szTitle, pszTitle);		  /* Program title.	      */
    strcpy(pib.szExecutable, pszNamePath);	  /* Executable's name/path.  */
    strcpy(pib.szStartupDir, pszWorkDir);	  /* Current directory.       */

    /* Add the new program entry. */
    hprog = WinAddProgram(vhab, (PPIBSTRUCT)&pib, hGroup);

    if (hprog == NULL)

	   /* On an error, return the error code. */
	   return ((SHORT)WinGetLastError(vhab));
    else
	   return 0;				  /* Completed without error. */
}


/*
 *    DisplayMsg controls all message display. The function uses the
 *    passed message ID to find and display the requested message.
 *    DisplayMsg returns the user's message response to the calling
 *    procedure.
 */


USHORT	DisplayMsg(  USHORT msgid, SHORT style){

    CHAR     szMsgStr[MAXMSGSIZE];		 /* Message string buffer.    */

    /* Load message buffer with message text.  */
    WinLoadString(vhab, NULL, msgid, sizeof szMsgStr, szMsgStr) ;


    /* Display message and return response to calling procedure. */
    return(WinMessageBox(HWND_DESKTOP, vhwndClient, szMsgStr,
			(PSZ)vszTitle, NULL, style | MB_HELP |
					      MB_MOVEABLE ));

}


/*
 *   CrtDirDlgProc is the window procedure for the create directory dialog
 *   box. This is a example of using a dialog box to give experienced users
 *   the option to change the installation defaults.
 */


MRESULT APIENTRY CrtDirDlgProc(HWND hdlg, USHORT msg,
					  MPARAM MParam1, MPARAM MParam2){

    SHORT    cch;				   /*  String char. count     */
    SHORT    rc;				   /*  Return code variable   */
    SHORT   idThread;				   /*  Thread ID variable     */

    switch (msg) {

    case WM_INITDLG:

	/* Remove greyed System menu items */
	RemoveSysItems(hdlg);

	/* Set the dialog box input field size limit. */
	WinSendDlgItemMsg(hdlg, IDC_DIRPATH, EM_SETTEXTLIMIT,
			  MPFROMSHORT(MAXSTRSIZE), 0L);

	/* Display default directory path in input field. */
	WinSetWindowText(WinWindowFromID(hdlg, IDC_DIRPATH), vszProgInfo[4]);

	break;	/* End WM_INITDLG */

    case WM_COMMAND:
	switch (LOUSHORT(MParam1)) {

	case IDC_OK:

	    /* Query the input field for the size of the path string. */
	    cch = WinQueryWindowTextLength( WinWindowFromID( hdlg
					  , IDC_DIRPATH));

	    /* Read the new path string */
	    WinQueryWindowText( WinWindowFromID( hdlg, IDC_DIRPATH)
			      , cch + 1, vszProgInfo[4] );

	    /* Convert path string to upper case */
	    strupr(vszProgInfo[4]);

		     /* Create directory  */
	    rc = DosMkDir((PSZ)vszProgInfo[4], (ULONG)0);


	 /* Check if directory creation completed without error.  If the */
	 /* directory already existed before the call, OS/2 will return  */
	 /* an ERROR_ACCESS_DENIED code.   In this case we aren't        */
	 /* considering this as an error.				 */

	    if ( (rc == 0) || (rc == ERROR_ACCESS_DENIED ) ){

	       /* Start the installation copy thread running. */
		DosCreateThread( ControlThread, &idThread,
				       (PBYTE)&vStack[STACK_SIZE-2] );

		/* Show the status window */
		WinShowWindow(vhwndFrame, TRUE);

		/* Remove the dialog window. */
		WinDismissDlg( hdlg, TRUE );
	    }else{

	       /* We got an error creating the directory.  */
	       /* Display a message to tell the user.	   */
		if(rc == ERROR_PATH_NOT_FOUND)

		   rc = DisplayMsg(  MSG009, MB_RETRYCANCEL | MB_ICONHAND);
		else
		   rc = DisplayMsg(  MSG001, MB_CANCEL | MB_ICONHAND);


		if(rc == MBID_CANCEL){

		   /* Post a WM_QUIT message to the message queue to end the  */
		   /* installation program.				      */
		    WinPostMsg( vhwndClient, WM_QUIT, NULL, NULL );

		   /* Remove the dialog window. */
		    WinDismissDlg( hdlg, FALSE );

		 }else

		    /* Set focus on input field and let the user try again.*/
		     WinSetFocus(HWND_DESKTOP, WinWindowFromID(hdlg,
								 IDC_DIRPATH));
	    }
	    break;   /* End IDC_OK Case */

	case IDC_CANCEL:

	    /* The user has clicked on the cancel button. Display a  */
	    /* terminate confirmation message.			     */

	    switch( DisplayMsg( MSG008, MB_YESNO | MB_ICONQUESTION )){

		  case MBID_YES:

		     /* Remove dialog box and terminate the program. */
		      WinDismissDlg( hdlg, FALSE );
		      WinPostMsg( vhwndClient, WM_QUIT, NULL, NULL );
		      break;

		  case MBID_NO:

		    /* Set focus on input field and let the user try again.*/
		  WinSetFocus(HWND_DESKTOP, WinWindowFromID(hdlg,
							     IDC_DIRPATH));
	    }
	    break;  /* End IDC_CANCEL Case */

	}
	break; /* End WM_COMMAND Case */

    default:
	return WinDefDlgProc(hdlg, msg, MParam1, MParam2);
	break;
    }

    return 0L;
}

/*
 *   Since our dialog box can't be resized, minimized, maximized, or restored,
 *   RemoveSysItems deletes these greyed items from the System menu. This is
 *   in conformation with CUA.
 */


VOID RemoveSysItems(HWND hdlg){

#define ITEMS 4
SHORT i;
HWND  hSysMenu; 				   /* System menu's handle    */
static SHORT items[] =				   /* Items to remove	      */
		       {
			  SC_RESTORE  ,
			  SC_SIZE     ,
			  SC_MINIMIZE ,
			  SC_MAXIMIZE
			};

    /* Get the handle of the dialog's system menu. */
    hSysMenu = WinWindowFromID(hdlg, FID_SYSMENU);

    /* Send messages to system menu to delete the items. */
    for(i = 0; i < ITEMS; ++i)
	WinSendMsg(hSysMenu, MM_DELETEITEM, MPFROM2SHORT(items[i],
							    TRUE), NULL);
}
