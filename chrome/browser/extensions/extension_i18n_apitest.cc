// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/extension_apitest.h"

IN_PROC_BROWSER_TEST_F(ExtensionApiTest, I18N) {
  ASSERT_TRUE(StartTestServer());
  ASSERT_TRUE(RunExtensionTest("i18n")) << message_;
}
