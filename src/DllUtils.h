#pragma once
#include <boost/dll.hpp>
#include <functional>
#include <memory>

template<class TFunc>
bool tryLoad(std::unique_ptr<boost::dll::shared_library>& lib, const std::string& symbol, std::function<TFunc>& result) {
	try {
		result = lib->get<TFunc>(symbol);
		return (bool)result; //This looks dumb. But I did it because result wasn't being implicitly converted to bool
							 //it still looks dumb. 
							 //return (bool)result;
	}
	catch (const boost::system::system_error&) {
		return false;
	}

}