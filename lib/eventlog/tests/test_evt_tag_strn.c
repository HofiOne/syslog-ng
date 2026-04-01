/*
 * Copyright (c) 2026 One Identity
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#include <criterion/criterion.h>
#include <evtlog.h>

static void
_assert_evt_tag_strn(const gchar *input, gsize len, const gchar *expected)
{
  gchar *key = "test:evt_tag_strn";
  EVTCONTEXT *ctx = evt_ctx_init("evt_tag_strn", LOG_AUTH);
  EVTREC *event_rec = evt_rec_init(ctx, LOG_INFO, "");

  evt_rec_add_tag(event_rec, evt_tag_strn(key, input, len));
  gchar *formatted_result = evt_format(event_rec);

  GString *expected_result = g_string_sized_new(16);
  g_string_append_printf(expected_result, "; %s='%s'", key, expected);

  cr_assert_str_eq(formatted_result, expected_result->str);

  free(formatted_result);
  g_string_free(expected_result, TRUE);
  evt_ctx_free(ctx);
}

Test(evt_tag_strn, test_multiple_inputs)
{
  _assert_evt_tag_strn("hello", 10, "hello");    /* 5 <= 10: no truncation */
  _assert_evt_tag_strn("hello", 5,  "hello");    /* 5 <= 5: exact fit, no truncation */
  _assert_evt_tag_strn("hello", 4,  "hell...");  /* 5 > 4: 4 visible chars + "..." */
  _assert_evt_tag_strn("hello", 2,  "he...");    /* 5 > 2: 2 visible chars + "..." */
  _assert_evt_tag_strn("hello", 1,  "h...");     /* 5 > 1: 1 visible char + "..." */
  _assert_evt_tag_strn("hello", 0,  "...");      /* 5 > 0: 0 visible chars + "..." */
  _assert_evt_tag_strn(NULL,    6,  "(null)");   /* "(null)" == 6: exact fit, no truncation */
  _assert_evt_tag_strn(NULL,    3,  "(nu...");   /* "(null)" > 3: 3 visible chars + "..." */
}
