#pragma once

#include <array>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace qpp::examples::trie {

struct TrieNode {
    std::array<std::unique_ptr<TrieNode>, 26> children{};
    bool is_word{false};
};

inline TrieNode* ensure_child(TrieNode& node, char letter) {
    const auto index = static_cast<std::size_t>(letter - 'a');
    if (!node.children[index])
        node.children[index] = std::make_unique<TrieNode>();
    return node.children[index].get();
}

inline TrieNode* find_child(TrieNode& node, char letter) {
    const auto index = static_cast<std::size_t>(letter - 'a');
    return node.children[index] ? node.children[index].get() : nullptr;
}

inline const TrieNode* find_child(const TrieNode& node, char letter) {
    const auto index = static_cast<std::size_t>(letter - 'a');
    return node.children[index] ? node.children[index].get() : nullptr;
}

class Trie {
  public:
    Trie() : root_(std::make_unique<TrieNode>()) {}

    void insert(std::string_view word) {
        auto* current = root_.get();
        for (const auto letter : word) {
            current = ensure_child(*current, letter);
        }
        current->is_word = true;
    }

    [[nodiscard]] bool search(std::string_view word) const {
        const auto* node = find_node(word);
        return node && node->is_word;
    }

    [[nodiscard]] bool starts_with(std::string_view prefix) const {
        return find_node(prefix) != nullptr;
    }

  private:
    [[nodiscard]] const TrieNode* find_node(std::string_view key) const {
        auto* current = root_.get();
        for (const auto letter : key) {
            current = find_child(*current, letter);
            if (!current)
                return nullptr;
        }
        return current;
    }

    std::unique_ptr<TrieNode> root_;
};

class WordDictionary {
  public:
    WordDictionary() : root_(std::make_unique<TrieNode>()) {}

    void add_word(std::string_view word) {
        auto* current = root_.get();
        for (const auto letter : word)
            current = ensure_child(*current, letter);
        current->is_word = true;
    }

    [[nodiscard]] bool search(std::string_view word) const {
        return search_recursive(root_.get(), word, 0);
    }

  private:
    [[nodiscard]] bool search_recursive(const TrieNode* node, std::string_view word,
                                        std::size_t index) const {
        if (!node)
            return false;

        if (index == word.size())
            return node->is_word;

        const char letter = word[index];
        if (letter == '.') {
            for (const auto& child : node->children) {
                if (child && search_recursive(child.get(), word, index + 1))
                    return true;
            }
            return false;
        }

        const auto* next = find_child(*node, letter);
        return search_recursive(next, word, index + 1);
    }

    std::unique_ptr<TrieNode> root_;
};

struct WordSearchNode {
    std::array<std::unique_ptr<WordSearchNode>, 26> children{};
    std::string word{};
};

class WordSearchTrie {
  public:
    WordSearchTrie() : root_(std::make_unique<WordSearchNode>()) {}

    void insert(const std::string& word) {
        auto* current = root_.get();
        for (const auto letter : word) {
            const auto index = static_cast<std::size_t>(letter - 'a');
            if (!current->children[index])
                current->children[index] = std::make_unique<WordSearchNode>();
            current = current->children[index].get();
        }
        current->word = word;
    }

    [[nodiscard]] WordSearchNode* root() const { return root_.get(); }

  private:
    std::unique_ptr<WordSearchNode> root_;
};

inline void dfs(std::vector<std::string>& board, int row, int col, WordSearchNode* node,
                std::vector<std::string>& result) {
    const char letter = board[row][col];
    const auto index = static_cast<std::size_t>(letter - 'a');
    auto* next = node->children[index].get();
    if (!next)
        return;

    if (!next->word.empty()) {
        result.push_back(next->word);
        next->word.clear();
    }

    board[row][col] = '#';

    constexpr std::array<std::pair<int, int>, 4> kDirections{std::pair{-1, 0}, {1, 0},
                                                             {0, -1}, {0, 1}};
    for (const auto [dr, dc] : kDirections) {
        const auto next_row = row + dr;
        const auto next_col = col + dc;
        if (next_row < 0 || next_row >= static_cast<int>(board.size()) || next_col < 0 ||
            next_col >= static_cast<int>(board.front().size()) || board[next_row][next_col] == '#')
            continue;
        dfs(board, next_row, next_col, next, result);
    }

    board[row][col] = letter;
}

inline std::vector<std::string> word_search_ii(std::vector<std::string> board,
                                               const std::vector<std::string>& words) {
    if (board.empty() || board.front().empty() || words.empty())
        return {};

    WordSearchTrie trie;
    for (const auto& word : words)
        trie.insert(word);

    std::vector<std::string> result;
    for (int row = 0; row < static_cast<int>(board.size()); ++row) {
        for (int col = 0; col < static_cast<int>(board.front().size()); ++col) {
            dfs(board, row, col, trie.root(), result);
        }
    }

    return result;
}

} // namespace qpp::examples::trie

