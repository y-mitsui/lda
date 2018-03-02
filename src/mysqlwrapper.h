#ifndef MYSQLWRAPPER_H
#define MYSQLWRAPPER_H

#include	<string>
#include	<map>
#include	<list>
#include	<my_global.h>
#include	<mysql.h>

#define	LOCK_MODE_READ	1
#define	LOCK_MODE_WRITE	2

#define	MYSQL_TIMEOUT 1000000

using namespace std;

enum DbValueType{
    TypeString,
    TypeDefault
};

struct DbValue{
    enum DbValueType type;
    std::string value;
};

class MySqlWrapper
{
public:
	MySqlWrapper();
	~MySqlWrapper();

public:
    MYSQL *m_connection;
    MYSQL m_mysql;
	bool init();
	bool connect(const char *host, const char *user, const char *password, const char *dbname, int port = 3306);
	bool isConnect();
	bool close();
	bool query(const char *pQuery);
	MYSQL_RES* getResult();
	MYSQL_ROW getRow();
	bool clear();

	const char* getError();
	bool lock(char *szTable, int nMode);
	bool unLock();

	long long getLastID();
	long long getRowCount();

	void convertEscape(std::string &str);
	bool allive_check();

    bool insertUpdate(string table, map<string, DbValue> &save_data);
private:
	
	
	MYSQL_RES *m_result;
	MYSQL_ROW m_row;
	bool m_flg;
};

class MySqlWrapper2 {
    
    MYSQL_RES *m_result;
    public:
        MYSQL m_mysql;
        MySqlWrapper2(const char *host, const char *user, const char *password, const char *dbname, int port = 3306);
        ~MySqlWrapper2();
        MYSQL_RES *query(string sql);
        MYSQL_ROW getRow();
        bool insertUpdate(string table, map<string, DbValue> &save_data);
};

#endif
