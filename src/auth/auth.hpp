#pragma once

#include <boost/property_tree/ptree_fwd.hpp>

namespace authentication
{
	void on_login(const std::string& ssl_id, const boost::property_tree::ptree& arguments, boost::property_tree::ptree& resp);
}