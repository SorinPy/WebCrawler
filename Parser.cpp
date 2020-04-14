#include "Parser.h"

void Parser::ParsePage(boost::shared_ptr<Page> page, parser_callback cb)
{
	m_start_time = boost::chrono::high_resolution_clock::now();
	m_page = page;
	m_cb = cb;

	std::vector<std::string> urls;

	std::string content((std::istreambuf_iterator<char>(&page->getContentBuff())), std::istreambuf_iterator<char>());

	std::cout << content << std::endl;
	urls = searchLinks(content);
	for (auto& url : urls)
	{
		if (url.length() > 0 && url.substr(0, 4) != "http")
		{
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
		cb(shared_from_this(), (double)boost::chrono::duration_cast<boost::chrono::microseconds>(m_end_time - m_start_time).count() / 1000, ToAdd);

}

std::vector<std::string> Parser::searchLinks(std::string content)
{
	std::vector<std::string> ret;
	int searchOffset = 0;
	int spos = 0, epos;

	while (spos >= 0)
	{
		spos = content.find("href=", searchOffset, 5);
		if (spos >= 0)
		{
			spos += 5;
			while (content.at(spos) == ' ')
			{
				spos++;
			}
			epos = content.find(content.at(spos), spos + 1);
			ret.push_back(content.substr(spos + 1, epos - spos - 1));
			searchOffset = epos + 1;
		}
	}

	return ret;
}