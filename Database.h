#pragma once

#include <cppconn\driver.h>
#include <cppconn\connection.h>
#include <cppconn\exception.h>
#include <cppconn\statement.h>
#include <cppconn\resultset.h>
#include <cppconn\prepared_statement.h>

#include <vector>

#include "Page.h"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/system/error_code.hpp>


class Database
{
public:

	Database();
	Database(std::string, std::string, std::string);

	~Database();


	void connect();

	void reconnect();

	void insertPages(std::vector<boost::shared_ptr<Page>>&);

	void getPages(std::vector<boost::shared_ptr<Page>>&, size_t);

	void updatePages(std::vector<boost::shared_ptr<Page>>&);

	void moveTempPages();

	bool isConnected() { if (m_Connection == NULL) return false; return !m_Connection->isClosed(); }
private:
	sql::Driver* m_Driver;
	boost::shared_ptr<sql::Connection> m_Connection;

	std::string m_database_host;
	std::string m_database_username;
	std::string m_database_password;
};

