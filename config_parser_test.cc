#include "gtest/gtest.h"
#include "config_parser.h"
#include <sstream>

TEST(NginxConfigParserTest, SimpleConfig) {
  NginxConfigParser parser;
  NginxConfig out_config;

  bool success = parser.Parse("example_config", &out_config);

  EXPECT_TRUE(success);
}

TEST(NginxConfigParserTest, StatementParsed) {
  NginxConfigParser parser;
  NginxConfig config;

  std::stringstream test_config_stream("port 8080;");

  bool success = parser.Parse(&test_config_stream, &config);

  EXPECT_TRUE(success);

  // one statement
  ASSERT_EQ(config.statements_.size(), 1);
  // two tokens
  ASSERT_EQ(config.statements_[0]->tokens_.size(), 2);

  EXPECT_EQ(config.statements_[0]->tokens_[0], "port");
  EXPECT_EQ(config.statements_[0]->tokens_[1], "8080");
}

TEST(NginxConfigParserTest, InsignificantWhitespace) {
  NginxConfigParser parser;
  NginxConfig config;

  std::stringstream test_config_stream(" \t   foo  \t   bar     ;    \t");

  bool success = parser.Parse(&test_config_stream, &config);

  EXPECT_TRUE(success);

  // one statement
  ASSERT_EQ(config.statements_.size(), 1);
  // two tokens
  ASSERT_EQ(config.statements_[0]->tokens_.size(), 2);

  EXPECT_EQ(config.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config.statements_[0]->tokens_[1], "bar");
}
