#pragma once

#include <sstream>

#define PRINT Output().get()
#define PRINT_ERR Output().get(Output::ERROR)
#define PRINT_WARNING Output().get(Output::WARNING)
#define PRINT_INFO Output().get(Output::INFO)
#define PRINT_DEBUG Output().get(Output::DEBUG)

#define LOG Output(Output::Type::LOGGER).get()
#define LOG_ERR Output(Output::Type::LOGGER).get(Output::ERROR)
#define LOG_WARNING Output(Output::Type::LOGGER).get(Output::WARNING)
#define LOG_INFO Output(Output::Type::LOGGER).get(Output::INFO)
#define LOG_DEBUG Output(Output::Type::LOGGER).get(Output::DEBUG)

class Output {
	public:
		enum Level {
			DEFAULT,
			ERROR,
			WARNING,
			INFO,
			DEBUG
		};

		enum class Type {
			CONSOLE,
			LOGGER
		};

		typedef std::ostringstream Stream;

	private:
		Output(Output const &);
		Output& operator=(Output const &);

	public:
		Output(Type type = Type::CONSOLE);
		~Output();

		std::ostringstream& get(Level level = Output::DEFAULT);
		static void clearLog();

	private:
		void log() const;
		void console() const;
		void print(const std::string& color, std::ostream& out) const;
		std::string getLevelString() const;

	private:
		static constexpr const char* LOGFILE = "./result.log";

	private:
		std::ostringstream os;
		Level level;
		Type type;
};
