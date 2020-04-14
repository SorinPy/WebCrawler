#include "WebRequest.h"

WebRequest::WebRequest(boost::shared_ptr<boost::asio::io_context> io_context, boost::shared_ptr<boost::asio::ssl::context> ssl_context) : Connection(io_context,ssl_context)
{
	m_callback = NULL;
}

void WebRequest::LoadPage(boost::shared_ptr<Page> page)
{
	m_start_time = boost::chrono::high_resolution_clock::now();
	m_page = page;
	m_FullLink = m_page->getAddress();


	ParseLink();
	std::ostream req_stream(&m_request);
	req_stream << "GET ";
	req_stream << m_RequestQuery;
	req_stream << " HTTP/1.1\r\n";
	req_stream << "Accept-Encoding: None\r\n";
	req_stream << "Host: " << m_RequestHost << "\r\n";
	req_stream << "Accept: */*\r\n";
	req_stream << "Connection: close\r\n";
	req_stream << "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.122 Safari/537.36\r\n";
	req_stream << "\r\n";
	std::cout << m_RequestQuery << std::endl;
	std::cout << m_FullLink << std::endl;
	Connect(m_RequestHost, "https");
}

void WebRequest::LoadPage(boost::shared_ptr<Page> page, wr_callback cb)
{
	m_callback = cb;
	LoadPage(page);
}

void WebRequest::ParseLink()
{
	//using namespace std;

	int i = 0;
	int offset = 0;

	m_RequestHost.clear();
	m_RequestQuery.clear();
	try {
		while (i < m_FullLink.length())
		{
			m_FullLink[i] = tolower(m_FullLink[i]);
			i++;
		}
		if (m_FullLink.substr(0, 5) == "https")
		{
			std::string baseAddress = "https://";
			offset += 8;
			m_RequestType = HTTPS;

			int iRes = m_FullLink.find_first_of('/', offset);
			if (iRes == -1)
			{
				int iDomain = m_FullLink.find_last_of('.', m_FullLink.length());
				std::string Domain = m_FullLink.substr(iDomain + 1);
				int iSubDomain = m_FullLink.find_first_of('.', offset);

				std::string SubDomain, Name;
				if (iSubDomain == iDomain)
				{
					SubDomain = "";
					Name = m_FullLink.substr(offset, iSubDomain - offset);
				}
				else
				{
					SubDomain = m_FullLink.substr(offset, iSubDomain - offset);
					Name = m_FullLink.substr(iSubDomain + 1, iDomain - iSubDomain - 1);
				}
				if (SubDomain.length() > 0)
				{
					m_RequestHost.append(SubDomain);
					m_RequestHost.append(".");
				}
				m_RequestHost.append(Name);
				m_RequestHost.append(".");
				m_RequestHost.append(Domain);

				m_RequestQuery = "/";
			}
			else
			{
				std::string subStr = m_FullLink.substr(0, iRes);
				int iDomain = m_FullLink.find_last_of('.', subStr.length());
				std::string Domain = m_FullLink.substr(iDomain + 1, iRes - iDomain - 1);
				int iSubDomain = m_FullLink.find_first_of('.', offset);

				std::string SubDomain, Name;
				if (iSubDomain == iDomain)
				{
					SubDomain = "";
					Name = m_FullLink.substr(offset, iSubDomain - offset);
				}
				else
				{
					SubDomain = m_FullLink.substr(offset, iSubDomain - offset);
					Name = m_FullLink.substr(iSubDomain + 1, iDomain - iSubDomain - 1);
				}
				if (SubDomain.length() > 0)
				{
					m_RequestHost.append(SubDomain);
					m_RequestHost.append(".");
				}
				m_RequestHost.append(Name);
				m_RequestHost.append(".");
				m_RequestHost.append(Domain);

				m_RequestQuery = m_FullLink.substr(iRes);
			}
			baseAddress.append(m_RequestHost);
			m_page->setBaseAddress(baseAddress);

		}
		else if (m_FullLink.substr(0, 4) == "http")
		{
			std::string baseAddress = "http://";
			offset += 7;
			m_RequestType = HTTP;

			int iRes = m_FullLink.find_first_of('/', offset);
			if (iRes == -1)
			{
				int iDomain = m_FullLink.find_last_of('.', m_FullLink.length());
				std::string Domain = m_FullLink.substr(iDomain + 1);
				int iSubDomain = m_FullLink.find_first_of('.', offset);

				std::string SubDomain, Name;
				if (iSubDomain == iDomain)
				{
					SubDomain = "www";
					Name = m_FullLink.substr(offset, iSubDomain - offset);
				}
				else
				{
					SubDomain = m_FullLink.substr(offset, iSubDomain - offset);
					Name = m_FullLink.substr(iSubDomain + 1, iDomain - iSubDomain - 1);
				}
				m_RequestHost.append(SubDomain);
				m_RequestHost.append(".");
				m_RequestHost.append(Name);
				m_RequestHost.append(".");
				m_RequestHost.append(Domain);

				m_RequestQuery = "/";
			}
			else
			{
				std::string subStr = m_FullLink.substr(0, iRes);
				int iDomain = m_FullLink.find_last_of('.', subStr.length());
				std::string Domain = m_FullLink.substr(iDomain + 1, iRes - iDomain - 1);
				int iSubDomain = m_FullLink.find_first_of('.', offset);

				std::string SubDomain, Name;
				if (iSubDomain == iDomain)
				{
					SubDomain = "www";
					Name = m_FullLink.substr(offset, iSubDomain - offset);
				}
				else
				{
					SubDomain = m_FullLink.substr(offset, iSubDomain - offset);
					Name = m_FullLink.substr(iSubDomain + 1, iDomain - iSubDomain - 1);
				}
				m_RequestHost.append(SubDomain);
				m_RequestHost.append(".");
				m_RequestHost.append(Name);
				m_RequestHost.append(".");
				m_RequestHost.append(Domain);

				m_RequestQuery = m_FullLink.substr(iRes);
			}
			baseAddress.append(m_RequestHost);
			m_page->setBaseAddress(baseAddress);
		}
	}
	catch (std::exception ex)
	{
		std::cout << "[Exception][" << __FUNCTION__ << "]:" << ex.what() << std::endl;
	}
}

void WebRequest::OnConnect()
{
	//std::cout << "Connected" << std::endl;
}

void WebRequest::OnSend()
{
	Recv(m_response);
}

void WebRequest::OnRecv(size_t size)
{
	std::istream is(&m_response);
	std::ostream headers(&m_page->getHeadersBuff());
	std::ostream content(&m_page->getContentBuff());

	std::string result_line;
	while (!is.eof())
	{
		std::getline(is, result_line);
		//std::cout << result_line << std::endl;
		if (!m_page->headerLoaded())
		{
			if (result_line.length() < 3)
			{
				m_page->headerLoaded(true);
			}
			else {
				headers << result_line << std::endl;
			}
		}
		else {
			content << result_line << std::endl;
		}
	}

	Recv(m_response);
}

void WebRequest::OnHandshake()
{
	Send(m_request);
}

void WebRequest::OnDisconnect()
{
	m_end_time = boost::chrono::high_resolution_clock::now();
	//std::cout << "Request end:" << (double)boost::chrono::duration_cast<boost::chrono::microseconds>(m_end_time-m_start_time).count()/1000 << std::endl;
	if (m_callback != NULL)
		m_callback(shared_from_this(), (double)boost::chrono::duration_cast<boost::chrono::microseconds>(m_end_time - m_start_time).count() / 1000);
}
