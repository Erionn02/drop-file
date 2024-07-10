#include <gtest/gtest.h>

#include "SessionsManager.hpp"

using namespace std::chrono_literals;

TEST(SessionsManagerTests, throwsOnNonExistentCodeWords) {
    SessionsManager manager;
    ASSERT_EQ(manager.currentSessions(), 0);
    ASSERT_THROW(manager.getSenderWithMetadata("non-existent-code-words"), SessionsManagerException);
}

TEST(SessionsManagerTests, addingConnectionsIncreasesReturnedAmount) {
    SessionsManager manager;
    ASSERT_EQ(manager.currentSessions(), 0);
    ASSERT_NO_FATAL_FAILURE(manager.registerSender(nullptr, {})); // also does not use any of actual values
    ASSERT_EQ(manager.currentSessions(), 1); // also does not use any of actual values
}

TEST(SessionsManagerTests, canRetrieveSessionWithCorrectCodeWords) {
    SessionsManager manager;
    std::string code_words = manager.registerSender(nullptr, {});
    ASSERT_EQ(manager.currentSessions(), 1);
    ASSERT_NO_THROW(manager.getSenderWithMetadata(code_words));
    ASSERT_EQ(manager.currentSessions(), 0);
}

TEST(SessionsManagerTests, removesClientsAfterTimeoutPeriod) {
    std::chrono::seconds timeout{3s};
    std::chrono::seconds check_period{1s};
    SessionsManager manager{timeout, check_period};

    manager.registerSender(nullptr, {});
    ASSERT_EQ(manager.currentSessions(), 1);
    std::this_thread::sleep_for(6s);
    ASSERT_EQ(manager.currentSessions(), 0);
}