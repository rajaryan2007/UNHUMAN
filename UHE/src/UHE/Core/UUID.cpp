#include "uhepch.h"
#include "UUID.h"

#include <mutex>
#include <random>
#include <unordered_set>
namespace UHE {
static std::random_device rd;
static std::mt19937_64 s_Engine(rd());
static std::uniform_int_distribution<u64> s_Distribution;

// Tracking set for session-minted UUID collision avoidance.
// NOTE: Explicitly constructed UUIDs (e.g. from scene deserialization) bypass this set.
// TODO: Migrate to 128-bit UUID (RFC 4122) if distributed multi-user editing is required.
static std::mutex s_UUIDMutex;
static std::unordered_set<u64> s_UUIDMap;

UUID::UUID()
{
    std::scoped_lock lock(s_UUIDMutex);
    do
    {
        m_UUID = s_Distribution(s_Engine);
    } while (!s_UUIDMap.insert(m_UUID).second);
}

UUID::UUID(u64 uuid)
    : m_UUID(uuid)
{
}
} // namespace UHE