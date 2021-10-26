#pragma once

#include <map>
#include <string>
#include <functional>

/*
Functionality:
	- operator[key] = value
	- bool contains(key)
	- value_type& get(); Returns the value of the key foundin call to last contains(key)
	- pair_type get(key):
		pair_type.first: iterator to element if it exists
		pair_type.second: bool that's true if the element was present, otherwise false
*/


class HeaderField
{
private:
	struct case_insensitive_less
		: public std::binary_function<std::string, std::string, bool>
	{
		bool operator() (const std::string& x, const std::string& y) const;
	};

public:
	typedef std::map<std::string, std::string, case_insensitive_less> map_t;

	typedef std::string key_type;
	typedef std::string value_type;

	typedef map_t::iterator iterator;
	typedef map_t::const_iterator const_iterator;
	typedef map_t::size_type size_type;
	typedef std::pair<iterator, bool> pair_type;
	typedef std::pair<const_iterator, bool> const_pair_type;

public:
	value_type& operator[](const key_type& key);
	bool contains(const key_type& key);
	bool contains(const key_type& key) const;
	value_type& get();
	pair_type get(const key_type& key);
	const_pair_type get(const key_type& key) const;

/* Regular Map Accessors */
	iterator find(const key_type& key);
	const_iterator find(const key_type& key) const;

	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

	size_type size() const;
	void clear();

private:
	map_t _map;
	iterator _cached_iterator;
};
