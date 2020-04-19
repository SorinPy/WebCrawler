#include "Parser.h"


void Parser::ParsePage(boost::shared_ptr<Page> page, parser_callback cb)
{
	m_start_time = boost::chrono::high_resolution_clock::now();
	m_page = page;
	m_cb = cb;

	std::vector<std::string> urls;

	std::string content((std::istreambuf_iterator<char>(&page->getContentBuff())), std::istreambuf_iterator<char>());

	urls = searchLinks(content);
	for (auto& url : urls)
	{
		if (url.length() > 0 && url.substr(0, 4) != "http")
		{
			if (url.at(0) == '#')
			{
				continue;
			}
			if (url.at(0) != '/')
			{
				url = '/' + url;
			}
			url = page->getBaseAddress() + url;
		}
	}

	std::vector<boost::shared_ptr<Page>> ToAdd;
	for (auto url : urls)
	{
		if (url.length() > 0)
		{
			auto tpage = boost::shared_ptr<Page>(new Page());
			tpage->setAddress(url);
			ToAdd.push_back(tpage);
		}
	}
	
	m_end_time = boost::chrono::high_resolution_clock::now();

	if (cb != NULL)
	{
		cb(shared_from_this(), (double)boost::chrono::duration_cast<boost::chrono::microseconds>(m_end_time - m_start_time).count() / 1000, ToAdd);
	}

}

std::vector<std::string> Parser::searchLinks(std::string content)
{
	/*
	int searchOffset = 0;
	int spos = 0, epos;

	while (spos >= 0)
	{
		spos = content.find("href=", searchOffset, 5);
		if (spos >= 0)
		{
			spos += 5;
			char schar = content.at(spos);
			std::cout << "Char at pos :" << schar << std::endl;
			int secondPos = content.find(schar, spos + 1);

			//epos = content.find(content.at(spos), spos + 1);
			std::string link = content.substr(spos -5, secondPos - spos +5);
			ret.push_back(link);
			std::cout << "Link:" << link << std::endl;
			searchOffset = secondPos + 1;
		}
	}*/
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
	}
	return ret;
}