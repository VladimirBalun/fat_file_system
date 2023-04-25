#pragma once

#include <array>
#include <string>

namespace file_system::fat {

namespace details {

struct Cluster
{
    void* memory;
    Cluster* next;
};

template<uint64_t FS_SIZE>
constexpr uint64_t calculate_cluster_size()
{
    // 32 MB–64 MB   => 512 bytes
    // 64 MB–128 MB  => 1 KB
    // 128 MB–256 MB => 2KB
    // 256 MB–8GB    => 4KB
    // 8GB–16GB      => 8KB

    if constexpr(FS_SIZE < 64 << 20)
        return 512;
    else if(FS_SIZE < 128 << 20)
        return 1 << 10;
    else if(FS_SIZE < 256 << 20)
        return 2 << 10;
    else if(FS_SIZE < static_cast<uint64_t>(8) << 30)
        return 4 << 10;
    else
        return 8 << 10;
}

template<uint16_t BPB_SIZE, uint16_t CLUSTER_SIZE, uint64_t FS_SIZE>
constexpr uint64_t calculate_file_allocation_table_size()
{
    const uint64_t diff = FS_SIZE - BPB_SIZE;
    const uint64_t clusters_number = diff / CLUSTER_SIZE;
    return clusters_number * sizeof(Cluster);
}

} // namespace details

template<uint64_t FS_SIZE>
struct MemoryManager
{
    // FS  => File System
    // BPB => Bios Parameters Block
    // FAT => File Allocation Table

    static_assert(FS_SIZE >= 32 >> 20, "Incorrect file system size");

    static constexpr uint16_t BPB_SIZE = 512;
    static constexpr uint16_t CLUSTER_SIZE = details::calculate_cluster_size<FS_SIZE>();
    static constexpr uint64_t FAT_MEMORY_SIZE =
            details::calculate_file_allocation_table_size<BPB_SIZE, CLUSTER_SIZE, FS_SIZE>();
    static constexpr uint64_t CLUSTERS_MEMORY_SIZE = FS_SIZE - BPB_SIZE - FAT_MEMORY_SIZE;

    MemoryManager() noexcept
    {
        size_t cluster_idx = 0;
        const int64_t available_clusters = CLUSTERS_MEMORY_SIZE / CLUSTER_SIZE;
        while (cluster_idx < available_clusters) {
            void* cluster_memory = m_clusters_memory.data() + (cluster_idx * CLUSTER_SIZE);
            void* cluster_descriptor_memory = m_file_allocation_table_memory.data() +
                (cluster_idx * sizeof(details::Cluster));

            auto cluster = new (cluster_descriptor_memory) details::Cluster();
            cluster->memory = cluster_memory;
            cluster->next = m_available_clusters;
            m_available_clusters = cluster;
            ++cluster_idx;
        }
    }

    details::Cluster* allocate_cluster() noexcept
    {
        if (!m_available_clusters)
            return nullptr;

        details::Cluster* cluster = m_available_clusters;
        m_available_clusters = m_available_clusters->next;
        cluster->next = nullptr;
        return cluster;
    }

    void deallocate_cluster(details::Cluster* cluster) noexcept
    {
        if (!cluster)
            return;

        cluster->next = m_available_clusters;
        m_available_clusters = cluster;
    }

private:
    std::array<char, BPB_SIZE> m_bpb_memory = {0};
    std::array<char, CLUSTERS_MEMORY_SIZE> m_clusters_memory = {0};
    std::array<char, FAT_MEMORY_SIZE> m_file_allocation_table_memory = {0};
    details::Cluster* m_available_clusters = nullptr;
};

} // namespace file_system::fat