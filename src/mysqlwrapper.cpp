#include	"mysqlwrapper.h"

std::string str_replace( std::string &String1, std::string &String2, std::string &String3 )
{
    std::string::size_type  Pos( String1.find( String2 ) );

    while( Pos != std::string::npos )
    {
        String1.replace( Pos, String2.length(), String3 );
        Pos = String1.find( String2, Pos + String3.length() );
    }

    return String1;
}

MySqlWrapper2::MySqlWrapper2(const char *host, const char *user, const char *password, const char *dbname, int port){
    if( !mysql_init(&m_mysql) ) throw "mysql_init";
    mysql_options(&m_mysql, MYSQL_SET_CHARSET_NAME, "utf8");
    if (mysql_real_connect(&m_mysql, host, user, password, dbname, port, NULL, 0) == NULL) throw mysql_error(&m_mysql);
}

MySqlWrapper2::~MySqlWrapper2(){
    mysql_close(&m_mysql);
}

MYSQL_RES *MySqlWrapper2::query(string sql) {
    if( mysql_query(&m_mysql, sql.c_str()) != 0 ){
		throw mysql_error(&m_mysql);
	}
	m_result = mysql_store_result(&m_mysql);
	return m_result;
}

MYSQL_ROW MySqlWrapper2::getRow() {
    return mysql_fetch_row( m_result );
}

