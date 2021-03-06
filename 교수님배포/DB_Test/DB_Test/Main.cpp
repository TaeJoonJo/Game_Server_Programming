
// SQLBindCol_ref.cpp  
// compile with: odbc32.lib  
#include <windows.h>  
#include <stdio.h>  
#include <iostream>

using namespace std;

#define UNICODE  
#include <sqlext.h>  

#define NAME_LEN 50  
#define PHONE_LEN 20  

void display_error(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode) 
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER  iError;
	WCHAR       wszMessage[1000];
	WCHAR       wszState[SQL_SQLSTATE_SIZE + 1];
	if (RetCode == SQL_INVALID_HANDLE) 
	{ 
		fwprintf(stderr, L"Invalid handle!\n"); 
		return; 
	} 
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage, (SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS)
	{ // Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) 
		{ 
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}

void show_error() 
{
	printf("error\n");
}

int main() {
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;
	SQLWCHAR uname[NAME_LEN]{};
	SQLINTEGER uid{}, ulevel{};
	SQLLEN cb_uname = 0, cb_uid = 0, cb_ulevel = 0;
	SQLWCHAR myid[] = { L"TJ" };
	SQLWCHAR mypw[] = { L"102030" };
	setlocale(LC_ALL, "korean");

	// Allocate environment handle  
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source  
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"Game_Server", SQL_NTS, (SQLWCHAR*)myid, wcslen(myid), (SQLWCHAR*)mypw, wcslen(mypw));

				// Allocate statement handle  
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					cout << "Database Connect OK\n";
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

					//retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"SELECT user_id, user_name, user_level FROM user_table ORDER BY 2, 1, 3", SQL_NTS);
					retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"EXEC select_highlevel 1", SQL_NTS);
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

						// Bind columns 1, 2, and 3  
						retcode = SQLBindCol(hstmt, 1, SQL_INTEGER, &uid, 10, &cb_uid);
						retcode = SQLBindCol(hstmt, 2, SQL_C_CHAR, uname, NAME_LEN, &cb_uname);
						retcode = SQLBindCol(hstmt, 3, SQL_INTEGER, &ulevel, 10, &cb_ulevel);

						// Fetch and print each row of data. On an error, display a message and exit.
						for (int i = 0; ; i++) {
							retcode = SQLFetch(hstmt);
							if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
								show_error();
							if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
								wprintf(L"%d: %d %S %d\n", i + 1, uid, (char*)uname, ulevel);
							else
								break;
						}
					}
					else {
						display_error(hstmt, SQL_HANDLE_STMT, retcode);
					}

					// Process data  
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
						SQLCancel(hstmt);
						SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
					}

					SQLDisconnect(hdbc);
				}
				else
				{
					display_error(hdbc, SQL_HANDLE_DBC, retcode);
				}

				SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
			}
		}
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
	}
}