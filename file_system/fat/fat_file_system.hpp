#pragma once

#include <vector>

#include "fat_file.hpp"
#include "fat_memory_manager.hpp"
#include "../file_system.hpp"

namespace file_system::fat {

template<size_t FS_SIZE>
struct FileSystem : file_system::FileSystem<FileSystem<FS_SIZE>>
{
    file_system::File* get_file_impl(std::string_view file_name)
    {
        for (size_t i = 0; i < m_files.max_size(); ++i) {
            File* file = m_files[i];
            if (file && file->m_name == file_name)
                return file;
        }

        return nullptr;
    }

    file_system::File* create_file_impl(std::string_view file_name)
    {
        constexpr size_t STRING_MAX_SSO_SIZE = 16;
        if (file_name.size() > STRING_MAX_SSO_SIZE)
            return nullptr;

        if (get_file_impl(file_name))
            return nullptr;

        details::Cluster* cluster = m_memory_manager.allocate_cluster();
        if (!cluster)
            return nullptr;

        const size_t cluster_offset = sizeof(File);
        const size_t cluster_size = MemoryManager<FS_SIZE>::CLUSTER_SIZE;
        auto file = new (cluster->memory) File(std::string(file_name), cluster, cluster_offset, cluster_size);

        for (size_t i = 0; i < m_files.max_size(); ++i) {
            if (!m_files[i]) {
                m_files[i] = file;
                return file;
            }
        }

        m_memory_manager.deallocate_cluster(cluster);
        return nullptr;
    }

    void remove_file_impl(std::string_view file_name)
    {
        auto file = dynamic_cast<File*>(get_file_impl(file_name));
        if (!file)
            return;

        details::Cluster* current_cluster = file->m_clusters;
        while (current_cluster) {
            details::Cluster* next_cluster = current_cluster->next;
            m_memory_manager.deallocate_cluster(current_cluster);
            current_cluster = next_cluster;
        }

        for (size_t i = 0; i < m_files.max_size(); ++i) {
            if (m_files[i] && m_files[i]->m_name == file_name) {
                m_files[i] = nullptr;
                return;
            }
        }
    }

    bool read_file_impl(file_system::File* f, std::vector<char>& data)
    {
        auto file = dynamic_cast<File*>(f);
        if (!file)
            return false;

        details::Cluster* cluster = file->m_clusters;
        const size_t initial_cluster_offset = sizeof(File);
        for (size_t i = 0; i < file->m_cluster_idx; ++i) {
            const int offset = (i == 0) ?
                    initial_cluster_offset : 0;

            std::copy(
                static_cast<char*>(cluster->memory) + offset,
                static_cast<char*>(cluster->memory) + file->m_cluster_size,
                std::back_inserter(data)
            );

            cluster = cluster->next;
        }

        const int offset = (file->m_cluster_idx == 0) ?
            initial_cluster_offset : 0;

        std::copy(
            static_cast<char*>(cluster->memory) + offset,
            static_cast<char*>(cluster->memory) + file->m_cluster_offset,
            std::back_inserter(data)
        );
    }

    bool write_file_impl(file_system::File* f, const std::vector<char>& data)
    {
        auto file = dynamic_cast<File*>(f);
        if (!file)
            return false;

        const auto timestamp = std::time(nullptr);
        file->m_last_modified_time = timestamp;

        size_t written_bytes = 0;
        details::Cluster* cluster = file->m_clusters;
        file->m_cluster_offset = sizeof(File);

        while (written_bytes < data.size()) {
            const size_t available_bytes = file->m_cluster_size - file->m_cluster_offset;
            char* cluster_memory = static_cast<char*>(cluster->memory) + file->m_cluster_offset;
            for (size_t i = 0; i < std::min(available_bytes, data.size()); ++i)
                cluster_memory[i] = data[written_bytes++];

            if (written_bytes < data.size()) {
                file->m_cluster_offset = 0;
                details::Cluster* new_cluster = m_memory_manager.allocate_cluster();
                if (!new_cluster)
                    return false;

                cluster->next = new_cluster;
                cluster = new_cluster;
                ++file->m_cluster_idx;
            } else {
                file->m_cluster_offset += written_bytes;
            }
        }

        return true;
    }

private:
    static constexpr size_t ROOT_DIRECTORY_SIZE = 1024;
    MemoryManager<FS_SIZE - ROOT_DIRECTORY_SIZE> m_memory_manager;
    std::array<File*, ROOT_DIRECTORY_SIZE / sizeof(uintptr_t)> m_files = { nullptr };
};

} // namespace file_system::fat
