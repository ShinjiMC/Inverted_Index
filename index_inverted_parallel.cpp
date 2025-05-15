#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <omp.h>

constexpr size_t BLOCK_SIZE = 1 << 22; // 4MB

std::string normalize(const std::string &word)
{
    std::string result;
    result.reserve(word.size());
    for (char c : word)
        if (std::isalnum(static_cast<unsigned char>(c)))
            result += std::tolower(c);
    return result;
}

void process_chunk(std::ifstream &file, size_t start, size_t end,
                   std::unordered_map<std::string, std::unordered_set<std::string>> &local_map,
                   const std::string &filename)
{
    size_t chunk_size = end - start;
    std::vector<char> buffer(chunk_size);
    file.seekg(start);
    file.read(buffer.data(), chunk_size);
    size_t bytes_read = file.gcount();

    std::string word;
    for (size_t i = 0; i < bytes_read; ++i)
    {
        char c = buffer[i];
        if (std::isalnum(static_cast<unsigned char>(c)))
            word += std::tolower(c);
        else if (!word.empty())
        {
            local_map[word].insert(filename);
            word.clear();
        }
    }
    if (!word.empty())
        local_map[word].insert(filename);
}

void process_file_blocks(const std::string &filename,
                         std::unordered_map<std::string, std::unordered_set<std::string>> &global_index)
{
    std::ifstream test_file(filename, std::ios::binary | std::ios::ate);
    if (!test_file)
    {
        std::cerr << "No se pudo abrir: " << filename << "\n";
        return;
    }
    size_t file_size = test_file.tellg();
    test_file.close();
    size_t num_blocks = (file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    std::vector<std::unordered_map<std::string, std::unordered_set<std::string>>> thread_maps(num_blocks);

#pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < num_blocks; ++i)
    {
        size_t start = i * BLOCK_SIZE;
        size_t end = std::min(start + BLOCK_SIZE, file_size);
        std::ifstream file(filename, std::ios::binary);
        if (!file)
            continue;
        if (i != 0)
        {
            file.seekg(start);
            char c;
            while (start < end && file.get(c) && std::isalnum(static_cast<unsigned char>(c)))
                ++start;
        }
        if (end < file_size && i != num_blocks - 1)
        {
            file.seekg(end);
            char c;
            while (end < file_size && file.get(c) && std::isalnum(static_cast<unsigned char>(c)))
                ++end;
        }
        process_chunk(file, start, end, thread_maps[i], filename);
    }

#pragma omp critical
    for (const auto &local : thread_maps)
    {
        for (const auto &[word, files] : local)
            global_index[word].insert(files.begin(), files.end());
    }
}

void write_index(const std::unordered_map<std::string, std::unordered_set<std::string>> &index,
                 const std::string &output)
{
    std::vector<std::string> words;
    words.reserve(index.size());
    for (const auto &[word, _] : index)
        words.push_back(word);
    std::sort(words.begin(), words.end());
    std::ofstream out(output);
    if (!out)
    {
        std::cerr << "No se pudo crear archivo de salida\n";
        return;
    }
    for (const auto &word : words)
    {
        out << word << "\t";
        const auto &files = index.at(word);
        bool first = true;
        for (const auto &file : files)
        {
            if (!first)
                out << " ";
            out << file;
            first = false;
        }
        out << "\n";
    }
    out.close();
}

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        std::cerr << "Uso: " << argv[0] << " archivo1 archivo2 ...\n";
        return 1;
    }
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::string> files(argv + 1, argv + argc);
    size_t n = files.size();
    std::vector<std::unordered_map<std::string, std::unordered_set<std::string>>> partial_indexes(n);
    std::cout << "Generando índice invertido...\n";

#pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < n; ++i)
        process_file_blocks(files[i], partial_indexes[i]);

    std::unordered_map<std::string, std::unordered_set<std::string>> final_index;

#pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < n; ++i)
    {
#pragma omp critical
        for (const auto &[word, fileset] : partial_indexes[i])
            final_index[word].insert(fileset.begin(), fileset.end());
    }

    write_index(final_index, "indice_final.txt");

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Índice invertido almacenado en 'indice_final.txt'.\n";
    std::cout << "Tiempo total de ejecución:\t"
              << std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count() << " segundos.\n";
    return 0;
}
