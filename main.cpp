#include "file_system/file_system.hpp"

#include "unit_tests.hpp"

int main()
{
    using FATFileSystem = file_system::fat::FileSystem<1 << 30>;
    auto fat_file_system = file_system::make_file_system<FATFileSystem>();
    file_system::FileSystem<FATFileSystem>* file_system = fat_file_system;
    file_system::File* file = file_system->create_file("test_1.txt");
    file_system->write_file(file, {'H','e','l','l','o',' ','w','o','r','l','d'});
    file_system->remove_file("test_1.txt");;
    return EXIT_SUCCESS;
}
