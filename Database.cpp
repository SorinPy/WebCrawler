#include "Database.h"

Database::Database() : m_Driver(NULL)
{
	
}

Database::Database(std::string server, std::string username, std::string password) : m_Driver(NULL)
{
	m_database_host = server;
	m_database_username = username;
	m_database_password = password;
	try {
		m_Driver = get_driver_instance();
	}
	catch(sql::SQLException &ex){
		std::cout << "[SQLException][" << __FUNCTION__ << "]:" << ex.getErrorCode() << " ," << ex.getSQLStateCStr() << std::endl;
	}
}

Database::~Database()
{
}

void Database::connect()
{
	boost::shared_ptr<sql::Statement> query;
	if (m_Connection == NULL || m_Connection->isClosed())
	{
		try
		{
			std::cout << "[" << __FUNCTION__ << "]: Connecting to MySQL on " << m_database_host << "('" << m_database_username << "','#')" << std::endl;
			m_Connection.reset(m_Driver->connect(m_database_host.c_str(), m_database_username.c_str(), m_database_password.c_str()));
		}
		catch (sql::SQLException ex)
		{
			std::cout << "[SQLException][" << __FUNCTION__ << "]:" << ex.getErrorCode() << " ," << ex.getSQLStateCStr() << std::endl;
		}
		if (m_Connection != NULL && m_Connection->isValid())
		{
			std::cout << "[" << __FUNCTION__ << "]:Connected to MySQL Database!" << std::endl;
			query.reset(m_Connection->createStatement());

			try
			{
				query->execute("USE `webcrawler`");
			}
			catch (sql::SQLException& ex)
			{
				std::cout << "[SQLException][" << __FUNCTION__ << "]:" << ex.getErrorCode() << " ," << ex.getSQLStateCStr() << std::endl;
			}
			m_Connection->setAutoCommit(false);
			m_Connection->setTransactionIsolation(sql::enum_transaction_isolation::TRANSACTION_READ_COMMITTED);
		}
	}
}

void Database::reconnect()
{
	if (m_Connection != NULL) {
		try {
			m_Connection->reconnect();
		}
		catch (sql::SQLException & ex)
		{
			std::cout << "[SQLException][" << __FUNCTION__ << "]:" << ex.getErrorCode() << " ," << ex.getSQLStateCStr() << std::endl;
		}
	}
	else {
		connect();
	}
}

void Database::insertPages(std::vector<boost::shared_ptr<Page>>& pages)
{
	if (!isConnected())
	{
		reconnect();
	}
	else {
		//milliseconds msStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		std::ostringstream query;

		while (!pages.empty())
		{
			boost::shared_ptr<sql::PreparedStatement> stmt;
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


				stmt.reset(m_Connection->prepareStatement(qAsStr.c_str()));
				stmt->execute();
				m_Connection->commit();
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
}

void Database::getPages(std::vector<boost::shared_ptr<Page>>& pages, size_t limit = 10)
{
	if (!isConnected())
	{
		reconnect();
	}
	else {
		boost::shared_ptr<sql::ResultSet> result;
		boost::shared_ptr<sql::Statement> query;
		try {

		
			query.reset(m_Connection->createStatement());
			
			query->execute("SET @uids := 0");

			query->execute(" UPDATE `link` SET `parse_date` = `parse_date`+1 WHERE `parse_date` < 1 AND(SELECT @uids := CONCAT_WS(',', `id`, @uids)) order by `id` asc limit 10");

			result.reset(query->executeQuery("SELECT `id`,`add_date`,`parse_date`,`address` from `link` where FIND_IN_SET(`id`,@uids)"));


			m_Connection->commit();
			
			if (result->rowsCount() > 0)
			{
				while (result->next())
				{
					std::string address = result->getString("address").c_str();
					int addDate = result->getInt(1);
					boost::shared_ptr<Page> tpage = boost::shared_ptr<Page>(new Page(addDate));
					tpage->setAddress(address);
					pages.push_back(tpage);
				}
			}
		}
		catch (sql::SQLException & ex)
		{
			std::cout << "[SQLException][" << __FUNCTION__ << "]:" << ex.getErrorCode() << " - " << ex.what() << std::endl;
		}
		catch (std::exception & ex)
		{
			std::cout << "[Exception][" << __FUNCTION__ << "]:" << ex.what() << std::endl;
		}
	}
}

void Database::updatePages(std::vector<boost::shared_ptr<Page>>&)
{
}

void Database::moveTempPages()
{
	if (!isConnected())
	{
		reconnect();
	}
	else {
		boost::shared_ptr<sql::Statement> query;
		try {

			query.reset(m_Connection->createStatement());
			query->execute("insert into link select id,add_date,parse_date , address from link_temp where NOT EXISTS(SELECT * FROM link WHERE (link_temp.address=link.address)) group by address");

			query->execute("DELETE FROM link_temp");

			m_Connection->commit();

		}
		catch (sql::SQLException & ex)
		{
			std::cout << "[SQLException][" << __FUNCTION__ << "]:" << ex.getErrorCode() << " - " << ex.what() << std::endl;
		}
		catch (std::exception & ex)
		{
			std::cout << "[Exception][" << __FUNCTION__ << "]:" << ex.what() << std::endl;
		}
	}
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