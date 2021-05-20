// Copyright (c) Facebook, Inc. and its affiliates.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "rsocket/framing/Framer.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace rsocket;
using namespace testing;

class FramerMock : public Framer {
 public:
  explicit FramerMock(ProtocolVersion protocolVersion = ProtocolVersion::Latest)
      : Framer(protocolVersion, true) {}

  MOCK_METHOD1(error, void(const char*));
  MOCK_METHOD1(onFrame_, void(std::unique_ptr<folly::IOBuf>&));

  void onFrame(std::unique_ptr<folly::IOBuf> frame) override {
    onFrame_(frame);
  }
};

MATCHER_P(isIOBuffEq, n, "") {
  return folly::IOBufEqualTo()(arg, n);
}

TEST(Framer, TinyFrame) {
  FramerMock framer;

  // Not using hex string-literal as std::string ctor hits '\x00' and stops
  // reading.
  auto buf = folly::IOBuf::createCombined(4);
  buf->append(4);
  buf->writableData()[0] = '\x00';
  buf->writableData()[1] = '\x00';
  buf->writableData()[2] = '\x00';
  buf->writableData()[3] = '\x02';

  EXPECT_CALL(framer, error(_));
  framer.addFrameChunk(std::move(buf));
}

TEST(Framer, CantDetectVersion) {
  FramerMock framer(ProtocolVersion::Unknown);

  EXPECT_CALL(framer, error(_));

  auto buf = folly::IOBuf::copyBuffer("ABCDEFGHIJKLMNOP");
  framer.addFrameChunk(std::move(buf));
}

TEST(Framer, ParseFrame) {
  FramerMock framer;

  auto buf = folly::IOBuf::copyBuffer("ABCDEFGHIJKLMNOP");
  EXPECT_CALL(framer, onFrame_(Pointee(isIOBuffEq(*buf))));

  framer.addFrameChunk(framer.prependSize(std::move(buf)));
}
