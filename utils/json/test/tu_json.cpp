//
// Test unit using jsmn JSON parser
//

#include "c3qo/tu.hpp"

// Logger library should be linked
extern enum logger_level logger_level;

class tu_json : public testing::Test
{
  public:
    void SetUp()
    {
        LOGGER_OPEN("tu_json");
        logger_set_level(LOGGER_LEVEL_DEBUG);
    }
    void TearDown()
    {
        logger_set_level(LOGGER_LEVEL_NONE);
        LOGGER_CLOSE();
    }
};

//
// @brief Parse a JSON string
//
TEST_F(tu_json, parse)
{
    std::vector<struct json_entry> token;
    std::string json;

    json = "{ \"begin\": \"hello world\", \"list\": [1, null, true], \"end\": \"goodbye world\" }";
    EXPECT_TRUE(json_parse(json.c_str(), token));

    json = "{ \"begin\": \"hello world\", \"object\": {\"a\": 1, \"b\": null, \"c\": true}, \"end\": \"goodbye world\" }";
    EXPECT_TRUE(json_parse(json.c_str(), token));
}

//
// @brief Call the helper method
//
TEST_F(tu_json, helper)
{
    for (int i = -9; i < 10; ++i)
    {
        EXPECT_NE(json_type_to_string(static_cast<enum json_type>(i)), nullptr);
    }
}

//
// @brief Test some error cases
//
TEST_F(tu_json, error)
{
    std::vector<struct json_entry> token;
    std::string json;

    EXPECT_FALSE(json_parse(nullptr, token));

    json = "";
    EXPECT_FALSE(json_parse(json.c_str(), token));

    json = "{[}"
           "]";
    EXPECT_FALSE(json_parse(json.c_str(), token));
}
