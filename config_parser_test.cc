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
  EXPECT_TRUE(parseString("port 8080;"));

  // one statement
  ASSERT_EQ(config_.statements_.size(), 1);
  // two tokens
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 2);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "port");
  EXPECT_EQ(config_.statements_[0]->tokens_[1], "8080");
}

TEST_F(NginxConfigParserStringTest, InsignificantWhitespace) {
  EXPECT_TRUE(parseString(" \t   foo  \t   bar     ;    \t"));

  // one statement
  ASSERT_EQ(config_.statements_.size(), 1);
  // two tokens
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 2);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config_.statements_[0]->tokens_[1], "bar");
}

TEST_F(NginxConfigParserStringTest, NoSemicolon) {
  EXPECT_FALSE(parseString("foo bar"));
}

TEST_F(NginxConfigParserStringTest, UnexpectedNewline) {
  EXPECT_TRUE(parseString("\nfoo bar;"));

  // one statement
  ASSERT_EQ(config_.statements_.size(), 1);
  // two tokens
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 2);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config_.statements_[0]->tokens_[1], "bar");
}

TEST_F(NginxConfigParserStringTest, OneLineTwoStatements) {
  EXPECT_TRUE(parseString("foo bar; baz bop;"));

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
  EXPECT_TRUE(parseString("foo {\n\tbar baz;\n}"));

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
  EXPECT_TRUE(parseString(
          "foo {\
            bar {\
              hello;\
            }\
            baz;\
          }"));

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
  EXPECT_FALSE(parseString("foo } bar;"));
}

TEST_F(NginxConfigParserStringTest, InvalidChildStatement_NoCloseBrace) {
  EXPECT_FALSE(parseString("foo { bar;"));
}

TEST_F(NginxConfigParserStringTest, ExtraNewline) {
  EXPECT_TRUE(parseString("foo;\n\n\nbar;"));

  // two statements
  ASSERT_EQ(config_.statements_.size(), 2);
  // one tokens each
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 1);
  ASSERT_EQ(config_.statements_[1]->tokens_.size(), 1);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config_.statements_[1]->tokens_[0], "bar");

}

TEST_F(NginxConfigParserStringTest, Comments) {
  EXPECT_TRUE(parseString("foo; # this is a comment ;; } {\nbar;"));

  // two statements
  ASSERT_EQ(config_.statements_.size(), 2);
  // one tokens each
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 1);
  ASSERT_EQ(config_.statements_[1]->tokens_.size(), 1);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config_.statements_[1]->tokens_[0], "bar");

}

TEST_F(NginxConfigParserStringTest, DoubleQuotes) {
  EXPECT_TRUE(parseString("foo = \"Hello, World!\";"));

  // one statement
  ASSERT_EQ(config_.statements_.size(), 1);
  // three tokens
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 3);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config_.statements_[0]->tokens_[1], "=");
  EXPECT_EQ(config_.statements_[0]->tokens_[2], "\"Hello, World!\"");

}

TEST_F(NginxConfigParserStringTest, DoubleQuotesSpecialChars) {
  EXPECT_TRUE(parseString("foo = \"test \n test; ' } {\";"));

  // one statement
  ASSERT_EQ(config_.statements_.size(), 1);
  // three tokens
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 3);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config_.statements_[0]->tokens_[1], "=");
  EXPECT_EQ(config_.statements_[0]->tokens_[2], "\"test \n test; ' } {\"");

}

TEST_F(NginxConfigParserStringTest, InvalidDoubleQuotes) {
  EXPECT_FALSE(parseString("foo = test\"hello;"));
}
