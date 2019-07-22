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
		SQLINTEGER ulevel{}, uexp{}, ux{}, uy{}, uhp{}, ump{};
		SQLLEN cb_ux = 0, cb_uy = 0, cb_ulevel = 0, cb_uexp = 0, cb_uhp = 0, cb_ump = 0;

		retcode = SQLBindCol(m_hStmt, 1, SQL_INTEGER, &ulevel, 10, &cb_ulevel);
		retcode = SQLBindCol(m_hStmt, 2, SQL_INTEGER, &uexp, 10, &cb_uexp);
		retcode = SQLBindCol(m_hStmt, 3, SQL_INTEGER, &ux, 10, &cb_ux);
		retcode = SQLBindCol(m_hStmt, 4, SQL_INTEGER, &uy, 10, &cb_uy);
		retcode = SQLBindCol(m_hStmt, 5, SQL_INTEGER, &uhp, 10, &cb_uhp);
		retcode = SQLBindCol(m_hStmt, 6, SQL_INTEGER, &ump, 10, &cb_ump);

		retcode = SQLFetch(m_hStmt);
		if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
			return false;
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			//wprintf(L"level : %d x : %d y : %d hp : %d mp : %d\n", ulevel, ux, uy, uhp, ump);
			pclient->m_nLevel = ulevel;
			pclient->m_nExp = uexp;
			pclient->m_nX = ux;
			pclient->m_nY = uy;
			pclient->m_nHP = uhp;
			pclient->m_nMP = ump;

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

	////////////////////////////// ITEM //////////////////////////////

	str = L"EXEC get_useritem ";
	str += pwname;

	execStr = (TCHAR*)str.c_str();

	if (ExecDirect((SQLWCHAR*)execStr) == true)
	{
		SQLINTEGER uhppottionnum{}, umppotionnum{}, umoney{};
		SQLLEN cb_uhppottionnum = 0, cb_umppotionnum = 0, cb_umoney = 0;

		retcode = SQLBindCol(m_hStmt, 1, SQL_INTEGER, &uhppottionnum, 10, &cb_uhppottionnum);
		retcode = SQLBindCol(m_hStmt, 2, SQL_INTEGER, &umppotionnum, 10, &cb_umppotionnum);
		retcode = SQLBindCol(m_hStmt, 3, SQL_INTEGER, &umoney, 10, &cb_umoney);

		retcode = SQLFetch(m_hStmt);
		if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
			return false;
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			pclient->m_Item.m_nHPPotionNum = uhppottionnum;
			pclient->m_Item.m_nMPPotionNum = umppotionnum;
			pclient->m_Item.m_nMoney = umoney;

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

	////////////////////////////// STATUS //////////////////////////////

	str = L"EXEC get_userstatus ";
	str += pwname;

	execStr = (TCHAR*)str.c_str();

	if (ExecDirect((SQLWCHAR*)execStr) == true)
	{
		SQLINTEGER umaxhp{}, umaxmp{}, ustr{}, udex{}, uwill{}, uint{}, upoints{};
		SQLLEN cb_umaxhp = 0, cb_umaxmp = 0, cb_ustr = 0, cb_udex = 0, cb_uwill = 0, cb_uint = 0, cb_upoints = 0;

		retcode = SQLBindCol(m_hStmt, 1, SQL_INTEGER, &umaxhp, 10, &cb_umaxhp);
		retcode = SQLBindCol(m_hStmt, 2, SQL_INTEGER, &umaxmp, 10, &cb_umaxmp);
		retcode = SQLBindCol(m_hStmt, 3, SQL_INTEGER, &ustr, 10, &cb_ustr);
		retcode = SQLBindCol(m_hStmt, 4, SQL_INTEGER, &udex, 10, &cb_udex);
		retcode = SQLBindCol(m_hStmt, 5, SQL_INTEGER, &uwill, 10, &cb_uwill);
		retcode = SQLBindCol(m_hStmt, 6, SQL_INTEGER, &uint, 10, &cb_uint);
		retcode = SQLBindCol(m_hStmt, 7, SQL_INTEGER, &upoints, 10, &cb_upoints);

		retcode = SQLFetch(m_hStmt);
		if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
			return false;
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			pclient->m_Status.m_nMaxHP = umaxhp;
			pclient->m_Status.m_nMaxMP = umaxmp;
			pclient->m_Status.m_nStr = ustr;
			pclient->m_Status.m_nDex = udex;
			pclient->m_Status.m_nWill = uwill;
			pclient->m_Status.m_nInt = uint;
			pclient->m_Status.m_nStatPoints = upoints;

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
		SQLINTEGER ulevel{}, ux{}, uy{}, uhp{}, ump{};
		SQLLEN cb_ux = 0, cb_uy = 0, cb_ulevel = 0, cb_uhp = 0, cb_ump = 0;

		retcode = SQLBindCol(m_hStmt, 1, SQL_INTEGER, &ulevel, 10, &cb_ulevel);
		retcode = SQLBindCol(m_hStmt, 2, SQL_INTEGER, &ux, 10, &cb_ux);
		retcode = SQLBindCol(m_hStmt, 3, SQL_INTEGER, &uy, 10, &cb_uy);
		retcode = SQLBindCol(m_hStmt, 4, SQL_INTEGER, &uhp, 10, &cb_uhp);
		retcode = SQLBindCol(m_hStmt, 5, SQL_INTEGER, &ump, 10, &cb_ump);

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
		//display_error_DB()
		return false;
	}

	return true;
}

