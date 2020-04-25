#include "Parser.h"
#include "Parser.h"


void Parser::ParsePage(boost::shared_ptr<Page> page, parser_callback cb)
{

	m_start_time = boost::chrono::high_resolution_clock::now();
	m_page = page;
	m_cb = cb;

	std::vector<std::string> urls;
	std::vector<boost::shared_ptr<Page>> ToAdd;

	//std::string headers((std::istreambuf_iterator<char>(&page->getHeadersBuff())), std::istreambuf_iterator<char>());
	std::string content((std::istreambuf_iterator<char>(&page->getContentBuff())), std::istreambuf_iterator<char>());

	
	parseHeaders();
	


	if (m_page->StatusCode == 200)
	{
		
		searchLinks(content , urls);
		
		for (std::string& el : urls)
		{
			if (el.find("http", 0, 4))
			{
				if (el.at(0) != '/')
				{
					el = '/' + el;
				}
				el = page->getBaseAddress() + el;
			}
			auto tpage = boost::shared_ptr<Page>(new Page());
			tpage->setAddress(el);
			ToAdd.push_back(tpage);
		}
	}

	m_end_time = boost::chrono::high_resolution_clock::now();
	if (cb != NULL)
	{
		cb(shared_from_this(), (double)boost::chrono::duration_cast<boost::chrono::microseconds>(m_end_time - m_start_time).count() / 1000, ToAdd);
	}

}

void Parser::parseHeaders()
{
	std::istream headers(&m_page->getHeadersBuff());
	std::string line;

	int separatorPos;
	int sec_pos;

	std::getline(headers, line);
	separatorPos = line.find_first_of(" ");
	if (separatorPos != std::string::npos)
	{
		sec_pos = line.find_first_of(" ", separatorPos + 1);
		if (sec_pos != std::string::npos)
		{
			std::string statusCode = line.substr(separatorPos + 1, sec_pos - separatorPos - 1);
			m_page->StatusCode = boost::lexical_cast<int>(statusCode);

			if (m_page->StatusCode == 200)
			{
				while (!headers.eof())
				{
					std::getline(headers, line);
					separatorPos = line.find_first_of(": ");
					if (separatorPos != std::string::npos)
					{
						m_page->ResponseHeaders[line.substr(0, separatorPos)] = line.substr(separatorPos + 2);
					}
				}
			}
		}
	}
}

void Parser::searchLinks(std::string content , std::vector<std::string> & ret)
{
	int searchOffset = 0;
	int spos = 0;

	/*
	try {
		std::regex pattern("<[a]+\\s+(?:[^>]*?\\s+)?[href]+=[\"']([^\"']*)['\"]");
		std::smatch match;
		//boost::regex forbiden("/([\'\\])/s");

		std::string fstr = R"reg((^#|[\'\\]))reg";
		std::regex forbiden( fstr );

		

		std::string::const_iterator searchStart(content.cbegin());

		while (std::regex_search(searchStart, content.cend(), match, pattern))
		{
			std::string link(match[1].str());
			//if(link.length() < 255)
			{
				//link.erase(std::remove(link.begin(), link.end(), '\n'), link.end());
				//if (!std::regex_search(link, forbiden))
				{
					ret.push_back(link);
				}
			}
			searchStart = match.suffix().first;
		}
	}
	catch (std::exception & ex)
	{
		std::cout << "[Exception][" << __FUNCTION__ << "] " << ex.what() << std::endl;
	}
	*/
	
	while (spos >= 0)
	{
		spos = content.find("href=\"", searchOffset, 5);
		if (spos >= 0)
		{
			spos += 5;
			//char schar = content.at(spos);
			int secondPos = content.find('"', spos + 1);

			//epos = content.find(content.at(spos), spos + 1);
			if (secondPos - 1 > spos + 1)
			{
				std::string link = content.substr(spos + 1, secondPos - spos - 1);

				if (isValidUrl(link))
				{
					ret.push_back(link);
				}
			}
			searchOffset = secondPos + 1;
		}
	}
	
	/*
	std::vector<std::string> ret;

	std::regex e("<a\\s[^>]*href=(\"?)([^\" >]*?)\\1[^>]*>(.*?)</a>");

	std::smatch m;

	try {
		while (std::regex_search(content, m, e)) {
			std::string url = m[2];
			if (url.at(0) != '#')
			{
				ret.push_back(m[2]);
			}
			content = m.suffix().str();
		}
	}
	catch (std::exception& ex)
	{
		std::cout << "[Exception][" << __FUNCTION__ << "]: " << ex.what() << std::endl;
	}*/
}

bool Parser::isValidUrl(const std::string& data)
{
	auto filter_ends = { "gif","jpeg","css","js","png","rar","jpg" };

	if (data.length() > 254 || data.length() == 0)
		return false;
	if (data.at(0) == '#')
		return false;
	if (data.find('\'') != std::string::npos || data.find('\\') != std::string::npos)
		return false;
	for (const std::string& filter : filter_ends)
		if (boost::algorithm::ends_with(data, filter))
			return false;
	return true;
}
