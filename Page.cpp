#include "Page.h"

Page::Page(time_t add_date)
{
	m_addDate = boost::chrono::system_clock::from_time_t(add_date);
	m_headersLoaded = false;
}
