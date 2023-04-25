#pragma once

#include <string>

#include "../file.hpp"

namespace file_system::fat {

namespace details {
struct Cluster;
} // namespace details

template<size_t FS_SIZE>
struct FileSystem;

struct File : file_system::File
{
    template<size_t FS_SIZE>
    friend struct FileSystem;

    File(File&& other) = delete;
    File(const File& other) = delete;
    File& operator = (File&& other) = delete;
    File& operator = (const File& other) = delete;

    [[nodiscard]] size_t get_size() const noexcept override;
    [[nodiscard]] size_t get_created_time() const noexcept override;
    [[nodiscard]] size_t get_last_modified_time() const noexcept override;
    [[nodiscard]] std::string_view get_name() const noexcept override;

private:
    explicit File(const std::string& file_name, details::Cluster* cluster, size_t offset, size_t size);

private:
    std::string m_name;

    size_t m_cluster_idx = 0;
    size_t m_cluster_size = 0;
    size_t m_cluster_offset = 0;
    details::Cluster* m_clusters;

    size_t m_size = 0;
    size_t m_created_time = 0;
    size_t m_last_modified_time = 0;
};

} // namespace file_system::fat