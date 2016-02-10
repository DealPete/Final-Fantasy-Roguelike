#include <string>

#include "Misc.h"

const unsigned int MESSAGE_BUFFER_SIZE = 1000;

extern buffer help_buffer;
extern buffer message_buffer;

void add_message(const std::string&);
void add_message(const char*, ...);
void error(const std::string msg);
void error(const char* format, ...);
void format_string(buffer&, std::string, unsigned int width);
void flush_message_buf(bool more_prompt = false);
void message(const std::string&);
void message(const char*, ...);
void more(const std::string&);
void more(const char*, ...);
void prompt(const std::string&);
void prompt_input(const std::string&, std::string& input);
bool yesno();
