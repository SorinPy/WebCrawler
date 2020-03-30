#include "Database.h"

Database::Database()
{
	m_Driver = get_driver_instance();
	try
	{
		m_Connection = m_Driver->connect("tcp://127.0.0.1:3306", "root", "");
	}
	catch (sql::SQLException ex)
	{
		std::cout << "[SQLException][" << __FUNCTION__ << "]:" << ex.getErrorCode() << " ," << ex.getSQLStateCStr() << std::endl;
	}
	if (m_Connection != NULL && m_Connection->isValid())
	{
		std::cout << "[" << __FUNCTION__ << "]:Conexiune cu serverul reusita!" << std::endl;
		m_Query = m_Connection->createStatement();

		try
		{
			m_Query->execute("USE `webcrawler`");
		}
		catch (sql::SQLException ex)
		{
			std::cout << "[SQLException][" << __FUNCTION__ << "]:" << ex.getErrorCode() << " ," << ex.getSQLStateCStr() << std::endl;
		}
		m_Connection->setAutoCommit(true);
		//m_Connection->setTransactionIsolation(sql::enum_transaction_isolation::TRANSACTION_READ_COMMITTED);
	}
}

Database::~Database()
{
	delete m_Connection;
	delete m_Query;
}

void Database::insertPages(std::vector<boost::shared_ptr<Page>>& pages)
{
	sql::PreparedStatement* stmt;
	//milliseconds msStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	std::ostringstream query;
	
	while (!pages.empty())
	{
		try {
			int i = 0;
			query << "INSERT INTO `link_temp` (`add_date`,`address`) VALUES ";

			while (!pages.empty() && i < 50)
			{
				i++;
				boost::shared_ptr<Page> page = pages.back();
				pages.pop_back();
				query << boost::format("('%1%','%2%')") % page->getAddDate() % page->getAddress();
				if (!pages.empty() && i < 50)
				{
					query << ",";
				}
			}

			std::string qAsStr = query.str();


			stmt = m_Connection->prepareStatement(qAsStr.c_str());
			stmt->execute();
			delete stmt;
			query.clear();
		}
		catch (sql::SQLException & ex)
		{
			std::cout << "[SQLException][" << __FUNCTION__ << "]:" << ex.getErrorCode() << " ," << ex.getSQLStateCStr() << std::endl;
		}
		catch (std::exception & ex)
		{
			std::cout << "[Exception][" << __FUNCTION__ << "]:" << ex.what() << std::endl;
		}
	}
}

void Database::getPages(std::vector<boost::shared_ptr<Page>>& pages, size_t limit = 10)
{
	sql::PreparedStatement* stmt;
	sql::ResultSet* result = NULL;

	try {
		stmt = m_Connection->prepareStatement("SELECT add_date,address FROM `link` where `parse_date` = 0 LIMIT ?");
		stmt->setInt(1, limit);
		stmt->execute();
		result = stmt->getResultSet();
		if (result->rowsCount() > 0)
		{
			while (result->next())
			{
				try {
					std::string address = result->getString(2).c_str();
					int addDate = result->getInt(1);
					boost::shared_ptr<Page> tpage = boost::shared_ptr<Page>(new Page(addDate));
					tpage->setAddress(address);
					pages.push_back(tpage);
				}
				catch (sql::SQLException& ex)
				{
					std::cout << "[SQLException][" << __FUNCTION__ << "]:" << ex.getErrorCode() << " - " << ex.what() << std::endl;
				}
				catch (std::exception& ex)
				{
					std::cout << "[Exception][" << __FUNCTION__ << "]:" << ex.what() << std::endl;
				}
				catch (...)
				{
					std::cout << "[Exception][" << __FUNCTION__ << "]: Access violation!" << std::endl;
				}

			}
		}
	}
	catch (sql::SQLException ex)
	{
		std::cout << "[SQLException][" << __FUNCTION__ << "]:" << ex.getErrorCode() << " - " << ex.what() << std::endl;
	}
	delete result, stmt;

}

/*
SQL Move from temp to link table
insert into link
select id,add_date,parse_date , address
from link_temp
where NOT EXISTS(SELECT *
				 FROM link
				 WHERE (link_temp.address=link.address)
				 )
group by address;
Delete from link_temp
*/