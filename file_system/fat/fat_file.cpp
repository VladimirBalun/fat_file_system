#include "fat_file.hpp"

file_system::fat::File::File(const std::string& file_name, details::Cluster* cluster, size_t offset, size_t size)
    : m_name(file_name), m_clusters(cluster), m_cluster_size(size), m_cluster_offset(offset)
{
    const auto timestamp = std::time(nullptr);
    m_created_time = timestamp;
    m_last_modified_time = timestamp;
}

size_t file_system::fat::File::get_size() const noexcept
{
    return m_size;
}

size_t file_system::fat::File::get_created_time() const noexcept
{
    return m_created_time;
}

size_t file_system::fat::File::get_last_modified_time() const noexcept
{
    return m_last_modified_time;
}

std::string_view file_system::fat::File::get_name() const noexcept
{
    return m_name;
}