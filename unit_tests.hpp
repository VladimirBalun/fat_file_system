#pragma once

#include "file_system/fat/fat_file_system.hpp"
#include "file_system/fat/fat_memory_manager.hpp"

inline void test_fat_cluster_size_calculation();
inline void test_fat_calculate_file_allocation_table_size();
inline void test_fat_memory_manager();
inline void test_file_system();

inline void run_unit_tests()
{
    test_fat_cluster_size_calculation();
    test_fat_calculate_file_allocation_table_size();
    test_fat_memory_manager();
    test_file_system();
}

inline void test_fat_cluster_size_calculation()
{
    // 32 MB–64 MB   => 512 bytes
    // 64 MB–128 MB  => 1 KB
    // 128 MB–256 MB => 2KB
    // 256 MB–8GB    => 4KB
    // 8GB–16GB      => 8KB

    using namespace file_system::fat::details;
    assert(512 == calculate_cluster_size<34 << 20>());
    assert(1 << 10 == calculate_cluster_size<68 << 20>());
    assert(2 << 10 == calculate_cluster_size<130 << 20>());
    assert(4 << 10 == calculate_cluster_size<258 << 20>());
    assert(8 << 10 == calculate_cluster_size<static_cast<uint64_t>(9) << 30>());
}

inline void test_fat_calculate_file_allocation_table_size()
{
    using namespace file_system::fat::details;
    static constexpr uint16_t BPB_SIZE = 512;
    static constexpr uint16_t CLUSTER_SIZE = 8192;
    static constexpr uint64_t FS_SIZE = 32768;

    const auto result = calculate_file_allocation_table_size<BPB_SIZE, CLUSTER_SIZE, FS_SIZE>();
    assert(3 * sizeof(Cluster) == result);
}

inline void test_fat_memory_manager()
{
    using namespace file_system::fat;
    MemoryManager<32768> memory_manager;
    details::Cluster* first_cluster = memory_manager.allocate_cluster();
    assert(first_cluster != nullptr);
    details::Cluster* second_cluster = memory_manager.allocate_cluster();
    assert(second_cluster != nullptr);
    details::Cluster* third_cluster = memory_manager.allocate_cluster();
    assert(third_cluster != nullptr);

    memory_manager.deallocate_cluster(first_cluster);
    details::Cluster* same_first_cluster = memory_manager.allocate_cluster();
    assert(first_cluster == same_first_cluster);
}

inline void test_file_system()
{
    using namespace file_system::fat;
    FileSystem<32768> file_system;

    file_system::File* file = file_system.get_file_impl("test_1.txt");
    assert(file == nullptr);

    file_system::File* first_file = file_system.create_file_impl("test_1.txt");
    file_system::File* second_file = file_system.create_file_impl("test_2.txt");
    assert(first_file != nullptr);
    assert(second_file != nullptr);

    assert(file_system.get_file_impl("test_1.txt") != nullptr);
    assert(file_system.get_file_impl("test_2.txt") != nullptr);
    assert(file_system.get_file_impl("test_3.txt") == nullptr);

    file_system.remove_file_impl("test_1.txt");
    assert(file_system.get_file_impl("test_1.txt") == nullptr);
    assert(file_system.get_file_impl("test_2.txt") != nullptr);

    bool result = file_system.write_file_impl(second_file, {'H','e','l','l','o',' ','w','o','r','l','d'});
    assert(result);
}