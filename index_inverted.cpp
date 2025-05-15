#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <chrono>

constexpr size_t BLOCK_SIZE = 4 * 1024 * 1024;

std::string normalize(const std::string &word)
{
    std::string result;
    result.reserve(word.size());
    for (char c : word)
        if (std::isalnum(static_cast<unsigned char>(c)))
            result += std::tolower(c);
    return result;
}

void processBlock(const std::string &block,
                  std::unordered_map<std::string, std::unordered_set<std::string>> &localIndex,
                  const std::string &filename)
{
    size_t start = 0, len = block.length();
    while (start < len)
    {
        while (start < len && !std::isalnum(static_cast<unsigned char>(block[start])))
            ++start;
        size_t end = start;
        while (end < len && std::isalnum(static_cast<unsigned char>(block[end])))
            ++end;
        if (end > start)
        {
            std::string word = normalize(block.substr(start, end - start));
            if (!word.empty())
                localIndex[word].insert(filename);
        }
        start = end + 1;
    }
}

void processFile(const std::string &file,
                 std::unordered_map<std::string, std::unordered_set<std::string>> &globalIndex)
{
    std::ifstream in(file, std::ios::binary);
    if (!in)
    {
        std::cerr << "No se pudo abrir " << file << "\n";
        return;
    }

    std::vector<char> buffer(BLOCK_SIZE);
    std::string leftover;
    while (in.read(buffer.data(), buffer.size()) || in.gcount() > 0)
    {
        size_t bytesRead = in.gcount();
        std::string block = leftover + std::string(buffer.data(), bytesRead);

        size_t lastSpace = block.find_last_of(" \n\r\t");
        if (lastSpace != std::string::npos && lastSpace + 1 < block.size())
        {
            leftover = block.substr(lastSpace + 1);
            block = block.substr(0, lastSpace + 1);
        }
        else
            leftover.clear();
        processBlock(block, globalIndex, file);
    }

    if (!leftover.empty())
        processBlock(leftover, globalIndex, file);

    in.close();
}

void writeIndexToFile(const std::unordered_map<std::string, std::unordered_set<std::string>> &index,
                      const std::string &outputFile)
{
    std::vector<std::string> words;
    words.reserve(index.size());
    for (const auto &entry : index)
        words.push_back(entry.first);
    std::sort(words.begin(), words.end());
    std::ofstream out(outputFile);
    if (!out)
    {
        std::cerr << "No se pudo crear el archivo final\n";
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

int main(int argc, char *argv[])
{
    using namespace std::chrono;

    if (argc < 2)
    {
        std::cerr << "Uso: " << argv[0] << " archivo1.txt archivo2.txt ...\n";
        return 1;
    }

    auto start_total = high_resolution_clock::now();

    std::vector<std::string> files(argv + 1, argv + argc);
    std::unordered_map<std::string, std::unordered_set<std::string>> invertedIndex;
    std::cout << "Procesando archivos...\n";
    for (const auto &file : files)
        processFile(file, invertedIndex);
    std::string finalFile = "indice_final.txt";
    std::filesystem::remove(finalFile);
    std::cout << "Escribiendo índice en '" << finalFile << "'...\n";
    writeIndexToFile(invertedIndex, finalFile);
    auto end_total = high_resolution_clock::now();
    std::cout << "Tiempo total de ejecución:\t"
              << duration_cast<milliseconds>(end_total - start_total).count() / 1000.0
              << " segundos\n";
    return 0;
}