bool CDB::SaveClientInfoInDB(CChessClient* pclient)
{
	SQLRETURN retcode{};

	// name, level, exp, x, y, hp, mp
	wstring str = L"EXEC update_clientinfo ";
	
	TCHAR* name = pclient->m_Name;
	INT level = pclient->m_nLevel;
	INT exp = pclient->m_nExp;
	INT x = (INT)pclient->m_nX;
	INT y = (INT)pclient->m_nY;
	INT hp = pclient->m_nHP;
	INT mp = pclient->m_nMP;

	TCHAR temp[10]{};

	str += name;
	str += L", ";
	_itow_s(level, temp, 10);
	str += temp;
	str += L", ";
	_itow_s(exp, temp, 10);
	str += temp;
	str += L", ";
	_itow_s(x, temp, 10);
	str += temp;
	str += L", ";
	_itow_s(y, temp, 10);
	str += temp;
	str += L", ";
	_itow_s(hp, temp, 10);
	str += temp;
	str += L", ";
	_itow_s(mp, temp, 10);
	str += temp;

	TCHAR* execStr = (TCHAR*)str.c_str();

	if (ExecDirect((SQLWCHAR*)execStr) == true)
	{
		
	}
	else {
		return false;
	}

	//////////////////////////////// ITEM /////////////////////////////////////

	str = L"EXEC update_useritem ";

	int hppottionnum = pclient->m_Item.m_nHPPotionNum;
	int mppottionnum = pclient->m_Item.m_nMPPotionNum;
	int money = pclient->m_Item.m_nMoney;

	str += name;
	str += L", ";
	_itow_s(hppottionnum, temp, 10);
	str += temp;
	str += L", ";
	_itow_s(mppottionnum, temp, 10);
	str += temp;
	str += L", ";
	_itow_s(money, temp, 10);
	str += temp;
	
	execStr = (TCHAR*)str.c_str();

	if (ExecDirect((SQLWCHAR*)execStr) == true)
	{

	}
	else {
		return false;
	}

	//////////////////////////////// STATUS /////////////////////////////////////

	str = L"EXEC update_userstatus ";

	int maxhp = pclient->m_Status.m_nMaxHP;
	int maxmp = pclient->m_Status.m_nMaxMP;
	int Str = pclient->m_Status.m_nStr;
	int dex = pclient->m_Status.m_nDex;
	int will = pclient->m_Status.m_nWill;
	int Int = pclient->m_Status.m_nInt;
	int points = pclient->m_Status.m_nStatPoints;

	str += name;
	str += L", ";
	_itow_s(maxhp, temp, 10);
	str += temp;
	str += L", ";
	_itow_s(maxmp, temp, 10);
	str += temp;
	str += L", ";
	_itow_s(Str, temp, 10);
	str += temp;
	str += L", ";
	_itow_s(dex, temp, 10);
	str += temp;
	str += L", ";
	_itow_s(will, temp, 10);
	str += temp;
	str += L", ";
	_itow_s(Int, temp, 10);
	str += temp;
	str += L", ";
	_itow_s(points, temp, 10);
	str += temp;

	execStr = (TCHAR*)str.c_str();

	if (ExecDirect((SQLWCHAR*)execStr) == true)
	{

	}
	else {
		return false;
	}

	return true;
}
