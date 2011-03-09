// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/autofill/autofill_country.h"

#include <map>
#include <utility>

#include "base/scoped_ptr.h"
#include "base/singleton.h"
#include "base/string_util.h"
#include "base/utf_string_conversions.h"
#include "chrome/browser/browser_process.h"
#include "grit/generated_resources.h"
#include "ui/base/l10n/l10n_util_collator.h"
#include "ui/base/l10n/l10n_util.h"
#include "unicode/coll.h"
#include "unicode/locid.h"
#include "unicode/ucol.h"
#include "unicode/uloc.h"

namespace {

struct AutofillCountryData {
  std::string country_code;
  int postal_code_label_id;
  int state_label_id;
};

// The maximum capacity needed to store a locale up to the country code.
const size_t kLocaleCapacity =
    ULOC_LANG_CAPACITY + ULOC_SCRIPT_CAPACITY + ULOC_COUNTRY_CAPACITY + 1;

// Maps country codes to localized label string identifiers.
const AutofillCountryData kCountryData[] = {
  {"AD", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PARISH},
  {"AE", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_EMIRATE},
  {"AF", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"AG", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"AI", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"AL", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"AM", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"AN", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"AO", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"AQ", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"AR", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_STATE},
  {"AS", IDS_AUTOFILL_DIALOG_ZIP_CODE,    IDS_AUTOFILL_DIALOG_STATE},
  {"AT", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"AU", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_STATE},
  {"AW", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"AX", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"AZ", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"BA", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"BB", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PARISH},
  {"BD", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"BE", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"BF", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"BG", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"BH", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"BI", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"BJ", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"BL", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"BM", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"BN", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"BO", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"BR", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_STATE},
  {"BS", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_ISLAND},
  {"BT", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"BV", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"BW", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"BY", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"BZ", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"CA", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"CC", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"CD", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"CF", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"CG", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"CH", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"CI", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"CK", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"CL", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_STATE},
  {"CM", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"CN", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"CO", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"CR", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"CV", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_ISLAND},
  {"CX", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"CY", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"CZ", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"DE", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"DJ", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"DK", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"DM", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"DO", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"DZ", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"EC", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"EE", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"EG", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"EH", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"ER", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"ES", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"ET", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"FI", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"FJ", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"FK", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"FM", IDS_AUTOFILL_DIALOG_ZIP_CODE,    IDS_AUTOFILL_DIALOG_STATE},
  {"FO", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"FR", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"GA", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"GB", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_COUNTY},
  {"GD", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"GE", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"GF", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"GG", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"GH", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"GI", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"GL", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"GM", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"GN", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"GP", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"GQ", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"GR", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"GS", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"GT", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"GU", IDS_AUTOFILL_DIALOG_ZIP_CODE,    IDS_AUTOFILL_DIALOG_STATE},
  {"GW", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"GY", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"HK", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_AREA},
  {"HM", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"HN", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"HR", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"HT", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"HU", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"ID", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"IE", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_COUNTY},
  {"IL", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"IM", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"IN", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_STATE},
  {"IO", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"IQ", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"IS", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"IT", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"JE", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"JM", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PARISH},
  {"JO", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"JP", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PREFECTURE},
  {"KE", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"KG", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"KH", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"KI", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_ISLAND},
  {"KM", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"KN", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_ISLAND},
  {"KP", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"KR", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"KW", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"KY", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_ISLAND},
  {"KZ", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"LA", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"LB", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"LC", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"LI", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"LK", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"LR", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"LS", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"LT", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"LU", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"LV", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"LY", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"MA", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"MC", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"MD", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"ME", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"MF", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"MG", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"MH", IDS_AUTOFILL_DIALOG_ZIP_CODE,    IDS_AUTOFILL_DIALOG_STATE},
  {"MK", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"ML", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"MN", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"MO", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"MP", IDS_AUTOFILL_DIALOG_ZIP_CODE,    IDS_AUTOFILL_DIALOG_STATE},
  {"MQ", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"MR", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"MS", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"MT", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"MU", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"MV", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"MW", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"MX", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_STATE},
  {"MY", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_STATE},
  {"MZ", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"NA", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"NC", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"NE", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"NF", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"NG", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_STATE},
  {"NI", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_DEPARTMENT},
  {"NL", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"NO", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"NP", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"NR", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_DISTRICT},
  {"NU", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"NZ", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"OM", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"PA", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"PE", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"PF", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_ISLAND},
  {"PG", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"PH", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"PK", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"PL", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"PM", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"PN", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"PR", IDS_AUTOFILL_DIALOG_ZIP_CODE,    IDS_AUTOFILL_DIALOG_PROVINCE},
  {"PS", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"PT", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"PW", IDS_AUTOFILL_DIALOG_ZIP_CODE,    IDS_AUTOFILL_DIALOG_STATE},
  {"PY", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"QA", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"RE", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"RO", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"RS", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"RU", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"RW", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"SA", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"SB", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"SC", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_ISLAND},
  {"SE", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"SG", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"SH", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"SI", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"SJ", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"SK", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"SL", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"SM", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"SN", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"SO", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"SR", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"ST", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"SV", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"SZ", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"TC", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"TD", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"TF", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"TG", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"TH", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"TJ", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"TK", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"TL", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"TM", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"TN", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"TO", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"TR", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"TT", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"TV", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_ISLAND},
  {"TW", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_COUNTY},
  {"TZ", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"UA", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"UG", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"UM", IDS_AUTOFILL_DIALOG_ZIP_CODE,    IDS_AUTOFILL_DIALOG_STATE},
  {"US", IDS_AUTOFILL_DIALOG_ZIP_CODE,    IDS_AUTOFILL_DIALOG_STATE},
  {"UY", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"UZ", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"VA", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"VC", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"VE", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"VG", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"VI", IDS_AUTOFILL_DIALOG_ZIP_CODE,    IDS_AUTOFILL_DIALOG_STATE},
  {"VN", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"VU", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"WF", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"WS", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"YE", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"YT", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"ZA", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"ZM", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
  {"ZW", IDS_AUTOFILL_DIALOG_POSTAL_CODE, IDS_AUTOFILL_DIALOG_PROVINCE},
};

// A singleton class that encapsulates a map from country codes to country data.
class AutofillCountries {
 public:
  // Map type from country codes to country data.
  typedef std::map<std::string, AutofillCountryData> MapType;

  static AutofillCountries* GetInstance();
  static const MapType& countries();

 private:
  AutofillCountries();
  friend struct DefaultSingletonTraits<AutofillCountries>;

  MapType countries_;

  DISALLOW_COPY_AND_ASSIGN(AutofillCountries);
};

// static
AutofillCountries* AutofillCountries::GetInstance() {
  return Singleton<AutofillCountries>::get();
}

AutofillCountries::AutofillCountries() {
  // Add all the countries we have explicit data for.
  for (size_t i = 0; i < arraysize(kCountryData); ++i) {
    const AutofillCountryData& data = kCountryData[i];
    countries_.insert(std::make_pair(data.country_code, data));
  }

  // Add any other countries that ICU knows about, falling back to default data
  // values.
  for (const char* const* country_pointer = Locale::getISOCountries();
       *country_pointer;
       ++country_pointer) {
    std::string country_code = *country_pointer;
    if (!countries_.count(country_code)) {
      AutofillCountryData data = {
        country_code,
        IDS_AUTOFILL_DIALOG_POSTAL_CODE,
        IDS_AUTOFILL_DIALOG_PROVINCE
      };
      countries_.insert(std::make_pair(country_code, data));
    }
  }
}

const AutofillCountries::MapType& AutofillCountries::countries() {
  return GetInstance()->countries_;
}

// Returns the country name corresponding to |country_code|, localized to the
// |display_locale|.
string16 GetDisplayName(const std::string& country_code,
                        const icu::Locale& display_locale) {
  icu::Locale country_locale(NULL, country_code.c_str());
  icu::UnicodeString name;
  country_locale.getDisplayName(display_locale, name);

  DCHECK_GT(name.length(), 0);
  return string16(name.getBuffer(), name.length());
}

}  // namespace

AutofillCountry::AutofillCountry(const std::string& country_code,
                                 const std::string& locale) {
  const AutofillCountries::MapType& countries = AutofillCountries::countries();
  DCHECK(countries.count(country_code));
  const AutofillCountryData& data = countries.find(country_code)->second;

  country_code_ = country_code;
  name_ = GetDisplayName(country_code, icu::Locale(locale.c_str()));
  postal_code_label_ = l10n_util::GetStringUTF16(data.postal_code_label_id);
  state_label_ = l10n_util::GetStringUTF16(data.state_label_id);
}

AutofillCountry::~AutofillCountry() {
}

// static
void AutofillCountry::GetAvailableCountries(
    std::vector<std::string>* country_codes) {
  DCHECK(country_codes);

  const AutofillCountries::MapType& country_data =
      AutofillCountries::countries();
  for (AutofillCountries::MapType::const_iterator it = country_data.begin();
       it != country_data.end();
       ++it) {
    country_codes->push_back(it->first);
  }
}

// static
const std::string AutofillCountry::CountryCodeForLocale(
    const std::string& locale) {
  // Add likely subtags to the locale. In particular, add any likely country
  // subtags -- e.g. for locales like "ru" that only include the language.
  std::string likely_locale;
  UErrorCode error_ignored = U_ZERO_ERROR;
  uloc_addLikelySubtags(locale.c_str(),
                        WriteInto(&likely_locale, kLocaleCapacity),
                        kLocaleCapacity,
                        &error_ignored);

  // Extract the country code.
  std::string country_code = icu::Locale(likely_locale.c_str()).getCountry();

  // Default to the United States if we have no better guess.
  if (!AutofillCountries::countries().count(country_code))
    return "US";
  return country_code;
}

// static
const std::string AutofillCountry::GetCountryCode(
    const string16& country, const std::string& locale) {
  // First, check for a few common synonyms.
  if (country == ASCIIToUTF16("United States of America"))
    return "US";
  if (country == ASCIIToUTF16("Great Britain") ||
      country == ASCIIToUTF16("UK"))
    return "GB";
  if (country == ASCIIToUTF16("Brasil"))
    return "BR";
  if (country == ASCIIToUTF16("Deutschland"))
    return "DE";

  const AutofillCountries::MapType& country_data =
      AutofillCountries::countries();

  // Check to see if |country| is actually a country code, in which case we can
  // short-circuit a lot of the hard work.
  std::string country_utf8 = UTF16ToUTF8(StringToUpperASCII(country));
  if (country_data.count(country_utf8))
    return country_utf8;

  const icu::Locale icu_locale(locale.c_str());

  // Compare case-insensitively and ignoring punctuation.
  UErrorCode ignored = U_ZERO_ERROR;
  scoped_ptr<icu::Collator> collator(
      icu::Collator::createInstance(icu_locale, ignored));
  collator->setStrength(icu::Collator::SECONDARY);
  ignored = U_ZERO_ERROR;
  collator->setAttribute(UCOL_ALTERNATE_HANDLING, UCOL_SHIFTED, ignored);

  for (AutofillCountries::MapType::const_iterator it = country_data.begin();
       it != country_data.end();
       ++it) {
    std::string country_code = it->first;
    std::string iso3_country_code =
        icu::Locale(NULL, country_code.c_str()).getISO3Country();

    string16 name = GetDisplayName(country_code, icu_locale);
    if (country == UTF8ToUTF16(iso3_country_code) ||
        l10n_util::CompareString16WithCollator(collator.get(),
                                               country,
                                               name) == UCOL_EQUAL) {
      return country_code;
    }
  }

  // As a fallback, try assuming the country named is localized to US English.
  if (locale != "en_US")
    return GetCountryCode(country, "en_US");

  return std::string();
}

// static
const std::string AutofillCountry::ApplicationLocale() {
  return g_browser_process->GetApplicationLocale();
}

AutofillCountry::AutofillCountry(const std::string& country_code,
                                 const string16& name,
                                 const string16& postal_code_label,
                                 const string16& state_label)
    : country_code_(country_code),
      name_(name),
      postal_code_label_(postal_code_label),
      state_label_(state_label) {
}
