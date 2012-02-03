// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/bind.h"
#include "base/environment.h"
#include "base/basictypes.h"
#include "base/logging.h"
#include "base/synchronization/waitable_event.h"
#include "media/audio/audio_output_controller.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Exactly;
using ::testing::InvokeWithoutArgs;
using ::testing::NotNull;
using ::testing::Return;

static const int kSampleRate = AudioParameters::kAudioCDSampleRate;
static const int kBitsPerSample = 16;
static const ChannelLayout kChannelLayout = CHANNEL_LAYOUT_STEREO;
static const int kSamplesPerPacket = kSampleRate / 10;
static const int kHardwareBufferSize = kSamplesPerPacket *
    ChannelLayoutToChannelCount(kChannelLayout) * kBitsPerSample / 8;
static const int kBufferCapacity = 3 * kHardwareBufferSize;

namespace media {

class MockAudioOutputControllerEventHandler
    : public AudioOutputController::EventHandler {
 public:
  MockAudioOutputControllerEventHandler() {}

  MOCK_METHOD1(OnCreated, void(AudioOutputController* controller));
  MOCK_METHOD1(OnPlaying, void(AudioOutputController* controller));
  MOCK_METHOD1(OnPaused, void(AudioOutputController* controller));
  MOCK_METHOD2(OnError, void(AudioOutputController* controller,
                             int error_code));
  MOCK_METHOD2(OnMoreData, void(AudioOutputController* controller,
                                AudioBuffersState buffers_state));

 private:
  DISALLOW_COPY_AND_ASSIGN(MockAudioOutputControllerEventHandler);
};

class MockAudioOutputControllerSyncReader
    : public AudioOutputController::SyncReader {
 public:
  MockAudioOutputControllerSyncReader() {}

  MOCK_METHOD1(UpdatePendingBytes, void(uint32 bytes));
  MOCK_METHOD2(Read, uint32(void* data, uint32 size));
  MOCK_METHOD0(Close, void());
  MOCK_METHOD0(DataReady, bool());

 private:
  DISALLOW_COPY_AND_ASSIGN(MockAudioOutputControllerSyncReader);
};

ACTION_P(SignalEvent, event) {
  event->Signal();
}

// Closes AudioOutputController synchronously.
static void CloseAudioController(AudioOutputController* controller) {
  base::WaitableEvent closed_event(true, false);
  controller->Close(base::Bind(&base::WaitableEvent::Signal,
                               base::Unretained(&closed_event)));
  closed_event.Wait();
}

TEST(AudioOutputControllerTest, CreateAndClose) {
  scoped_refptr<AudioManager> audio_manager(AudioManager::Create());
  if (!audio_manager->HasAudioOutputDevices())
    return;

  MockAudioOutputControllerEventHandler event_handler;

  EXPECT_CALL(event_handler, OnCreated(NotNull()))
      .Times(1);
  EXPECT_CALL(event_handler, OnMoreData(NotNull(), _));

  AudioParameters params(AudioParameters::AUDIO_PCM_LINEAR, kChannelLayout,
                         kSampleRate, kBitsPerSample, kSamplesPerPacket);
  scoped_refptr<AudioOutputController> controller =
      AudioOutputController::Create(audio_manager, &event_handler, params,
                                    kBufferCapacity);
  ASSERT_TRUE(controller.get());

  // Close the controller immediately.
  CloseAudioController(controller);
}

TEST(AudioOutputControllerTest, PlayAndClose) {
  scoped_refptr<AudioManager> audio_manager(AudioManager::Create());
  if (!audio_manager->HasAudioOutputDevices())
    return;

  MockAudioOutputControllerEventHandler event_handler;
  base::WaitableEvent event(false, false);

  // If OnCreated is called then signal the event.
  EXPECT_CALL(event_handler, OnCreated(NotNull()))
      .WillOnce(SignalEvent(&event));

  // OnPlaying() will be called only once.
  EXPECT_CALL(event_handler, OnPlaying(NotNull()))
      .Times(Exactly(1));

  // If OnMoreData is called enough then signal the event.
  EXPECT_CALL(event_handler, OnMoreData(NotNull(), _))
      .Times(AtLeast(10))
      .WillRepeatedly(SignalEvent(&event));

  AudioParameters params(AudioParameters::AUDIO_PCM_LINEAR, kChannelLayout,
                         kSampleRate, kBitsPerSample, kSamplesPerPacket);
  scoped_refptr<AudioOutputController> controller =
      AudioOutputController::Create(audio_manager, &event_handler, params,
                                    kBufferCapacity);
  ASSERT_TRUE(controller.get());

  // Wait for OnCreated() to be called.
  event.Wait();

  controller->Play();

  // Wait until the date is requested at least 10 times.
  for (int i = 0; i < 10; i++) {
    event.Wait();
    uint8 buf[1];
    controller->EnqueueData(buf, 0);
  }

  // Now stop the controller.
  CloseAudioController(controller);
}

TEST(AudioOutputControllerTest, PlayAndCloseLowLatency) {
  scoped_refptr<AudioManager> audio_manager(AudioManager::Create());
  if (!audio_manager->HasAudioOutputDevices())
    return;

  MockAudioOutputControllerEventHandler event_handler;
  base::WaitableEvent event(false, false);

  // If OnCreated is called then signal the event.
  EXPECT_CALL(event_handler, OnCreated(NotNull()))
      .WillOnce(SignalEvent(&event));

  // OnPlaying() will be called only once.
  EXPECT_CALL(event_handler, OnPlaying(NotNull()))
      .Times(Exactly(1));

  MockAudioOutputControllerSyncReader sync_reader;
  EXPECT_CALL(sync_reader, UpdatePendingBytes(_))
      .Times(AtLeast(10));
  EXPECT_CALL(sync_reader, DataReady())
      .WillOnce(Return(false))
      .WillOnce(Return(false))
      .WillRepeatedly(Return(true));
  EXPECT_CALL(sync_reader, Read(_, kHardwareBufferSize))
      .Times(AtLeast(10))
      .WillRepeatedly(DoAll(SignalEvent(&event),
                            Return(4)));
  EXPECT_CALL(sync_reader, Close());

  AudioParameters params(AudioParameters::AUDIO_PCM_LINEAR, kChannelLayout,
                         kSampleRate, kBitsPerSample, kSamplesPerPacket);
  scoped_refptr<AudioOutputController> controller =
      AudioOutputController::CreateLowLatency(audio_manager,
                                              &event_handler,
                                              params,
                                              &sync_reader);
  ASSERT_TRUE(controller.get());

  // Wait for OnCreated() to be called.
  event.Wait();

  controller->Play();

  // Wait until the date is requested at least 10 times.
  for (int i = 0; i < 10; i++) {
    event.Wait();
    uint8 buf[1];
    controller->EnqueueData(buf, 0);
  }

  // Now stop the controller.
  CloseAudioController(controller);
}

TEST(AudioOutputControllerTest, PlayPauseClose) {
  scoped_refptr<AudioManager> audio_manager(AudioManager::Create());
  if (!audio_manager->HasAudioOutputDevices())
    return;

  MockAudioOutputControllerEventHandler event_handler;
  base::WaitableEvent event(false, false);
  base::WaitableEvent pause_event(false, false);

  // If OnCreated is called then signal the event.
  EXPECT_CALL(event_handler, OnCreated(NotNull()))
      .Times(Exactly(1))
      .WillOnce(InvokeWithoutArgs(&event, &base::WaitableEvent::Signal));

  // OnPlaying() will be called only once.
  EXPECT_CALL(event_handler, OnPlaying(NotNull()))
      .Times(Exactly(1));

  // If OnMoreData is called enough then signal the event.
  EXPECT_CALL(event_handler, OnMoreData(NotNull(), _))
      .Times(AtLeast(10))
      .WillRepeatedly(SignalEvent(&event));

  // And then OnPaused() will be called.
  EXPECT_CALL(event_handler, OnPaused(NotNull()))
      .Times(Exactly(1))
      .WillOnce(InvokeWithoutArgs(&pause_event, &base::WaitableEvent::Signal));

  AudioParameters params(AudioParameters::AUDIO_PCM_LINEAR, kChannelLayout,
                         kSampleRate, kBitsPerSample, kSamplesPerPacket);
  scoped_refptr<AudioOutputController> controller =
      AudioOutputController::Create(audio_manager, &event_handler, params,
                                    kBufferCapacity);
  ASSERT_TRUE(controller.get());

  // Wait for OnCreated() to be called.
  event.Wait();

  controller->Play();

  // Wait until the date is requested at least 10 times.
  for (int i = 0; i < 10; i++) {
    event.Wait();
    uint8 buf[1];
    controller->EnqueueData(buf, 0);
  }

  // And then wait for pause to complete.
  ASSERT_FALSE(pause_event.IsSignaled());
  controller->Pause();
  pause_event.Wait();

  // Now stop the controller.
  CloseAudioController(controller);
}

TEST(AudioOutputControllerTest, PlayPauseCloseLowLatency) {
  scoped_refptr<AudioManager> audio_manager(AudioManager::Create());
  if (!audio_manager->HasAudioOutputDevices())
    return;

  MockAudioOutputControllerEventHandler event_handler;
  base::WaitableEvent event(false, false);
  base::WaitableEvent pause_event(false, false);

  // If OnCreated is called then signal the event.
  EXPECT_CALL(event_handler, OnCreated(NotNull()))
      .WillOnce(InvokeWithoutArgs(&event, &base::WaitableEvent::Signal));

  // OnPlaying() will be called only once.
  EXPECT_CALL(event_handler, OnPlaying(NotNull()));

  MockAudioOutputControllerSyncReader sync_reader;
  EXPECT_CALL(sync_reader, UpdatePendingBytes(_))
      .Times(AtLeast(2));
  EXPECT_CALL(sync_reader, Read(_, kHardwareBufferSize))
      .WillRepeatedly(DoAll(SignalEvent(&event),
                            Return(4)));
  EXPECT_CALL(event_handler, OnPaused(NotNull()))
      .WillOnce(InvokeWithoutArgs(&pause_event, &base::WaitableEvent::Signal));
  EXPECT_CALL(sync_reader, Close());

  AudioParameters params(AudioParameters::AUDIO_PCM_LINEAR, kChannelLayout,
                         kSampleRate, kBitsPerSample, kSamplesPerPacket);
  scoped_refptr<AudioOutputController> controller =
      AudioOutputController::CreateLowLatency(audio_manager,
                                              &event_handler,
                                              params,
                                              &sync_reader);
  ASSERT_TRUE(controller.get());

  // Wait for OnCreated() to be called.
  event.Wait();

  ASSERT_FALSE(pause_event.IsSignaled());
  controller->Play();
  controller->Pause();
  pause_event.Wait();

  // Now stop the controller.
  CloseAudioController(controller);
}

TEST(AudioOutputControllerTest, PlayPausePlay) {
  scoped_refptr<AudioManager> audio_manager(AudioManager::Create());
  if (!audio_manager->HasAudioOutputDevices())
    return;

  MockAudioOutputControllerEventHandler event_handler;
  base::WaitableEvent event(false, false);
  base::WaitableEvent pause_event(false, false);

  // If OnCreated is called then signal the event.
  EXPECT_CALL(event_handler, OnCreated(NotNull()))
      .Times(Exactly(1))
      .WillOnce(InvokeWithoutArgs(&event, &base::WaitableEvent::Signal));

  // OnPlaying() will be called only once.
  EXPECT_CALL(event_handler, OnPlaying(NotNull()))
      .Times(Exactly(1))
      .RetiresOnSaturation();

  // If OnMoreData() is called enough then signal the event.
  EXPECT_CALL(event_handler, OnMoreData(NotNull(), _))
      .Times(AtLeast(1))
      .WillRepeatedly(SignalEvent(&event));

  // And then OnPaused() will be called.
  EXPECT_CALL(event_handler, OnPaused(NotNull()))
      .Times(Exactly(1))
      .WillOnce(InvokeWithoutArgs(&pause_event, &base::WaitableEvent::Signal));

  // OnPlaying() will be called only once.
  EXPECT_CALL(event_handler, OnPlaying(NotNull()))
    .Times(Exactly(1))
    .RetiresOnSaturation();

  AudioParameters params(AudioParameters::AUDIO_PCM_LINEAR, kChannelLayout,
                         kSampleRate, kBitsPerSample, kSamplesPerPacket);
  scoped_refptr<AudioOutputController> controller =
      AudioOutputController::Create(audio_manager, &event_handler, params,
                                    kBufferCapacity);
  ASSERT_TRUE(controller.get());

  // Wait for OnCreated() to be called.
  event.Wait();

  controller->Play();

  // Wait until the date is requested at least 10 times.
  for (int i = 0; i < 10; i++) {
    event.Wait();
    uint8 buf[1];
    controller->EnqueueData(buf, 0);
  }

  // And then wait for pause to complete.
  ASSERT_FALSE(pause_event.IsSignaled());
  controller->Pause();
  pause_event.Wait();

  // Then we play again.
  controller->Play();

  // Wait until the date is requested at least 10 times.
  for (int i = 0; i < 10; i++) {
    event.Wait();
    uint8 buf[1];
    controller->EnqueueData(buf, 0);
  }

  // Now stop the controller.
  CloseAudioController(controller);
}

TEST(AudioOutputControllerTest, HardwareBufferTooLarge) {
  scoped_refptr<AudioManager> audio_manager(AudioManager::Create());
  if (!audio_manager->HasAudioOutputDevices())
    return;

  // Create an audio device with a very large hardware buffer size.
  MockAudioOutputControllerEventHandler event_handler;
  AudioParameters params(AudioParameters::AUDIO_PCM_LINEAR, kChannelLayout,
                         kSampleRate, kBitsPerSample,
                         kSamplesPerPacket * 1000);
  scoped_refptr<AudioOutputController> controller =
      AudioOutputController::Create(audio_manager, &event_handler, params,
                                    kBufferCapacity);

  // Use assert because we don't stop the device and assume we can't
  // create one.
  ASSERT_FALSE(controller);
}

TEST(AudioOutputControllerTest, CloseTwice) {
  scoped_refptr<AudioManager> audio_manager(AudioManager::Create());
  if (!audio_manager->HasAudioOutputDevices())
    return;

  MockAudioOutputControllerEventHandler event_handler;
  base::WaitableEvent event(false, false);

  // If OnCreated is called then signal the event.
  EXPECT_CALL(event_handler, OnCreated(NotNull()))
      .WillOnce(SignalEvent(&event));

  // One OnMoreData() is expected.
  EXPECT_CALL(event_handler, OnMoreData(NotNull(), _))
      .Times(AtLeast(1))
      .WillRepeatedly(SignalEvent(&event));

  AudioParameters params(AudioParameters::AUDIO_PCM_LINEAR, kChannelLayout,
                         kSampleRate, kBitsPerSample, kSamplesPerPacket);
  scoped_refptr<AudioOutputController> controller =
      AudioOutputController::Create(audio_manager, &event_handler, params,
                                    kBufferCapacity);
  ASSERT_TRUE(controller.get());

  // Wait for OnCreated() to be called.
  event.Wait();

  // Wait for OnMoreData() to be called.
  event.Wait();

  base::WaitableEvent closed_event_1(true, false);
  controller->Close(base::Bind(&base::WaitableEvent::Signal,
                               base::Unretained(&closed_event_1)));

  base::WaitableEvent closed_event_2(true, false);
  controller->Close(base::Bind(&base::WaitableEvent::Signal,
                                base::Unretained(&closed_event_2)));

  closed_event_1.Wait();
  closed_event_2.Wait();
}

}  // namespace media
