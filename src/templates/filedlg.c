/*
 ** $Id:$
 ** 
 ** filedlg.c: File Open & Saveas Dialog.
 ** 
 ** Copyright (C) 2004 ~ 2008 Feynman Software.
 **
 ** Current maintainer: 
 **
 ** Create date: 2008/04/08
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#ifdef __LINUX__
#include <sys/time.h>
#include <unistd.h>
#include <pwd.h>
#elif defined(WIN32)
#define R_OK    4       /* Test for read permission.  */
#define W_OK    2       /* Test for write permission.  */
#define X_OK    1       /* Test for execute permission.  */
#define F_OK    0       /* Test for existence.  */
#include <io.h>
#endif
#include <errno.h>

#include "mgutils.h"

#define MIN_WIDTH       490
#define OTHERS_WIDTH    330
#define COL_COUNT       4

static HICON icon_ft_dir, icon_ft_file;

static char* head_text[] = {"Name", "Size", "Access Mode", "Last Modify Time"};

static CTRLDATA DefFileCtrl [] =
{
    { "static", WS_VISIBLE | SS_LEFT,
        7, 7, 70, 25,
        IDC_FOSD_PATH_NOTE, "Find in",  0, WS_EX_TRANSPARENT | WS_EX_USEPARENTRDR},

    { "combobox", WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_NOTIFY 
        | CBS_READONLY | WS_BORDER | CBS_EDITNOBORDER,
        84, 5, 160, 25,
        IDC_FOSD_PATH, NULL, 120, 0 },

    { "button", WS_VISIBLE |WS_TABSTOP ,
        252, 5, 58, 25,
        IDC_FOSD_UPPER, "Up", 0, WS_EX_USEPARENTRDR },

    { "listview", WS_VISIBLE | WS_CHILD |WS_TABSTOP |WS_BORDER |WS_VSCROLL 
        |WS_HSCROLL |LVS_SORT |LVS_NOTIFY,
        7, 35, 303, 85,
        IDC_FOSD_FILELIST, "File List", 0, 0 },

    { "static", WS_VISIBLE |SS_LEFT,
        7, 127, 70, 25,
        IDC_FOSD_FILENAME_NOTE, "File Name",  0, WS_EX_TRANSPARENT | WS_EX_USEPARENTRDR },

    { "sledit", WS_VISIBLE |WS_TABSTOP |WS_BORDER,
        84, 125, 226, 25,
        IDC_FOSD_FILENAME, NULL, 0, 0 },

    { "static", WS_VISIBLE |SS_LEFT,
        7, 157, 70, 25,
        IDC_FOSD_FILETYPE_NOTE, "File Type",  0, WS_EX_TRANSPARENT | WS_EX_USEPARENTRDR },

    { "combobox", WS_VISIBLE |WS_TABSTOP |CBS_DROPDOWNLIST |CBS_NOTIFY 
        |CBS_READONLY | WS_BORDER | CBS_EDITNOBORDER,
        84, 155, 226, 25,
        IDC_FOSD_FILETYPE, NULL, 120, 0 },

    { "button", WS_VISIBLE |WS_TABSTOP |BS_AUTOCHECKBOX,
        7, 185, 100, 25,
        IDC_FOSD_ISHIDE, "Hide File", 0, WS_EX_TRANSPARENT | WS_EX_USEPARENTRDR },

    { "button", WS_VISIBLE |WS_TABSTOP ,
        130, 185, 85, 25,
        IDC_FOSD_OK, "OK", 0, WS_EX_USEPARENTRDR },

    { "button", WS_VISIBLE |WS_TABSTOP ,
        225, 185, 85, 25,
        IDC_FOSD_CANCEL, "Cancel", 0, WS_EX_USEPARENTRDR }
};

static CTRLDATA DefSimpleFileCtrl [] =
{
    { "combobox", WS_VISIBLE |WS_TABSTOP |CBS_DROPDOWNLIST |CBS_NOTIFY 
        |CBS_READONLY | WS_BORDER | CBS_EDITNOBORDER,
        7, 5, 303, 25,
        IDC_FOSD_PATH, NULL, 120, WS_EX_USEPARENTRDR },

    { "listview", WS_VISIBLE |WS_CHILD |WS_TABSTOP |WS_BORDER |WS_VSCROLL 
        |WS_HSCROLL |LVS_SORT |LVS_NOTIFY,
        7, 35, 303, 115,
        IDC_FOSD_FILELIST, "File List", 0, WS_EX_USEPARENTRDR },

    { "static", WS_VISIBLE |SS_LEFT,
        7, 155, 70, 25,
        IDC_FOSD_FILENAME_NOTE, "File Name",  0, WS_EX_TRANSPARENT | WS_EX_USEPARENTRDR },

    { "sledit", WS_VISIBLE |WS_TABSTOP |WS_BORDER,
        84, 155, 70, 25,
        IDC_FOSD_FILENAME, NULL, 0, 0 },

    { "static", WS_VISIBLE |SS_LEFT,
        160, 155, 70, 25,
        IDC_FOSD_FILETYPE_NOTE, "File Type",  0, WS_EX_TRANSPARENT | WS_EX_USEPARENTRDR },

    { "combobox", WS_VISIBLE |WS_TABSTOP |CBS_DROPDOWNLIST |CBS_NOTIFY 
        |CBS_READONLY | WS_BORDER | CBS_EDITNOBORDER,
        235, 155, 75, 25,
        IDC_FOSD_FILETYPE, NULL, 120, WS_EX_USEPARENTRDR },

    { "button", WS_VISIBLE |WS_TABSTOP ,
        7, 185, 140, 25,
        IDC_FOSD_OK, "OK", 0, WS_EX_USEPARENTRDR },

    { "button", WS_VISIBLE |WS_TABSTOP ,
        170, 185, 140, 25,
        IDC_FOSD_CANCEL, "Cancel", 0, WS_EX_USEPARENTRDR }
};

