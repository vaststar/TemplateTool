#include <exception>
#include <iostream>

namespace sablelog::test {

void runPublicApiTests();
void runFileBehaviorTests();

} // namespace sablelog::test

int main()
{
    try
    {
        sablelog::test::runPublicApiTests();
        sablelog::test::runFileBehaviorTests();
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "[FAIL] " << error.what() << '\n';
    }
    catch (...)
    {
        std::cerr << "[FAIL] unexpected non-standard exception\n";
    }

    return 1;
}
