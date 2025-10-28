#pragma once

#include <array>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "qpp/pbool.h"
#include "qpp/qvector"

namespace qpp::examples::trie {

template <typename Node>
using QuantumChildPtr = std::unique_ptr<Node>;

template <typename Node, std::size_t N>
using QuantumChildArray = std::array<QuantumChildPtr<Node>, N>;

struct QuantumTrieNode {
    QuantumChildArray<QuantumTrieNode, 26> children{};
    qpp::pbool word_bias{0.0};

    void mark_word(qpp::pbool bias = qpp::pbool{1.0}) { word_bias = std::move(bias); }
};

inline QuantumTrieNode* ensure_child(QuantumTrieNode& node, char letter) {
    const auto index = static_cast<std::size_t>(letter - 'a');
    if (!node.children[index])
        node.children[index] = std::make_unique<QuantumTrieNode>();
    return node.children[index].get();
}

inline QuantumTrieNode* find_child(QuantumTrieNode& node, char letter) {
    const auto index = static_cast<std::size_t>(letter - 'a');
    return node.children[index] ? node.children[index].get() : nullptr;
}

inline const QuantumTrieNode* find_child(const QuantumTrieNode& node, char letter) {
    const auto index = static_cast<std::size_t>(letter - 'a');
    return node.children[index] ? node.children[index].get() : nullptr;
}

class Trie {
  public:
    Trie() : root_(std::make_unique<QuantumTrieNode>()) {}

    void insert(std::string_view word) {
        auto* current = root_.get();
        for (const auto letter : word) {
            current = ensure_child(*current, letter);
        }
        current->mark_word();
    }

    [[nodiscard]] qpp::pbool search(std::string_view word) const {
        const auto* node = find_node(word);
        if (!node)
            return qpp::pbool{0.0};
        return node->word_bias;
    }

    [[nodiscard]] qpp::pbool starts_with(std::string_view prefix) const {
        return find_node(prefix) ? qpp::pbool{1.0} : qpp::pbool{0.0};
    }

  private:
    [[nodiscard]] const QuantumTrieNode* find_node(std::string_view key) const {
        auto* current = root_.get();
        for (const auto letter : key) {
            current = find_child(*current, letter);
            if (!current)
                return nullptr;
        }
        return current;
    }

    std::unique_ptr<QuantumTrieNode> root_;
};

class WordDictionary {
  public:
    WordDictionary() : root_(std::make_unique<QuantumTrieNode>()) {}

    void add_word(std::string_view word) {
        auto* current = root_.get();
        for (const auto letter : word)
            current = ensure_child(*current, letter);
        current->mark_word();
    }

    [[nodiscard]] qpp::pbool search(std::string_view word) const {
        return search_recursive(root_.get(), word, 0);
    }

  private:
    [[nodiscard]] qpp::pbool search_recursive(const QuantumTrieNode* node,
                                              std::string_view word,
                                              std::size_t index) const {
        if (!node)
            return qpp::pbool{0.0};

        if (index == word.size())
            return node->word_bias;

        const char letter = word[index];
        if (letter == '.') {
            qpp::pbool result{0.0};
            for (const auto& child : node->children) {
                if (child)
                    result = result || search_recursive(child.get(), word, index + 1);
            }
            return result;
        }

        const auto* next = find_child(*node, letter);
        return search_recursive(next, word, index + 1);
    }

    std::unique_ptr<QuantumTrieNode> root_;
};

struct QuantumWordSearchNode {
    QuantumChildArray<QuantumWordSearchNode, 26> children{};
    std::string word{};
    qpp::pbool word_bias{0.0};

    void set_word(std::string value) {
        word = std::move(value);
        word_bias = qpp::pbool{1.0};
    }

    void clear_word() {
        word.clear();
        word_bias = qpp::pbool{0.0};
    }
};

class WordSearchTrie {
  public:
    WordSearchTrie() : root_(std::make_unique<QuantumWordSearchNode>()) {}

    void insert(const std::string& word) {
        auto* current = root_.get();
        for (const auto letter : word) {
            const auto index = static_cast<std::size_t>(letter - 'a');
            if (!current->children[index])
                current->children[index] = std::make_unique<QuantumWordSearchNode>();
            current = current->children[index].get();
        }
        current->set_word(word);
    }

    [[nodiscard]] QuantumWordSearchNode* root() const { return root_.get(); }

  private:
    std::unique_ptr<QuantumWordSearchNode> root_;
};

inline void dfs(qpp::qvector<std::string>& board, int row, int col,
                QuantumWordSearchNode* node, qpp::qvector<std::string>& result,
                qpp::pbool path_bias = qpp::pbool{1.0}) {
    const char letter = board[row][col];
    const auto index = static_cast<std::size_t>(letter - 'a');
    auto* next = node->children[index].get();
    if (!next)
        return;

    const qpp::pbool next_bias = path_bias && next->word_bias;

    if (!next->word.empty() && next_bias.probability() > 0.0) {
        result.push_back(next->word);
        next->clear_word();
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
        dfs(board, next_row, next_col, next, result, next_bias);
    }

    board[row][col] = letter;
}

inline qpp::qvector<std::string> word_search_ii(
    qpp::qvector<std::string> board, const qpp::qvector<std::string>& words) {
    if (board.empty() || board.front().empty() || words.empty())
        return {};

    WordSearchTrie trie;
    for (const auto& word : words)
        trie.insert(word);

    qpp::qvector<std::string> result;
    for (int row = 0; row < static_cast<int>(board.size()); ++row) {
        for (int col = 0; col < static_cast<int>(board.front().size()); ++col) {
            dfs(board, row, col, trie.root(), result);
        }
    }

    return result;
}

} // namespace qpp::examples::trie

