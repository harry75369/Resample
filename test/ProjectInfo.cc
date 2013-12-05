#include "Config.hpp"

int main()
{
  INFO("========== Project Infomation  ==========");
  INFO("Project Name: %s", XSTR(PROJECT_NAME));
  INFO("Project Version: %s.%s", XSTR(PROJECT_VERSION_MAJOR), XSTR(PROJECT_VERSION_MINOR));
  INFO("Build Date: %s", XSTR(BUILD_DATE));
  INFO("Author: Chaoya Li <harry75369@gmail.com>");
  WARN("This project is protected by Apache License!");
  return 0;
}
