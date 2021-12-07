#pragma once

# include <string>
# include <vector>
# include <utility>

struct Request;
struct ConfigInfo;
class FdTable;

class CgiExecutor
{
	public:
		typedef std::pair<std::string, std::string> MetaVariableType;
		typedef std::vector<MetaVariableType> MetaVariableContainerType;
		typedef std::vector<std::pair<std::string, std::string> > CgiVectorType;

	private:
		CgiExecutor(CgiExecutor const & rhs);
		CgiExecutor& operator=(CgiExecutor const & rhs);

	public:
		CgiExecutor();
		~CgiExecutor();
		int execute(FdTable & fd_table, Request & request, int* cgi_fds);
		int getStatusCode() const;
		void clear();

	private:
		std::string resolveCgiScript(std::string const target, CgiVectorType const & cgi);

		MetaVariableContainerType generateMetaVariables(const Request& request);
		void metaVariableContent(const Request& request, MetaVariableContainerType & meta_variables);
		void metaVariablePathInfo(const Request& request, MetaVariableContainerType & meta_variables);
		void metaVariableHeader(const Request& request, MetaVariableContainerType & meta_variables);
		MetaVariableType convertFieldToMeta(
				const std::string& key, const std::string& value) const;

		bool canBeExecuted(std::string const & script);
		int forkCgi(int* cgi_fds, FdTable& fd_table, std::string const & script, ConfigInfo const & info);
		int executeChildProcess(std::string const & script, ConfigInfo const & info) const;
		int prepareArguments(char *args[3], std::string const & script, ConfigInfo const & info) const;
		int prepareCgi(int* cgi_fds, FdTable& fd_table) const;
		int closeAll(FdTable& fd_table) const;
		int setEnvironment() const;
		int setRedirection(int* cgi_fds) const;

		int clearCgi();
		int killCgi(int* status);

		void setStatus(int status_code);

	/* Debugging */
		void print(MetaVariableContainerType const & meta_variables, std::string const & script, ConfigInfo const & info) const;

	private:
		int _status_code;
		pid_t _cgi_pid;
		MetaVariableContainerType _meta_variables;
};