DLGTEMPLATE DefFileDlg =
{
    WS_DLGFRAME | WS_BORDER | WS_CAPTION, 
    WS_EX_USEPARENTRDR,
    0, 0, 320, 240, 
    "File", 0, 0, 
    TABLESIZE (DefFileCtrl), 
    DefFileCtrl 
};

DLGTEMPLATE DefSimpleFileDlg=
{
    WS_DLGFRAME | WS_BORDER | WS_CAPTION, 
    WS_EX_NONE,
    0, 0, 320, 240, 
    "File", 0, 0, 
    TABLESIZE (DefSimpleFileCtrl), 
    DefSimpleFileCtrl 
};

typedef struct _FILEINFO
{
    char filename[MY_NAMEMAX+1];
    int  filesize;
    int  accessmode;
    time_t modifytime;
    BOOL IsDir;

} FILEINFO; 

typedef FILEINFO* PFILEINFO;

static int ColWidth [COL_COUNT] =
{
    60, 40, 75, 90
};

static BOOL IsInFilter (char *filtstr, char *filename)
{
    char chFilter[MAX_FILTER_LEN+1];
    char chTemp[255];
    char chFileExt[255];
    char *p1;
    char *p2;

    if (filename == NULL)
        return FALSE;
    if (filtstr == NULL)
        return TRUE;
   
    strtrimall (filename);
    strtrimall (filtstr);
    if (filename [0] == '\0')
        return FALSE;
    if (strlen (filtstr) == 0)
        return TRUE;
    
    p1 = strchr (filename, '.');
    p2 = NULL;
    while (p1 != NULL) {
        p2 = p1;
        p1 = strchr (p2 + 1, '.');
    }

    if (p2 != NULL) {
        strcpy (chFileExt, "*");
        strcpy (chFileExt + 1, p2);
    }
    else 
        strcpy (chFileExt, "*.*");
    
    p1 = strchr (filtstr, '(');
    p2 = strchr (filtstr, ')');

    if (p1 == NULL || p2 == NULL)
        return FALSE;

    memset (chFilter, 0, sizeof (chFilter)); 
    strncpy (chFilter, p1 + 1, p2 - p1 - 1); 

    p1 = strchr (chFilter, ';');
    p2 = chFilter;
    while ( p1 != NULL ) {
        strncpy (chTemp, p2, p1 - p2);
        strtrimall (chTemp);
        if ( strcmp (chTemp, "*.*") == 0 || strcmp (chTemp, chFileExt) == 0 ) 
            return TRUE;
        p2 = p1 + 1;
        p1 = strchr (p2, ';');
    }

    strcpy (chTemp, p2);
    strtrimall (chTemp);
    if ( strcmp (chTemp, "*.*") == 0 || strcmp (chTemp, chFileExt) == 0)
        return TRUE;

    return FALSE;

}

static char* GetParentDir (char *dir)
{
    int i, nParent = 0;
    
    for (i = strlen(dir)-1; i >= 0; i--)
	{
#ifdef WIN32
		if(dir [i] == '\\' || dir[i] == '/')
#else
       if (dir [i] == '/') 
#endif
	   {
            nParent = i;
			break;
	   }
	}

    if (nParent == 0)
        dir [nParent + 1] = '\0';
    else 
        dir [nParent] = '\0';

#ifdef WIN32
	if(dir[nParent-1] == ':'){
		if(i == nParent){
			dir[0] = '/';
			dir[1] = 0;
		}
		else {
			dir[nParent ++] = '\\';
			dir[nParent ] = '\0';
		}
	}
#endif

    return dir;
}

