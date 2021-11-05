#pragma once

#include "fd/AFdInfo.hpp"
#include "parser/HeaderField.hpp"

/*
Purpose of the class:
    - Read the output from the CGI program
    - Parse the CGI response into message_body and header
    - Set flags when the CGI request is finished
*/
class CgiReader : public AFdInfo
{
    public:
        CgiReader(int fd);
        ~CgiReader();

        struct pollfd getPollFd() const;
		int	writeEvent(FdTable & fd_table);
		int	readEvent(FdTable & fd_table);

    private:
        std::string _message_body;
        HeaderField _header;
};
