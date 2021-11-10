#include <string>
#include <iostream>

bool	isHostMatchFrontWildCard(std::string const & host, std::string const & wildcard)
{
	std::string	to_find_string = wildcard.substr(1);
	std::size_t	to_find_size = to_find_string.size();
	std::size_t	size = host.size();
	if (size < to_find_size)
	{
		return false;
	}
	return host.compare(size - to_find_size, to_find_size, to_find_string) == 0;
}

int main(int argc, char* argv[])
{
    std::string host = argv[1];
    std::string wildcard = argv[2];
    if (isHostMatchFrontWildCard(host, wildcard))
    {
        std::cout << "Match front!" << std::endl;
    }
}
