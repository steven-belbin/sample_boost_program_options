// test_boost_prg_options.cpp : Defines the entry point for the console application.
//

#include "pch.h"

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>

namespace filesystem = std::experimental::filesystem::v1;

enum class LogSeverityLevel : std::int8_t { E_LOG_TRACE = 0,
                                            E_LOG_DEBUG,
                                            E_LOG_INFO,
                                            E_LOG_WARN,
                                            E_LOG_ERROR};

const std::vector<std::string>& LogSeverityLevelTags = { "TRACE", "DEBUG", "INFO", "WARNING", "ERROR" };

void validate(boost::any& v,
              const std::vector<std::string>& values,
              LogSeverityLevel*, int)
{
    using namespace boost::program_options;

    // Make sure no previous assignment to 'a' was made.
    validators::check_first_occurrence(v);

    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = validators::get_single_string(values);

    const std::string& s_capitalized = boost::to_upper_copy(s);

    if (s_capitalized== "ERROR") {
        v = boost::any(LogSeverityLevel::E_LOG_ERROR);
    } else if (s_capitalized == "WARN") {
        v = boost::any(LogSeverityLevel::E_LOG_WARN);
    } else if (s_capitalized == "INFO") {
        v = boost::any(LogSeverityLevel::E_LOG_INFO);
    } else if (s_capitalized == "DEBUG") {
        v = boost::any(LogSeverityLevel::E_LOG_DEBUG);
    } else if (s_capitalized == "TRACE") {
        v = boost::any(LogSeverityLevel::E_LOG_TRACE);
    } else {
        throw validation_error(validation_error::invalid_option_value);
    }
}

int main(int argc, char* argv[])
{
   using namespace boost::program_options;

   options_description common_options_description;
   common_options_description.add_options()
      ("owner.name", value<std::string>()->default_value("bob"), "Owner's name.")
      ("owner.sex", value<std::string>()->default_value("female"), "Owner's sex.")
      ("owner.age", value<int>()->default_value(18), "Owner's age.")
      ("owner.has_pet", value<bool>()->default_value(true), "Owner's has pet.")
      ("animal.type", value<std::string>()->default_value("cat"), "Type of animal.")
      ("animal.pet_name", value<std::string>()->default_value("minou"), "Animal pet name.")
      ("logging", value<LogSeverityLevel>()->default_value(LogSeverityLevel::E_LOG_DEBUG, "DEBUG"), "Logging level");

   options_description cmd_line_options_description;
   cmd_line_options_description.add(common_options_description);
   cmd_line_options_description.add_options()
      ("version,v", "Print the version information.")
      ("help,h", "Print the help information.")
      ("configuration_file", value<std::string>()->default_value("foobar.ini"), "Configuration file to load the options");

   options_description cfg_file_options_description;
   cfg_file_options_description.add(common_options_description);

   variables_map variables;

   try
   {
      store(parse_command_line(argc, argv, cmd_line_options_description), variables);

      const variable_value& cfg_file_variable = variables["configuration_file"];
      const filesystem::path& cfg_file_path = cfg_file_variable.as<std::string>();

      if (!cfg_file_variable.defaulted() || filesystem::exists(cfg_file_path))
      {
         std::ifstream cfg_file(cfg_file_path.string().c_str());
         store(parse_config_file(cfg_file, cfg_file_options_description), variables);
      }

      notify(variables);

      for (decltype(auto) variable : variables)
      {
         const auto& variable_name = variable.first;
         const auto& variable_value = variable.second;
         decltype(auto) value = variable_value.value();

         std::cout << "name: " << variable_name << ", "
                   << "is_defaulted: " << variable_value.defaulted() << ", "
                   << "value: ";

         if (boost::any_cast<int>(&value))
         {
            std::cout << boost::any_cast<int>(value);
         }
         else if (boost::any_cast<double>(&value))
         {
            std::cout << boost::any_cast<double>(value);
         }
         else if (boost::any_cast<std::string>(&value))
         {
            std::cout << boost::any_cast<std::string>(value);
         }
         else if (boost::any_cast<LogSeverityLevel>(&value))
         {
            std::cout << LogSeverityLevelTags[static_cast<size_t>(boost::any_cast<LogSeverityLevel>(value))];
         }

         std::cout << std::endl;
      }
   }
   catch (const std::exception& ex)
   {
      std::cout << ex.what() << std::endl;
   }

   return 0;
}

