#include <gtest/gtest.h>

#include "InitSessionMessage.hpp"

using namespace ::testing;

struct DropFileServerIntegrationTests : public Test {
};

TEST_F(DropFileServerIntegrationTests, throwsOnStringThatIsNotAValidJson) {
    ASSERT_THROW(InitSessionMessage::create("not a json str"), InitSessionMessageException);
}

TEST_F(DropFileServerIntegrationTests, throwsOnJsonWithoutActionKey) {
    nlohmann::json json{};
    ASSERT_THROW(InitSessionMessage::create(json.dump()), InitSessionMessageException);
}