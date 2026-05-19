#pragma once
#include <functional>
#include <vector>

namespace UHE::RHI {


class DeletionQueue {
public:
    void Push(std::function<void()>&& fn) {
        m_deletors.push_back(std::move(fn));
    }

    /// Flush all pending deletions in LIFO order (reverse of creation).
    void Flush() {
        for (auto it = m_deletors.rbegin(); it != m_deletors.rend(); ++it) {
            (*it)();
        }
        m_deletors.clear();
    }

    [[nodiscard]] bool Empty() const { return m_deletors.empty(); }
    [[nodiscard]] size_t Size() const { return m_deletors.size(); }

private:
    std::vector<std::function<void()>> m_deletors;
};

} // namespace UHE::RHI