static int GetAccessMode (HWND hWnd, char * dir, BOOL IsSave, BOOL IsDisplay)
{
    char msg[255];
    int  nResult=0;

    memset (msg, 0, sizeof (msg));

    if (access (dir, F_OK) == -1){
        sprintf (msg, GetSysText(IDS_MGST_SHOWHIDEFILE), dir);
        nResult = -1;
    } 
    else {
        if ( access (dir, R_OK) == -1) {
            sprintf (msg, GetSysText(IDS_MGST_NR), dir);
            nResult = -2;
        } 
        else  if ( IsSave == TRUE && access (dir, W_OK) == -1) {
            sprintf (msg, GetSysText(IDS_MGST_NW), dir);
            nResult = -3;
        } 
    }
     
    if (IsDisplay && nResult != 0)
       MessageBox (hWnd, msg, GetSysText(IDS_MGST_INFO), 
           MB_OK | MB_ICONSTOP| MB_BASEDONPARENT);
    
    return nResult;
}

/*
 * If file has been exist or no write access, return non-zero. 
 * Otherwise return zero.
 * */
static int FileExistDlg (HWND hWnd, char *dir, char *filename)
{
    char msg[255];
    memset (msg, 0, sizeof (msg));

    if (access (dir, F_OK) == 0){
        sprintf (msg, GetSysText(IDS_MGST_FILEEXIST), filename);
        if (MessageBox (hWnd, msg, GetSysText(IDS_MGST_INFO), 
                MB_OKCANCEL | MB_ICONSTOP| MB_BASEDONPARENT) == IDOK)
        {
            if (access (dir, W_OK) == -1) { /*no write access*/
                sprintf (msg, GetSysText(IDS_MGST_NR), filename);

                MessageBox (hWnd, msg, GetSysText(IDS_MGST_INFO), 
                    MB_OK | MB_ICONSTOP| MB_BASEDONPARENT);
                return -1; 
            }
            else { /*want to replace*/
                return 0; 
            }
        }
        else /*no replace*/
            return -2; 
    }
    /*file doesn't exist*/
    return 0; 
}

