#include "HeaderField.hpp"

bool HeaderField::case_insensitive_less::operator() (const std::string& x, const std::string& y) const
{
	for (std::size_t i = 0; i < x.size() && i < y.size(); ++i) {
		if (toupper(x[i]) != toupper(y[i])) {
			return toupper(x[i]) < toupper(y[i]);
		}
	}
	return x.size() < y.size();
}

HeaderField::value_type& HeaderField::operator[](const key_type& key)
{
	return _map[key];
}

bool HeaderField::contains(const key_type& key)
{
	_cached_iterator = _map.find(key);
	return _cached_iterator == _map.end();
}

bool HeaderField::contains(const key_type& key) const
{
	return _map.find(key) == _map.end();
}

HeaderField::value_type& HeaderField::get()
{
	return _cached_iterator->second;
}

HeaderField::value_type& HeaderField::get(const key_type& key)
{
	return _map.find(key)->second;
}

const HeaderField::value_type& HeaderField::get(const key_type& key) const
{
	return _map.find(key)->second;
}

/* Map Accessors */

HeaderField::iterator HeaderField::find(const key_type& key)
{
	return _map.find(key);
}

HeaderField::const_iterator HeaderField::find(const key_type& key) const
{
	return _map.find(key);
}

HeaderField::iterator HeaderField::begin()
{
	return _map.begin();
}

HeaderField::const_iterator HeaderField::begin() const
{
	return _map.begin();
}

HeaderField::iterator HeaderField::end()
{
	return _map.end();
}

HeaderField::const_iterator HeaderField::end() const
{
	return _map.end();
}

HeaderField::size_type HeaderField::size() const
{
	return _map.size();
}

void HeaderField::clear()
{
	_map.clear();
}
