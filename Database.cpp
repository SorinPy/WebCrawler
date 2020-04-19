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
			m_Connection->setAutoCommit(true);
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
		
		boost::shared_ptr<sql::Statement> query;
		boost::shared_ptr<sql::ResultSet> result;

		
		while (!pages.empty())
		{
			
			
			try {
				std::ostringstream queryString;
				query.reset(m_Connection->createStatement());
				int i = 0;
				queryString << "INSERT INTO `link_temp` (`add_date`,`address`) VALUES ";

				while (!pages.empty() && i < 50)
				{
					i++;
					boost::shared_ptr<Page> page = pages.back();
					pages.pop_back();
					if (page->getAddress().length() > 254)
					{
						i--;
						std::cout << page->getAddress() << std::endl;
						continue;
					}
					queryString << boost::format("('%1%','%2%')") % page->getAddDate() % page->getAddress();
					if (!pages.empty() && i < 50)
					{
						queryString << ",";
					}
				}

				std::string qAsStr = queryString.str();
				query->execute(qAsStr.c_str());
				
				//m_Connection->commit();
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


			//m_Connection->commit();
			
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

			query->executeQuery("DELETE FROM link_temp");

			//m_Connection->commit();

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

DELIMITER |
CREATE EVENT e_every_10_mins
	ON SCHEDULE
	  EVERY 10 MINUTE
	COMMENT 'Clears out temp link table every 10 mins.'
	DO BEGIN
	  insert into link select id,add_date,parse_date , address from link_temp where NOT EXISTS(SELECT * FROM link WHERE (link_temp.address=link.address)) group by address;
	  DELETE FROM link_temp;
	END |
DELIMITER ;
*/