static void InsertToListView( HWND hWnd, PFILEINFO pfi)
{
    HWND      hListView;
    int       nItemCount;
    LVSUBITEM subdata;
    LVITEM    item;
    char      chTemp[255];
    struct tm *ptm;
    
    hListView = GetDlgItem (hWnd, IDC_FOSD_FILELIST);
    nItemCount = SendMessage (hListView, LVM_GETITEMCOUNT, 0, 0);
    
    item.nItem = nItemCount;
    item.itemData = (DWORD)pfi->IsDir;
    item.nItemHeight = 24;
    SendMessage (hListView, LVM_ADDITEM, 0, (LPARAM)&item);

    subdata.nItem = nItemCount;
    subdata.nTextColor = 0;
    subdata.flags = LVFLAG_ICON;
    
    if (pfi->IsDir)
        subdata.image = (DWORD)icon_ft_dir;
    else
        subdata.image = (DWORD)icon_ft_file;

    subdata.subItem = 0;
    subdata.pszText = (char *)malloc (MY_NAMEMAX+1);
    if ( subdata.pszText == NULL) 
        return ;

    strcpy (subdata.pszText, pfi->filename);
    SendMessage (hListView, LVM_SETSUBITEM, 0, (LPARAM)&subdata);

    subdata.flags = 0;
    subdata.image = 0;
    subdata.subItem = 1;
    sprintf(chTemp, " %d", pfi->filesize);
    strcpy(subdata.pszText, chTemp);
    SendMessage (hListView, LVM_SETSUBITEM, 0, (LPARAM)&subdata);

    subdata.subItem = 2;
    strcpy (subdata.pszText, "");
    switch (pfi->accessmode) {
    case 1:
        strcpy (subdata.pszText, "R");
        break;
    case 2:
        strcpy (subdata.pszText, "W");
        break;
    case 3:
        strcpy (subdata.pszText, "RW");
        break;
    }
    SendMessage (hListView, LVM_SETSUBITEM, 0, (LPARAM)&subdata);

    subdata.subItem = 3;
    ptm = (struct tm *)localtime (&(pfi->modifytime));

	if(ptm )
	{
		sprintf (subdata.pszText, "%d-%.2d-%.2d",
			ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);

		sprintf (subdata.pszText + 10, " %.2d:%.2d:%.2d",
			ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	}
	else
	{
		sprintf(subdata.pszText, "-------");
	}

    SendMessage (hListView, LVM_SETSUBITEM, 0, (LPARAM)&subdata);

    if (subdata.pszText != NULL) free (subdata.pszText);
}

static int 
ListViewSortBySize( HLVITEM nItem1, HLVITEM nItem2, PLVSORTDATA sortdata)
{
    HWND       hCtrlWnd;
    LVSUBITEM  subItem1, subItem2;
    int        nIsDir1, nIsDir2;
    int        nSize1, nSize2;

    hCtrlWnd = sortdata->hLV;

    nIsDir1 = SendMessage (hCtrlWnd, LVM_GETITEMADDDATA, 0, nItem1);
    nIsDir2 = SendMessage (hCtrlWnd, LVM_GETITEMADDDATA, 0, nItem2);

    if ( nIsDir1 >nIsDir2 )
        return 1;

    if (nIsDir1 < nIsDir2)
        return -1;

    //subItem1.nItem = nItem1;
    subItem1.subItem = 1;
    subItem1.pszText = (char *)malloc (MY_NAMEMAX+1);
    if ( subItem1.pszText == NULL) 
        return 0;
    SendMessage (hCtrlWnd, LVM_GETSUBITEMTEXT, 
            (WPARAM)nItem1, (LPARAM)&subItem1);

    //subItem2.nItem = nItem2;
    subItem2.subItem = 1;
    subItem2.pszText = (char *)malloc (MY_NAMEMAX+1);
    if (subItem2.pszText == NULL) {
        if (subItem1.pszText != NULL) free (subItem1.pszText);
        return 0;
    }

    SendMessage (hCtrlWnd, LVM_GETSUBITEMTEXT, 
            (WPARAM)nItem2, (LPARAM)&subItem2);

    nSize1 = atoi (subItem1.pszText);
    nSize2 = atoi (subItem2.pszText);

    if (subItem1.pszText) free (subItem1.pszText);
    if (subItem2.pszText) free (subItem2.pszText);
    
    if ( nSize1 >nSize2 )
        return 1;
    else if (nSize1 <nSize2)
        return -1;

    return 0;
}

static int 
ListViewSortByName( HLVITEM nItem1, HLVITEM nItem2, PLVSORTDATA sortdata)
{
    HWND       hCtrlWnd;
    LVSUBITEM  subItem1, subItem2;
    int        nIsDir1, nIsDir2;
    int        nResult;
    
    hCtrlWnd = sortdata->hLV;
    
    nIsDir1 = SendMessage (hCtrlWnd, LVM_GETITEMADDDATA, 0, nItem1);
    nIsDir2 = SendMessage (hCtrlWnd, LVM_GETITEMADDDATA, 0, nItem2);
   
    if ( nIsDir1 >nIsDir2 ) 
        return 1;
    
    if (nIsDir1 < nIsDir2)
        return -1;
   
    //subItem1.nItem = nItem1;
    subItem1.subItem = 0;
    subItem1.pszText = (char *)malloc (MY_NAMEMAX+1);
    if (subItem1.pszText == NULL) 
        return 0;

    SendMessage (hCtrlWnd, LVM_GETSUBITEMTEXT, 
            (WPARAM)nItem1, (LPARAM)&subItem1);

    //subItem2.nItem = nItem2;
    subItem2.subItem = 0;
    subItem2.pszText = (char *)malloc (MY_NAMEMAX+1);
    if (subItem2.pszText == NULL) {
        if (subItem1.pszText != NULL) free (subItem1.pszText);
        return 0;
    }

    SendMessage (hCtrlWnd, LVM_GETSUBITEMTEXT, 
            (WPARAM)nItem2, (LPARAM)&subItem2);

    nResult =  strcmp (subItem1.pszText, subItem2.pszText);

    if (subItem1.pszText != NULL) free (subItem1.pszText);
    if (subItem2.pszText != NULL) free (subItem2.pszText);

    return nResult;
}

static int 
ListViewSortByDate( HLVITEM nItem1, HLVITEM nItem2, PLVSORTDATA sortdata)
{
    HWND       hCtrlWnd;
    LVSUBITEM  subItem1, subItem2;
    int        nIsDir1, nIsDir2;
    int        nResult;
    
    hCtrlWnd = sortdata->hLV;

    nIsDir1 = SendMessage (hCtrlWnd, LVM_GETITEMADDDATA, 0, nItem1);
    nIsDir2 = SendMessage (hCtrlWnd, LVM_GETITEMADDDATA, 0, nItem2);
    
    if ( nIsDir1 >nIsDir2 )
        return 1;
        
    if (nIsDir1 < nIsDir2)
        return -1;

    //subItem1.nItem = nItem1;
    subItem1.subItem = 3;
    subItem1.pszText = (char *)malloc (MY_NAMEMAX+1);
    if (subItem1.pszText == NULL)
        return 0;
    SendMessage (hCtrlWnd, LVM_GETSUBITEMTEXT, 
            (WPARAM)nItem1, (LPARAM)&subItem1);

    //subItem2.nItem = nItem2;
    subItem2.subItem = 3;
    subItem2.pszText = (char *)malloc (MY_NAMEMAX+1);
    if (subItem2.pszText == NULL) {
        if (subItem1.pszText != NULL) free (subItem1.pszText);
        return 0;
    }
    SendMessage (hCtrlWnd, LVM_GETSUBITEMTEXT, 
            (WPARAM)nItem2, (LPARAM)&subItem2);

    nResult =  strcmp (subItem1.pszText, subItem2.pszText);

    if (subItem1.pszText != NULL) free (subItem1.pszText);
    if (subItem2.pszText != NULL) free (subItem2.pszText);

    return nResult;
}

static inline BOOL is_dir(const char* path)
{
	if(path == NULL)
		return FALSE;

#ifdef __LINUX__
	struct stat	s;
	if(stat(path, &s) != 0)
		return FALSE;
	return S_ISDIR(s.st_mode);	
#elif defined(WIN32) //for windows or other system
	return test_dir(path);
#endif
}

static void GetFileAndDirList( HWND hWnd, char* path, char* filtstr)
{
    HWND     hCtrlWnd;
    struct   dirent* pDirEnt;
    DIR*     dir;
    struct   stat ftype;
    char     fullpath [MY_PATHMAX + 1];
    char     filefullname[MY_PATHMAX+MY_NAMEMAX+1];
    FILEINFO fileinfo;
    int      nRet;
    int      nCheckState;
#ifdef WIN32
	char szcwd[MY_PATHMAX+1];
#endif

	if(!is_dir(path))
		return ;

    hCtrlWnd = GetDlgItem (hWnd, IDC_FOSD_ISHIDE);

    if (hCtrlWnd)
        nCheckState = SendMessage (hCtrlWnd, BM_GETCHECK, 0, 0);
    else
        /*For DefSimpleFileCtrl*/
        nCheckState = BST_CHECKED;

    hCtrlWnd = GetDlgItem (hWnd, IDC_FOSD_FILELIST);
    SendMessage (hCtrlWnd, LVM_DELALLITEM, 0, 0);
   
    if (path == NULL) return;
    strtrimall (path);
    if (strlen (path) == 0) return;

#ifdef WIN32
	if(strcmp(path, "/") == 0)
	{
		int i;
		unsigned int drivers = getdrivers();
		memset(&fileinfo, 0, sizeof(fileinfo));
		fileinfo.IsDir = TRUE;
		SendMessage (hCtrlWnd, MSG_FREEZECTRL, TRUE, 0);
		for(i=0; i<32; i++){
			if(0x1 & drivers){
				sprintf(fileinfo.filename, "%c:\\", i+'A');
				InsertToListView (hWnd, &fileinfo);
			}
			drivers >>= 1;
		}
		SendMessage (hCtrlWnd, LVM_COLSORT, (WPARAM)0, 0);
		SendMessage (hCtrlWnd, MSG_FREEZECTRL, FALSE, 0);
		return ;
	}

	if((path[strlen(path)-1] == '\\' ||
		path[strlen(path)-1] == '/') && path[strlen(path)-2] != ':')
#else
    if (strcmp (path, "/") != 0 && path [strlen(path)-1] == '/')
#endif
        path [strlen(path)-1]=0;

    SendMessage (hCtrlWnd, MSG_FREEZECTRL, TRUE, 0);
 

	/* bug :for windows
	* the opendir function will change currend work dir
	* to "path"
	*/
#ifdef WIN32
	getcwd(szcwd, sizeof(szcwd)-1);
#endif
    dir = opendir (path);

    while ((pDirEnt = readdir ( dir )) != NULL ) {

        memset (&fileinfo, 0, sizeof (fileinfo));    
        
         
        if ( strcmp (pDirEnt->d_name, ".") == 0 
            || strcmp (pDirEnt->d_name, "..") == 0) 
            continue;

        if ( nCheckState != BST_CHECKED) {
            if (pDirEnt->d_name [0] == '.') 
                continue;
        }

		strncpy (fullpath, path, MY_PATHMAX);
        strcat (fullpath, "/");
        strcat (fullpath, pDirEnt->d_name);

        if (stat (fullpath, &ftype) < 0 ){
           continue;
        }
        
        if (S_ISDIR (ftype.st_mode)){
            fileinfo.IsDir = TRUE;
            fileinfo.filesize = ftype.st_size;
        }
        else if (S_ISREG (ftype.st_mode)) {
            if ( !IsInFilter (filtstr, pDirEnt->d_name) )
                continue;
            
            fileinfo.IsDir = FALSE;
            fileinfo.filesize = ftype.st_size;
        }
        
        strcpy (fileinfo.filename, pDirEnt->d_name);
        
#ifdef WIN32
		sprintf (filefullname, "%s\\%s", 
            strcmp (path, "\\") == 0 ? "" : path, pDirEnt->d_name);  
#else
        sprintf (filefullname, "%s/%s", 
            strcmp (path, "/") == 0 ? "" : path, pDirEnt->d_name);  
#endif

        fileinfo.accessmode = 0;
        nRet = GetAccessMode (hWnd, filefullname, FALSE, FALSE);
        if (nRet != -1 && nRet != -2)
            fileinfo.accessmode = 1;


        nRet = GetAccessMode (hWnd, filefullname, TRUE, FALSE);
        if (nRet != -1 && nRet != -3)
            fileinfo.accessmode = fileinfo.accessmode + 2;

        fileinfo.modifytime = ftype.st_mtime;

        InsertToListView (hWnd, &fileinfo);
    }

    closedir (dir);

#ifdef WIN32
	chdir(szcwd);
#endif
    SendMessage (hCtrlWnd, LVM_COLSORT, (WPARAM)0, 0);
    SendMessage (hCtrlWnd, MSG_FREEZECTRL, FALSE, 0);
}

static void InitListView( HWND hWnd)
{
    HWND      hListView;
    int       i;
    LVCOLUMN  lvcol;
    int       nWidth, nColWidth;
    RECT      rcLv;
 
    hListView = GetDlgItem (hWnd, IDC_FOSD_FILELIST);
    GetWindowRect (hListView, &rcLv);
    nWidth = rcLv.right - rcLv.left;
    nColWidth = MIN_WIDTH - OTHERS_WIDTH;

    if (nWidth > MIN_WIDTH)
        nColWidth = nWidth - OTHERS_WIDTH;

    for (i = 0; i < COL_COUNT; i++) {
        lvcol.nCols = i;
        lvcol.pszHeadText = head_text[i];
        lvcol.width = ColWidth[i];
        /*lvcol.pfnCompare = ListViewSort;*/
        lvcol.pfnCompare = NULL;
        if (i == 0){ 
            lvcol.width = nColWidth - 28;
            lvcol.pfnCompare = ListViewSortByName;
        }
        else if (i == 1)
            lvcol.pfnCompare = ListViewSortBySize;
        else if (i == 3)
            lvcol.pfnCompare = ListViewSortByDate;

        lvcol.colFlags = 0;
        SendMessage (hListView, LVM_ADDCOLUMN, 0, (LPARAM) &lvcol);
    }
}

static void InitPathCombo(HWND hWnd, char *path)
{
    HWND hCtrlWnd;
    char chSubPath[MY_PATHMAX];
    char chPath[MY_PATHMAX];
    char *pStr;

    if (path == NULL) return;

    strcpy (chPath, path);
    strtrimall (chPath);
    if ( strlen (chPath) == 0 ) return;

    if (strcmp (chPath, "/") != 0 &&  chPath[strlen(chPath)-1] == '/') {
        chPath [strlen (chPath) - 1] = 0;
    }

    hCtrlWnd = GetDlgItem (hWnd, IDC_FOSD_PATH);
    SendMessage (hCtrlWnd, CB_RESETCONTENT, 0, 0);
    SendMessage (hCtrlWnd, CB_SETITEMHEIGHT, 0, (LPARAM)GetSysCharHeight()+2);
    
    strcpy(chSubPath, "/");
    SendMessage (hCtrlWnd, CB_ADDSTRING, 0,(LPARAM)chSubPath);
    
    pStr = strchr(chPath + 1, '/');
    while (pStr != NULL){
        memset (chSubPath, 0, sizeof (chSubPath));
        strncpy (chSubPath, chPath, pStr -chPath);
        SendMessage (hCtrlWnd, CB_INSERTSTRING, 0,(LPARAM)chSubPath);
        pStr = strchr (chPath + (pStr -chPath +1), '/');
    }
    
    if (strcmp (chPath, "/") != 0 ){
        SendMessage (hCtrlWnd, CB_INSERTSTRING, 0,(LPARAM)chPath);
    }

    SetWindowText (hCtrlWnd, chPath);            
}

static void InitFilterCombo( HWND hWnd, char * filtstr, int _index)
{
    HWND hCtrlWnd;
    char chFilter[MAX_FILTER_LEN+1];
    char chTemp[MAX_FILTER_LEN+1];
    char *p1=NULL;
    char *p2=NULL;
    int  nCount;

    if ( filtstr ==NULL ) return;
    
    strcpy (chFilter, filtstr);    
    strtrimall (chFilter);
    if ( strlen(chFilter) == 0 ) return;
     
    
    hCtrlWnd = GetDlgItem (hWnd, IDC_FOSD_FILETYPE);
    SendMessage (hCtrlWnd, CB_RESETCONTENT, 0, 0);
    SendMessage (hCtrlWnd, CB_SETITEMHEIGHT, 0, (LPARAM)GetSysCharHeight()+2);

    p1 = strchr (chFilter, '|');
    p2 = chFilter;
    while ( p1 != NULL ) {
        memset (chTemp, 0, sizeof (chTemp));
        strncpy (chTemp, p2, p1 - p2);
        SendMessage (hCtrlWnd, CB_ADDSTRING, 0,(LPARAM)chTemp);
        p2 = p1 + 1;
        p1 = strchr (p2, '|');
    }
   
    memset (chTemp, 0, sizeof(chTemp));
    strcpy (chTemp, p2);
    SendMessage (hCtrlWnd, CB_ADDSTRING, 0,(LPARAM)chTemp);
    nCount = SendMessage (hCtrlWnd, CB_GETCOUNT, 0, 0);
    
    if (_index >=nCount)
        SendMessage (hCtrlWnd, CB_SETCURSEL, nCount - 1, 0);
    else
        SendMessage (hCtrlWnd, CB_SETCURSEL, _index, 0);

}

static void InitOpenDialog( HWND hWnd, PFILEDLGDATA pfdd)
{
    HWND hCtrlWnd;
    char chFilter[MAX_FILTER_LEN+1];

    InitPathCombo (hWnd, pfdd->filepath);
    InitFilterCombo (hWnd, pfdd->filter, pfdd->filterindex); 
    InitListView (hWnd);
    
    hCtrlWnd = GetDlgItem (hWnd, IDC_FOSD_FILETYPE);
    memset (chFilter, 0, sizeof(chFilter));
    GetWindowText (hCtrlWnd, chFilter, MAX_FILTER_LEN);    
    //if ( GetAccessMode (hWnd, pnfdd->filepath, pnfdd->IsSave, TRUE)==0)
    GetFileAndDirList (hWnd, pfdd->filepath, chFilter);

    //hCtrlWnd = GetDlgItem (hWnd, IDC_FILECHOISE);
    //SendMessage (hCtrlWnd, LVM_COLSORT, (WPARAM)1, 0);
}

int DefFileDialogProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    HWND hCtrlWnd;
    int  nId, nNc;
    char chPath[MY_PATHMAX+1];
    char chFilter[MAX_FILTER_LEN+1];
    char chFileName[MY_NAMEMAX+1];
    char chFullName[MY_PATHMAX+MY_NAMEMAX+1];
    int  nSelItem;
    int  nIsDir;
    LVSUBITEM subItem; 

    switch (message){
        case MSG_INITDIALOG:
            {
                PFILEDLGDATA pfdd = (PFILEDLGDATA)lParam;
                
                /* get current directory name */
                if (strcmp (pfdd->filepath, ".") == 0 ||
                        strcmp (pfdd->filepath, "./") == 0)
                    getcwd (pfdd->filepath, MY_PATHMAX);

                SetWindowAdditionalData (hDlg, (DWORD)lParam);

                InitOpenDialog (hDlg, pfdd);

                if (pfdd->is_save) {
                    hCtrlWnd = GetDlgItem (hDlg, IDC_FOSD_OK); 
                    SetWindowText (hCtrlWnd, "Save");
                }
                
                if (pfdd->hook)
                    return pfdd->hook (hDlg, message, wParam, lParam);

                return 1;
            }

        case MSG_COMMAND:
            {
                PFILEDLGDATA pfdd = (PFILEDLGDATA)GetWindowAdditionalData (hDlg);

                hCtrlWnd = GetDlgItem (hDlg, IDC_FOSD_FILENAME);
                memset (chFileName, 0, sizeof(chFileName));
                GetWindowText (hCtrlWnd, chFileName, MY_NAMEMAX);

                hCtrlWnd = GetDlgItem (hDlg, IDC_FOSD_PATH);
                memset (chPath, 0, sizeof(chPath));
                GetWindowText (hCtrlWnd, chPath, MY_PATHMAX);

                hCtrlWnd = GetDlgItem (hDlg, IDC_FOSD_FILETYPE);
                memset (chFilter, 0, sizeof(chFilter));
                GetWindowText (hCtrlWnd, chFilter, MAX_FILTER_LEN);

                nId = LOWORD(wParam);
                nNc = HIWORD(wParam);

                hCtrlWnd = GetDlgItem (hDlg, nId);

                switch (nId){
                    case IDC_FOSD_PATH:
                    case IDC_FOSD_FILETYPE:
                    case IDC_FOSD_ISHIDE:
                        if (nNc == CBN_SELCHANGE || nNc == BN_CLICKED){
                            GetFileAndDirList (hDlg, chPath, chFilter);
                        }
                        break;
                    case IDC_FOSD_UPPER:
                        if (nNc == BN_CLICKED) {
							int _index;
                            GetParentDir (chPath);
                            hCtrlWnd = GetDlgItem (hDlg, IDC_FOSD_PATH);
                            
                            _index = SendMessage (hCtrlWnd, CB_FINDSTRINGEXACT, 0,(LPARAM)chPath);
                            
                            if (CB_ERR != _index){
                               SendMessage (hCtrlWnd, CB_SETCURSEL, _index, 0);
                            } else {

                                _index = SendMessage (hCtrlWnd, CB_INSERTSTRING, 0,(LPARAM)chPath);
                                SendMessage (hCtrlWnd, CB_SETCURSEL, _index, 0);
                            }
                            GetFileAndDirList (hDlg, chPath, chFilter);
                        } 
                        break;
                    case IDC_FOSD_OK:
                        if (strlen (chFileName) != 0) {
                            memset (chFullName, 0, sizeof(chFullName));
                            sprintf (chFullName, "%s/%s", 
                                    strcmp (chPath, "/") == 0 ? "" : chPath, chFileName);
                            if (!pfdd->is_save 
                                    || (pfdd->is_save 
                                        && FileExistDlg (hDlg, chFullName, chFileName) == 0))
                            {
                                strcpy (pfdd->filefullname, chFullName);
                                strcpy (pfdd->filename, chFileName);
                                strcpy (pfdd->filepath, chPath);
                                EndDialog (hDlg, IDOK);
                            }

                            if (!pfdd->hook || (pfdd->hook && !pfdd->hook (hDlg, MSG_FILESELOK, (WPARAM)0, (LPARAM)pfdd)))
                                break;
                            else{
                                EndDialog (hDlg, IDOK);
                                return 0;
                            }
                        }
                        else {
                            hCtrlWnd = GetDlgItem (hDlg, IDC_FOSD_FILELIST);
                            nSelItem = SendMessage (hCtrlWnd, LVM_GETSELECTEDITEM, 0, 0);
                            if (nSelItem > 0) {
                                nIsDir = SendMessage (hCtrlWnd, 
                                        LVM_GETITEMADDDATA, 0, nSelItem);
                                memset (&subItem, 0, sizeof (subItem));
                                //subItem.nItem = nSelItem;
                                subItem.subItem = 0;
                                subItem.pszText = (char *)malloc (MY_NAMEMAX+1);
                                if (subItem.pszText == NULL) 
                                    break;
                                SendMessage (hCtrlWnd, LVM_GETSUBITEMTEXT, 
                                        (WPARAM)nSelItem, (LPARAM)&subItem);
                                if (nIsDir == 1 ) {
#ifdef WIN32
									if(strcmp(chPath, "/") == 0)
										strcpy(chPath,subItem.pszText);
									else
										sprintf(chPath,"%s\\%s",chPath,subItem.pszText);
#else
                                    sprintf (chPath, "%s/%s", 
                                            strcmp (chPath, "/") == 0 ? "" : chPath, subItem.pszText);
#endif
                                    hCtrlWnd = GetDlgItem (hDlg, IDC_FOSD_PATH);
                                    if (GetAccessMode (hDlg, chPath, 0, TRUE) == 0){
                                        GetFileAndDirList (hDlg, chPath, chFilter);
                                        SetWindowText (hCtrlWnd, chPath);
                                        if (CB_ERR == SendMessage (hCtrlWnd, CB_FINDSTRINGEXACT, 0,(LPARAM)chPath)){
                                            int _index = SendMessage (hCtrlWnd, 
                                                    CB_INSERTSTRING, 0,(LPARAM)chPath);
                                            SendMessage (hCtrlWnd, CB_SETCURSEL, _index, 0);
                                        }
                                    }
                                }

                                if (subItem.pszText != NULL ) free (subItem.pszText);
                            }
                        } 
                        break;

                    case IDC_FOSD_CANCEL:
                        strcpy (pfdd->filefullname, "");
                        strcpy (pfdd->filename, "");
                        EndDialog (hDlg, IDCANCEL);
                        break;

                    case IDC_FOSD_FILELIST:
                        if (nNc == LVN_ITEMDBCLK || nNc == LVN_ITEMCLK || nNc == LVN_CLICKED) {
                            nSelItem = SendMessage (hCtrlWnd, LVM_GETSELECTEDITEM, 0, 0);
                            if (nSelItem > 0 ) {
                                nIsDir = SendMessage (hCtrlWnd, LVM_GETITEMADDDATA, 0, nSelItem);    
                                memset (&subItem, 0, sizeof (subItem));
                                //subItem.nItem = nSelItem;
                                subItem.subItem = 0;
                                subItem.pszText = (char *)calloc (MY_NAMEMAX + 1, 1);
                                if (subItem.pszText == NULL) 
                                    break;
                                SendMessage (hCtrlWnd, LVM_GETSUBITEMTEXT,  (WPARAM)nSelItem, (LPARAM)&subItem);

                                hCtrlWnd = GetDlgItem (hDlg, IDC_FOSD_FILENAME);
                                if (nIsDir == 0){
                                    SetWindowText (hCtrlWnd, subItem.pszText);
                                }
                                else 
                                    SetWindowText (hCtrlWnd, "");

                                if ((nIsDir ==0 && nNc == LVN_ITEMDBCLK) || (nIsDir == 1)) 
                                    SendNotifyMessage (hDlg, MSG_COMMAND, IDC_FOSD_OK, 0);

                                if (subItem.pszText !=NULL ) free (subItem.pszText); 
                            }          
                        }

                        break;
                } /*switch (nId) end */
				break;
            }
			case MSG_CLOSE:
				return SendMessage(hDlg, MSG_COMMAND, IDC_FOSD_CANCEL, 0);
    }
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

/**
* If parameter dlg_template is null, it will use default dialog template.
* If parameter proc is null, it will use default window procedure.
*/
BOOL FileOpenSaveDialog  (PDLGTEMPLATE dlg_template, 
        HWND hwnd, WNDPROC proc, PFILEDLGDATA pfdd)
{
    PDLGTEMPLATE file_dlg;

    WNDPROC file_proc;

    if (dlg_template) {
        file_dlg = dlg_template;
    }
    else {
        file_dlg = &DefFileDlg;
    }

    if (pfdd->is_trans == TRUE)
    {
        int i;
        for (i = 0; i < file_dlg->controlnr; i++)
            file_dlg->controls[i].dwExStyle |= WS_EX_TRANSPARENT;
    }

    if (proc) {
        file_proc = proc;
    }
    else {
        file_proc = DefFileDialogProc;
    }

    return ShowCommonDialog (file_dlg, hwnd, file_proc, pfdd);
}


