#include "gtest/gtest.h"
#include "config_parser.h"
#include <sstream>

TEST(NginxConfigParserTest, SimpleConfig) {
  NginxConfigParser parser;
  NginxConfig out_config;

  bool success = parser.Parse("example_config", &out_config);

  EXPECT_TRUE(success);
}

class NginxConfigParserStringTest : public ::testing::Test {
    protected:
        virtual bool parseString(std::string s) {
            std::stringstream test_config_stream(s);

            return parser_.Parse(&test_config_stream, &config_);
        }

        NginxConfigParser parser_;
        NginxConfig config_;
};

TEST_F(NginxConfigParserStringTest, StatementParsed) {
  bool success = parseString("port 8080;");

  EXPECT_TRUE(success);

  // one statement
  ASSERT_EQ(config_.statements_.size(), 1);
  // two tokens
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 2);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "port");
  EXPECT_EQ(config_.statements_[0]->tokens_[1], "8080");
}

TEST_F(NginxConfigParserStringTest, InsignificantWhitespace) {
  bool success = parseString(" \t   foo  \t   bar     ;    \t");

  EXPECT_TRUE(success);

  // one statement
  ASSERT_EQ(config_.statements_.size(), 1);
  // two tokens
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 2);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config_.statements_[0]->tokens_[1], "bar");
}

TEST_F(NginxConfigParserStringTest, NoSemicolon) {
  bool success = parseString("foo bar");

  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserStringTest, UnexpectedNewline) {
  bool success = parseString("\nfoo bar;");

  EXPECT_TRUE(success);

  // one statement
  ASSERT_EQ(config_.statements_.size(), 1);
  // two tokens
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 2);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config_.statements_[0]->tokens_[1], "bar");
}

TEST_F(NginxConfigParserStringTest, OneLineTwoStatements) {
  bool success = parseString("foo bar; baz bop;");

  EXPECT_TRUE(success);

  // two statements
  ASSERT_EQ(config_.statements_.size(), 2);
  // two tokens each
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 2);
  ASSERT_EQ(config_.statements_[1]->tokens_.size(), 2);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config_.statements_[0]->tokens_[1], "bar");

  EXPECT_EQ(config_.statements_[1]->tokens_[0], "baz");
  EXPECT_EQ(config_.statements_[1]->tokens_[1], "bop");
}

TEST_F(NginxConfigParserStringTest, ChildStatement) {
  bool success = parseString("foo {\n\tbar baz;\n}");

  EXPECT_TRUE(success);

  // one statement
  ASSERT_EQ(config_.statements_.size(), 1);
  // one token
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 1);
  // child block has one statement
  ASSERT_EQ(config_.statements_[0]->child_block_->statements_.size(), 1);
  // child block statement has two tokens
  ASSERT_EQ(config_.statements_[0]->child_block_->statements_[0]->tokens_.size(), 2);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config_.statements_[0]->child_block_->statements_[0]->tokens_[0], "bar");
  EXPECT_EQ(config_.statements_[0]->child_block_->statements_[0]->tokens_[1], "baz");
}

TEST_F(NginxConfigParserStringTest, NestedChildStatement) {
  bool success = parseString(
          "foo {\
            bar {\
              hello;\
            }\
            baz;\
          }");

  EXPECT_TRUE(success);

  NginxConfigStatement* fooStatement = &*config_.statements_[0];

  NginxConfigStatement* barStatement = &*fooStatement->child_block_->statements_[0];
  NginxConfigStatement* bazStatement = &*fooStatement->child_block_->statements_[1];

  NginxConfigStatement* helloStatement = &*barStatement->child_block_->statements_[0];


  EXPECT_EQ(fooStatement->tokens_[0], "foo");
  EXPECT_EQ(barStatement->tokens_[0], "bar");
  EXPECT_EQ(bazStatement->tokens_[0], "baz");
  EXPECT_EQ(helloStatement->tokens_[0], "hello");
}

TEST_F(NginxConfigParserStringTest, InvalidChildStatement_NoOpenBrace) {
  bool success = parseString("foo } bar;");

  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserStringTest, InvalidChildStatement_NoCloseBrace) {
  bool success = parseString("foo { bar;");

  EXPECT_FALSE(success);
}
