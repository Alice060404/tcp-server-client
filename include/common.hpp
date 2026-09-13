#include <string_view>
namespace common
{
namespace exception
{
void throw_if(bool condition, std::string_view msg);
} // namespace exception
} // namespace common
