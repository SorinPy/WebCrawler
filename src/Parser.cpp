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
	
	std::string content;


	parseHeaders();

	if (m_page->StatusCode == 200)
	{
		auto encoding = m_page->ResponseHeaders.find("transfer-encoding");
		if (encoding != m_page->ResponseHeaders.end())
		{
			if (encoding->second == "chunked")
			{
				//content = std::string((std::istreambuf_iterator<char>(&page->getContentBuff())), std::istreambuf_iterator<char>());
				//std::cout << content;

				std::istream is(&m_page->getContentBuff());
				std::string line = "";
				int chunk_size = 0;
				int chunk_parsed = 0;
				std::stringstream ss;



				while (!is.eof())
				{
					std::getline(is, line);
					//std::cout <<chunk_parsed << "/" << chunk_size << " - " << content.length() << std::endl;
					if (chunk_parsed < chunk_size)
					{
						chunk_parsed += line.length() + 1;
						boost::trim_right(line);
						content += line;
						
					}
					else {
						chunk_size = 0;
						chunk_parsed = 0;
						
						if (line.length() > 0)
						{
							ss << std::hex << line;
							ss >> chunk_size;
							if (chunk_size == 0)
								break;
						}
						else
						{
							is.seekg(0, std::ios::end);
						}
					}
				} 
			}
		}else
		{
			content = std::string((std::istreambuf_iterator<char>(&page->getContentBuff())), std::istreambuf_iterator<char>());
		}
		
		//std::string content((std::istreambuf_iterator<char>(&page->getContentBuff())), std::istreambuf_iterator<char>());


		searchLinks(content, urls);
	}
	else if (m_page->StatusCode == 301 
		|| m_page->StatusCode == 302
		|| m_page->StatusCode == 307
		|| m_page->StatusCode == 308)
	{
		auto location = m_page->ResponseHeaders.find("location");
		if (location != m_page->ResponseHeaders.end())
		{
			urls.push_back(m_page->ResponseHeaders["location"]);
		}
	}

	for (std::string& el : urls)
	{
		if (el.find("http", 0, 4) == std::string::npos)
		{
			if (el.at(0) != '/')
			{
				el = '/' + el;
			}
			el = page->getBaseAddress() + el;
		}
		if (m_followOnlyDomain && el.find(m_page->MainDomain) != std::string::npos)
		{
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


			while (!headers.eof())
			{
				std::getline(headers, line);
				separatorPos = line.find_first_of(": ");
				if (separatorPos != std::string::npos)
				{
					std::string header = line.substr(0, separatorPos);
					std::string value = line.substr(separatorPos + 2);
					boost::algorithm::to_lower(header);
					boost::algorithm::trim(value);
					m_page->ResponseHeaders[header] = value;
				}
			}
		}
	}
}

void Parser::searchLinks(std::string content , std::vector<std::string> & ret)
{
	size_t searchOffset = 0;
	size_t spos = 0;


	while (spos != std::string::npos)
	{
		spos = content.find("href=\"", searchOffset, 5);
		if (spos != std::string::npos)
		{
			spos += 5;
			//char schar = content.at(spos);
			size_t secondPos = content.find('"', spos + 1);

			//epos = content.find(content.at(spos), spos + 1);
			if (secondPos != std::string::npos)
			{
				std::string link = content.substr(spos + 1, secondPos - spos - 1);

				if (isValidUrl(link))
				{
					link.erase(std::remove(link.begin(), link.end(), '\n'), link.end());
					if (m_followOnlyDomain)
					{
						if (link.find(m_page->MainDomain.c_str(), 0, m_page->MainDomain.length()))
						{
							ret.push_back(link);
						}
					}
					else {
						ret.push_back(link);
					}
				}
				searchOffset = secondPos + 1;
			}
			else {
				break;
			}
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
	if (data.find('#') != std::string::npos)
		return false;
	if (data.find('\'') != std::string::npos || data.find('\\') != std::string::npos)
		return false;
	for (const std::string& filter : filter_ends)
		if (boost::algorithm::ends_with(data, filter))
			return false;
	return true;
}
