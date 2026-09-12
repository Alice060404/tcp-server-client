#include <cerrno>
#include <string_view>
#include <system_error>

namespace common
{
namespace exception
{
void throw_if(bool condition, std::string_view msg)
{
    if (condition)
    {
        throw std::system_error(errno, std::generic_category(), std::string(msg));
    }
}
} // namespace exception
} // namespace common
