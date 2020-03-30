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

class Database
{
public:

	Database();
	~Database();

	void insertPages(std::vector<boost::shared_ptr<Page>>&);

	void getPages(std::vector<boost::shared_ptr<Page>>&, size_t);

private:
	sql::Driver* m_Driver;
	sql::Connection* m_Connection;
	sql::Statement* m_Query;
};

