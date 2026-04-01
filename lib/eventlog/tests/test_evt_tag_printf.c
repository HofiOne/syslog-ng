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

#include <stdlib.h>
#include <string.h>

static void
_assert_evt_tag_printf_str(const gchar *input, const gchar *expected)
{
  gchar *key = "test:evt_tag_printf";
  EVTCONTEXT *ctx = evt_ctx_init("evt_tag_printf", LOG_AUTH);
  EVTREC *event_rec = evt_rec_init(ctx, LOG_INFO, "");

  evt_rec_add_tag(event_rec, evt_tag_printf(key, "%s", input));
  gchar *formatted_result = evt_format(event_rec);

  GString *expected_result = g_string_sized_new(64);
  g_string_append_printf(expected_result, "; %s='%s'", key, expected);

  cr_assert_str_eq(formatted_result, expected_result->str);

  free(formatted_result);
  g_string_free(expected_result, TRUE);
  evt_ctx_free(ctx);
}

static void
_assert_evt_tag_printf_int_pair(gint left, gint right, const gchar *expected)
{
  gchar *key = "test:evt_tag_printf";
  EVTCONTEXT *ctx = evt_ctx_init("evt_tag_printf", LOG_AUTH);
  EVTREC *event_rec = evt_rec_init(ctx, LOG_INFO, "");

  evt_rec_add_tag(event_rec, evt_tag_printf(key, "%d %d", left, right));
  gchar *formatted_result = evt_format(event_rec);

  GString *expected_result = g_string_sized_new(64);
  g_string_append_printf(expected_result, "; %s='%s'", key, expected);

  cr_assert_str_eq(formatted_result, expected_result->str);

  free(formatted_result);
  g_string_free(expected_result, TRUE);
  evt_ctx_free(ctx);
}

Test(evt_tag_printf, test_multiple_inputs)
{
#define LONG_INPUT_LEN 2000
#define MAX_BUF_LEN 1024

  gchar *long_input = malloc(LONG_INPUT_LEN + 1);
  memset(long_input, 'a', LONG_INPUT_LEN);
  long_input[LONG_INPUT_LEN] = '\0';

  GString *expected_truncated = g_string_sized_new(1100);
  g_string_append_len(expected_truncated, long_input, MAX_BUF_LEN - 1);
  g_string_append(expected_truncated, "...");

  _assert_evt_tag_printf_int_pair(5, 6, "5 6");
  _assert_evt_tag_printf_str("short", "short");
  _assert_evt_tag_printf_str(long_input, expected_truncated->str);

  g_string_free(expected_truncated, TRUE);
  free(long_input);

#undef LONG_INPUT_LEN
#undef MAX_BUF_LEN
}
