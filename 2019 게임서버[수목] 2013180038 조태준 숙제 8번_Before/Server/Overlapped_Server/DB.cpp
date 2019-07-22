#include "DB.h"

CDB::CDB()
	: m_bIsInitalize(false)
{
	setlocale(LC_ALL, "korean");
}

bool CDB::InitalizeDB()
{
	SQLRETURN retcode;

	// Allocate environment handle  
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hEnv);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(m_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, m_hEnv, &m_hDbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(m_hDbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
			}
			else
				return false;
		}
		else
			return false;
	}
	else 
		return false;

	cout << "Database Initalize OK!\n";
	m_bIsInitalize = true;
	return true;
}

bool CDB::ConnectDB(SQLWCHAR* dbName, SQLWCHAR* id, SQLWCHAR* pw)
{
	if (m_bIsInitalize == false)
		return false;

	SQLRETURN retcode;

	// Connect to data source  
	retcode = SQLConnect(m_hDbc, dbName, SQL_NTS, id, (SQLSMALLINT)wcslen(id), pw, (SQLSMALLINT)wcslen(pw));
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		m_bIsConnected = true;
		cout << "Database Connect OK\n";
		//retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_hDbc, &m_hStmt);
		return true;
	}
	else
		display_error_DB(m_hDbc, SQL_HANDLE_DBC, retcode);
	
	return false;
}

void CDB::ClearDB()
{
	DisconnectDB();

	SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc);

	SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
}

bool CDB::DisconnectDB()
{
	if (m_bIsConnected == false)
		return false;

	//SQLCancel(m_hStmt);
	//SQLFreeHandle(SQL_HANDLE_STMT, m_hStmt);

	SQLDisconnect(m_hDbc);

	return true;
}

bool CDB::ExecDirect(SQLWCHAR* wsqlStr)
{
	if (m_bIsConnected == false)
		return false;

	SQLRETURN retcode;
	
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_hDbc, &m_hStmt);

	retcode = SQLExecDirect(m_hStmt, wsqlStr, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		return true;
	else
		display_error_DB(m_hStmt, SQL_HANDLE_STMT, retcode);

	return false;
}

bool CDB::LoginToDB(TCHAR* pwname, CChessClient* pclient)
{
	SQLRETURN retcode;

	// name
	wstring str = L"EXEC select_username ";
	str += pwname;

	TCHAR* execStr = (TCHAR*)str.c_str();

	if (ExecDirect((SQLWCHAR*)execStr) == true)
	{
		SQLINTEGER ulevel{}, ux{}, uy{};
		SQLLEN cb_ux = 0, cb_uy = 0, cb_ulevel = 0;

		retcode = SQLBindCol(m_hStmt, 1, SQL_INTEGER, &ulevel, 10, &cb_ulevel);
		retcode = SQLBindCol(m_hStmt, 2, SQL_INTEGER, &ux, 10, &cb_ux);
		retcode = SQLBindCol(m_hStmt, 3, SQL_INTEGER, &uy, 10, &cb_uy);

		retcode = SQLFetch(m_hStmt);
		if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
			return false;
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			wprintf(L"level : %d x : %d y : %d\n", ulevel, ux, uy);
			pclient->m_nLevel = ulevel;
			pclient->m_X = (float)ux;
			pclient->m_Y = (float)uy;

			SQLCancel(m_hStmt);
			SQLFreeHandle(SQL_HANDLE_STMT, m_hStmt);
		}
		else {
			display_error_DB(m_hStmt, SQL_HANDLE_STMT, retcode);
			return false;
		}
	}
	else {
		return false;
	}

	return true;
}

bool CDB::SignupToDB(TCHAR* pwname)
{
	SQLRETURN retcode;

	// name
	wstring str = L"EXEC select_username ";
	str += pwname;

	TCHAR* execStr = (TCHAR*)str.c_str();

	if (ExecDirect((SQLWCHAR*)execStr) == true)
	{
		SQLINTEGER ulevel{}, ux{}, uy{};
		SQLLEN cb_ux = 0, cb_uy = 0, cb_ulevel = 0;

		retcode = SQLBindCol(m_hStmt, 1, SQL_INTEGER, &ulevel, 10, &cb_ulevel);
		retcode = SQLBindCol(m_hStmt, 2, SQL_INTEGER, &ux, 10, &cb_ux);
		retcode = SQLBindCol(m_hStmt, 3, SQL_INTEGER, &uy, 10, &cb_uy);

		retcode = SQLFetch(m_hStmt);
		if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
			return false;
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			SQLCancel(m_hStmt);
			SQLFreeHandle(SQL_HANDLE_STMT, m_hStmt);
			return false;
		}
		else {
			// name
			str = L"EXEC insert_username ";
			str += pwname;

			execStr = (TCHAR*)str.c_str();

			if (ExecDirect((SQLWCHAR*)execStr) == true)
			{
				//retcode = SQLFetch(m_hStmt);
			}
			else
				return false;
		}
	}
	else {
		return false;
	}

	return true;
}

bool CDB::SaveClientInfoInDB(CChessClient* pclient)
{
	SQLRETURN retcode{};

	// name, level, x, y
	wstring str = L"EXEC update_clientinfo ";
	
	TCHAR* name = pclient->m_Name;
	INT level = pclient->m_nLevel;
	INT x = (INT)pclient->m_X;
	INT y = (INT)pclient->m_Y;

	TCHAR temp[10]{};

	str += name;
	str += L", ";
	_itow_s(level, temp, 10);
	str += (temp);
	str += L", ";
	_itow_s(x, temp, 10);
	str += temp;
	str += L", ";
	_itow_s(y, temp, 10);
	str += temp;

	TCHAR* execStr = (TCHAR*)str.c_str();

	if (ExecDirect((SQLWCHAR*)execStr) == true)
	{
		
	}
	else {
		return false;
	}

	return true;
}
