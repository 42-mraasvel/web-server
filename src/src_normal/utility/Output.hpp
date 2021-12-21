#pragma once

#include "utility/color.hpp"
#include <sstream>

#define PRINT Output().get()
#define PRINT_ERR Output().get(Output::ERROR)
#define PRINT_WARNING Output().get(Output::WARNING)
#define PRINT_INFO Output().get(Output::INFO)
#define PRINT_DEBUG Output().get(Output::DEBUG)

class Output {
	public:
		enum Level {
			DEFAULT,
			ERROR,
			WARNING,
			INFO,
			DEBUG
		};

	private:
		Output(Output const &);
		Output& operator=(Output const &);

	public:
		Output();
		~Output();

		std::ostringstream& get(Level l = Output::DEFAULT);

	private:
		void log() const;
		std::string getLevelString() const;

	private:
		std::ostringstream os;
		Level level;
};
